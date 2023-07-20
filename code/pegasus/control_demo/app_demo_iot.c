/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hi_task.h>
#include <string.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cJSON.h"
#include "math.h"

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
#define ONE_SECOND                          (1000)
/* oc request id */
#define CN_COMMADN_INDEX                    "commands/request_id="
//light
#define WECHAT_SUBSCRIBE_LIGHT              "light"
#define WECHAT_SUBSCRIBE_LIGHT_ON_STATE     "1"
#define WECHAT_SUBSCRIBE_LIGHT_OFF_STATE    "0"
//car movement
#define WECHAT_SUBSCRIBE_MOVE               "move"
#define WECHAT_SUBSCRIBE_MOVE_LINE          "LineSpeed"
#define WECHAT_SUBSCRIBE_MOVE_ANGLE         "AngleSpeed"
#define WECHAT_SUBSCRIBE_STOP               "stop"
//servo rotation
#define WECHAT_SUBSCRIBE_ROTATE             "rotate"
#define WECHAT_SUBSCRIBE_ROTATE_LR          "LRSpeed"
#define WECHAT_SUBSCRIBE_ROTATE_UD          "UDSpeed"
//interaction
#define WECHAT_SUBSCRIBE_INTERACT           "interact"
#define WECHAT_SUBSCRIBE_ULTRALIGHT         "ultralight"//激光棒
#define WECHAT_SUBSCRIBE_tick               "tick"      //逗猫棒

#define WIDTH                               (20)
#define HEIGHT                              (21)

int g_masterStatus = 0;
int g_ligthStatus = -1;
int g_rodStatus = 0;
int g_penStatus = 0;
int g_controlMode = 0;
typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool  stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack    msgCallBack;
}FunctionCallback;
FunctionCallback g_functinoCallback;

/* CPU Sleep time Set */
// unsigned int TaskMsleep(unsigned int ms)
// {
//     if (ms <= 0) {
//         return HI_ERR_FAILURE;
//     }
//     return hi_sleep((hi_u32)ms);
// }

int GetMasterStatus(void)
{
    return g_masterStatus;
}

static void DeviceConfigInit(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

static int  DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_functinoCallback.msgCallBack = msgCallBack;
    return 0;
}

static void wechatControlDeviceMsg(hi_gpio_value val)
{
    DeviceConfigInit(val);
}

int map(int value, int in_min, int in_max, int out_min, int out_max)
{
    float scale = (float)(value - in_min) / (float)(in_max - in_min);
    return round(scale * (out_max - out_min) + out_min);
}

//cjson analyze data
void parse_master_switch(char *json_data_str){
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *masterSwitch = cJSON_GetObjectItemCaseSensitive(json_data, "master");
    if (cJSON_IsNumber(masterSwitch)) {
        g_masterStatus = masterSwitch->valueint;
        printf("masterSwitch: %d\n", masterSwitch->valueint);
    }
    cJSON_Delete(json_data);
}

void parse_speed_angle(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *speed1 = cJSON_GetObjectItemCaseSensitive(json_data, "speed1");
    cJSON *angle1 = cJSON_GetObjectItemCaseSensitive(json_data, "angle1");
    if (cJSON_IsNumber(speed1) && cJSON_IsNumber(angle1)) {
        printf("speed1: %d\n", speed1->valueint);
        printf("angle1: %d\n", angle1->valueint);
    }
    
    int speed_line = speed1->valueint;
    int speed_angle = angle1->valueint;
    printf("speed_angle: %d\n", abs(speed_angle));
    if(speed_line > 0){
        speed_line = map(speed_line, 1, 2500, 500, 2000);
    }
    if(abs(speed_angle) < 10){
        speed_angle = 0;

    }
    else if(abs(speed_angle) < 90){
        if(speed_angle > 0){
            speed_angle = map(speed_angle, 10, 90, 0, 1000);
            
        }
        else{
            speed_angle = map(speed_angle, -10, -90, 0, -1000);
            
        }

        
    }
    else if(abs(speed_angle) >= 90  &&  abs(speed_angle) < 170){
        speed_line = -speed_line;
        if(speed_angle > 0){
            speed_angle = map(speed_angle - 90, 0, 80, 1000, 0);
        }
        else{
            speed_angle = map(speed_angle + 90, 0, -80, -1000, 0);
        }
    }
    else if( abs(speed_angle) < 180 && abs(speed_angle) >= 170){
        speed_line = -speed_line;
        speed_angle =  0;
    }
    else{
        speed_angle = 0;
    }
    
    int speed_lh = speed_line + speed_angle;
    int speed_rh = speed_line - speed_angle;
    printf("speed_lh: %d\n", speed_lh);
    printf("speed_rh: %d\n", speed_rh);
    if(g_controlMode == 1){
        
        CarMovement(speed_lh, speed_rh);
        if(speed_lh == 0 && speed_rh == 0){
            printf("car stop\n");
            CarMovement(0, 0);
        }
        else{
            printf("car movement\n");
        }
    }
    //turn the line speed and angle speed into four wheels speed

    //CarMovement(speed1->valueint, angle1->valueint);
    cJSON_Delete(json_data);
}

void parse_pos(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *posx = cJSON_GetObjectItemCaseSensitive(json_data, "posx");
    cJSON *posy = cJSON_GetObjectItemCaseSensitive(json_data, "posy");
    if (cJSON_IsNumber(posx) && cJSON_IsNumber(posy)) {
        printf("posx: %d\n", posx->valueint);
        printf("posy: %d\n", posy->valueint);
    }
    int angle_lr = map(posx->valueint, -40, 40, -75, 75);
    angle_lr = 90 - angle_lr;
    
    int angle_ud = map(posy->valueint, -40, 40, -50, 50);
    angle_ud = 90 - angle_ud;

    printf("angle_lr: %d\n", angle_lr);
    printf("angle_ud: %d\n", angle_ud);
    
    if(g_controlMode){
        set_angle_lr((float)angle_lr);
        set_angle_ud((float)angle_ud);
    }

    cJSON_Delete(json_data);
}

void parse_control_mode(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *controlMode = cJSON_GetObjectItemCaseSensitive(json_data, "mode");
    if (cJSON_IsNumber(controlMode)) {
        g_controlMode = controlMode->valueint;
        printf("ControlMode: %d\n", controlMode->valueint);
    }
    cJSON_Delete(json_data);
}

void parse_laser_pen(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *laserPen = cJSON_GetObjectItemCaseSensitive(json_data, "LaserPen");
    if (cJSON_IsNumber(laserPen)) {
        g_penStatus = laserPen->valueint;
        printf("LaserPen: %d\n", laserPen->valueint);
    }
    cJSON_Delete(json_data);
}

void parse_rod(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }

    cJSON *rod = cJSON_GetObjectItemCaseSensitive(json_data, "CatRod");
    if (cJSON_IsNumber(rod)) {
        g_rodStatus = rod->valueint;
        printf("CatRod: %d\n", rod->valueint);
    }
}

void parse_json_all(char *json_data_str) {
    cJSON *json_data = cJSON_Parse(json_data_str);
    if (json_data == NULL) {
        printf("Error in parsing JSON data.\n");
        return;
    }
    if (cJSON_HasObjectItem(json_data, "master")) {
        parse_master_switch(json_data_str);
    } 
    else if (cJSON_HasObjectItem(json_data, "speed1") && cJSON_HasObjectItem(json_data, "angle1")) {
        parse_speed_angle(json_data_str);
    } 
    else if (cJSON_HasObjectItem(json_data, "posx") && cJSON_HasObjectItem(json_data, "posy")) {
        parse_pos(json_data_str);
    } 
    else if (cJSON_HasObjectItem(json_data, "LaserPen")) {
        parse_laser_pen(json_data_str);
    }
    else if (cJSON_HasObjectItem(json_data, "CatRod")) {
        parse_rod(json_data_str);
    }
    else if (cJSON_HasObjectItem(json_data, "mode")) {
        parse_control_mode(json_data_str);
    }

    cJSON_Delete(json_data);
}


// < this is the callback function, set to the mqtt, and if any messages come, it will be called
// < The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* 云端下发命令后，板端的操作处理 */
    
    parse_json_all(payload);    //parse the json string

    
    //for the car movement --


    //for the servo rotation --


    //for the interaction with cat/animals


    return HI_NULL;
}

/* publish sample */
hi_void IotPublishSample(void)
{
    /* reported attribute */
    WeChatProfile weChatProfile = {
        .subscribeType = "type",
        .status.subState = "state",
        .status.subReport = "reported",
        .status.reportVersion = "version",
        .status.Token = "clientToken",
        /* report motor */
        .reportAction.subDeviceActionMotor = "motor",
        .reportAction.motorActionStatus = 0, /* 0 : motor off */
        /* report temperature */
        .reportAction.subDeviceActionTemperature = "temperature",
        .reportAction.temperatureData = 30, /* 30 :temperature data */
        /* report humidity */
        .reportAction.subDeviceActionHumidity = "humidity",
        .reportAction.humidityActionData = 70, /* humidity data */
        /* report light_intensity */
        .reportAction.subDeviceActionLightIntensity = "light_intensity",
        .reportAction.lightIntensityActionData = 60, /* 60 : light_intensity */
        //add your own report here, and for this project, nothing needs to be done.
    };

    /* report light */
    if (g_ligthStatus == HI_TRUE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 1; /* 1: light on */
    } else if (g_ligthStatus == HI_FALSE) {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    } else {
        weChatProfile.reportAction.subDeviceActionLight = "light";
        weChatProfile.reportAction.lightActionStatus = 0; /* 0: light off */
    }

    /* report car speed */

    /*repoet servo angle*/


    /* profile report */
    IoTProfilePropertyReport(CONFIG_USER_ID, &weChatProfile);
}

// < this is the demo main task entry,here we will set the wifi/cjson/mqtt ready and
// < wait if any work to do in the while
static hi_void *DemoEntry(const char *arg)
{
    WifiStaReadyWait();
    cJsonInit();
    IoTMain();
    /* 云端下发回调 */
    IoTSetMsgCallback(DemoMsgRcvCallBack);
    /* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1) {
        /* 用户可以在这调用发布函数进行发布，需要用户自己写调用函数 */
        //IotPublishSample(); // 发布例程
#endif
        TaskMsleep(ONE_SECOND);
    }
    return NULL;
}

// < This is the demo entry, we create a task here,
// and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE  0x1000
#define CN_IOT_TASK_PRIOR 25
#define CN_IOT_TASK_NAME "IOTDEMO"

static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
 
    attr.name = "IOTDEMO";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CN_IOT_TASK_STACKSIZE;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)DemoEntry, NULL, &attr) == NULL) {
        printf("[mqtt] Falied to create IOTDEMO!\n");
    }
}

SYS_RUN(AppDemoIot);