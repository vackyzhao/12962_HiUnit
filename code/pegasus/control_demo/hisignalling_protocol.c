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

#include <hi_types_base.h>
#include <hi_early_debug.h>
#include <hi_stdlib.h>
#include <hi_uart.h>
#include <hi_task.h>
#include <app_demo_uart.h>
#include <iot_uart.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "hi_io.h"
#include "iot_gpio.h"
#include "hisignalling_protocol.h"
#include "hi_i2c.h"
#include "iot_i2c.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "iot_watchdog.h"
#include "iot_errno.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "hi_time.h"



hi_u8   g_sendUartBuff[UART_BUFF_SIZE];
UartDefConfig recConfig  = {0};


/* Log level look up table */
static const char *hisignallingLevelNames[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};
/* get hisignaling log level */
const char *HisignallingLevelNum (HisignallingLogType hisignallingLevel)
{
    if (hisignallingLevel >= HISIGNALLING_LEVEL_MAX) {
        return "NULL";
    } else {
        return hisignallingLevelNames[hisignallingLevel];
    }
}
#define RIGHT_MOVE_8_BIT (8)
#define RIGHT_MOVE_16_BIT (16)
#define RIGHT_MOVE_24_BIT (24)

/* hisignal Hi3861 message recevice */
HisignallingErrorType HisignallingMsgReceive(hi_u8 *buf, hi_u32 len, int *position_lr, int *position_ud)
{
    
    int temp_lr = 0;
    int temp_ud = 0;
    
    if (buf == HI_NULL && len > 0) {
        HISIGNALLING_LOG_FATAL("received buf is null");
        return HISGNALLING_RET_VAL_MAX;
    }

    /* 输出回显收到的数据 */
    
    // if ((buf[0] == HISIGNALLING_MSG_FRAME_HEADER_1) && (buf[1] == HISIGNALLING_MSG_FRAME_HEADER_2)) {
    //     HISIGNALLING_LOG_INFO("received data is:");
    //     for (int i = 0; i < len; i++) {
    //         HISIGNALLING_LOG_INFO("0x%x", buf[i]);
    //     }
    // }
    //printf("start converting\n");
    if ((buf[0] == HISIGNALLING_MSG_FRAME_HEADER_1) && (buf[1] == HISIGNALLING_MSG_FRAME_HEADER_2) && (buf[2] != HISIGNALLING_MSG_FRAME_HEADER_1)) {
        for (int i = 2; i < 6; i++) {
            temp_lr = temp_lr*10 + buf[i];
            //printf("temp_lr is %d\n", temp_lr);
        }
        if(0!=temp_lr)
        {
            *position_lr = temp_lr;
            //printf("position_lr is %d\n", *position_lr);
        }

        for (int i = 6; i < 10; i++) {
            temp_ud = temp_ud*10 + buf[i];
        }
        if(0!=temp_ud)
        {
            *position_ud = temp_ud;
        }
   }
    return HISIGNALLING_RET_VAL_CORRECT;
}


int SetUartReceiveFlag(void)
{
    return recConfig.g_uartReceiveFlag;
}
double PID_control(double setpoint, double measured_value, double Kp, double Ki, double Kd, double *previous_error, double *integral) {
    double error = setpoint - measured_value;
    *integral = *integral + error;
    double derivative = error - *previous_error;
    double output = Kp*error + Ki*(*integral) + Kd*derivative;
    *previous_error = error;
    return Kp*error;
}

void I2CInit(){
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoTI2cInit(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE); /* baudrate: 400kbps */
    IoTI2cSetBaudrate(IOT_I2C_IDX_0, HI_I2C_IDX_BAUDRATE);
}
unsigned int initPCA9685(unsigned int i2cId) {
    unsigned char data[2];
    unsigned int ret;
    double T = 20000;	//周期，单位是us
	unsigned char prescale;
	double osc_clock = 25000000;
	unsigned char oldmode = 0x20;
    unsigned char newmode = 0x00;
	T /= 1.05;	//justify
	T /= 1000000;	//us -> s
	prescale = (unsigned char)(osc_clock/4096*T - 1);
    // Set the PCA9685 to sleep mode
    newmode = (oldmode&0x7f) | 0x10;
    data[0] = MODE1;
    data[1] = newmode;
    ret = IoTI2cWrite(i2cId, PCA9685_ADDRESS, data, 2);
    if (ret != 0) {
        return ret;
    }

    // Set prescale for 50Hz PWM frequency
    data[0] = PRESCALE;
    data[1] = prescale;
    ret = IoTI2cWrite(i2cId, PCA9685_ADDRESS, data, 2);
    if (ret != 0) {
        return ret;
    }

    // Wake up the PCA9685
    oldmode &= 0xef;
    data[0] = MODE1;
    data[1] = oldmode;
    ret = IoTI2cWrite(i2cId, PCA9685_ADDRESS, data, 2);
    if (ret != 0) {
        return ret;
    }

    usleep(5);
    data[1] = oldmode | 0xa1;
    ret = IoTI2cWrite(i2cId, PCA9685_ADDRESS, data, 2);
    if (ret != 0) {
        return ret;
    }
    return 0;
}
unsigned int setPWM(unsigned int i2cId, unsigned char channel, unsigned short on, unsigned short off) {
    unsigned char data[5];
    unsigned int ret;

    // Set the ON and OFF timing for the PWM output
    data[0] = LED0_ON_L + 4*channel;
    data[1] = on & 0xFF;
    data[2] = on >> 8;
    data[3] = off & 0xFF;
    data[4] = off >> 8;
    ret = IoTI2cWrite(i2cId, PCA9685_ADDRESS, data, 5);
    if (ret != 0) {
        return ret;
    }

    return 0;
}
void AngleCalculation(int current_position_lr, int current_position_ud, int previous_position_lr, int previous_position_ud, double *delta_angle_lr, double *delta_angle_ud)
{
    
    *delta_angle_lr = KP_LR * (double)(current_position_lr - TARGETLR);
    *delta_angle_ud = KP_UD * (double)(current_position_ud - TARGETUD);
    printf("before -- delta_angle_lr: %f, delta_angle_ud: %f\n", delta_angle_lr, delta_angle_ud);
    int delta_position_lr = current_position_lr - previous_position_lr;
    int delta_position_ud = current_position_ud - previous_position_ud;
    if((abs(delta_position_lr) < 10) || abs(*delta_angle_lr) < 5)
    {
        *delta_angle_lr = 0;
    }
    if((abs(delta_position_ud) < 10) || abs(*delta_angle_ud) < 5)
    {
        *delta_angle_ud = 0;
    }
    printf("after -- delta_angle_lr: %f, delta_angle_ud: %f\n", delta_angle_lr, delta_angle_ud);
        
}
void set_angle_lr(double angle) {
    unsigned int ret;
    unsigned int ticks = TICK_FOR_0_DEGREE + angle * TICK_PER_DEGREE;
    ret = setPWM(I2CID, LRSERVO, 0, ticks);
    if (ret != 0) {
        printf("set ld servo failed\n");
    }
    printf("Setting servo angle (LR) to %f\n", angle);
}
void set_angle_ud(double angle) {
    unsigned int ret;
    if(angle > 40){
        unsigned int ticks = TICK_FOR_0_DEGREE + angle * TICK_PER_DEGREE;
        ret = setPWM(I2CID, UDSERVO, 0, ticks);
        if (ret != 0) {
            printf("set ud servo failed\n");
        }
    }
    else{
        printf("angle is too small\n");
    }
    
    printf("Setting servo angle (UD) to %f\n", angle);
}
void set_angle_lr_mild(int current_angle_lr, int delta)
{
    double desired_angle_lr = current_angle_lr;
    for(int i = 0; i < 100; i++)
    {
        desired_angle_lr = current_angle_lr + delta / 100;
        set_angle_lr(desired_angle_lr);
        usleep(MILDTIME);
    }
}
void set_angle_ud_mild(int current_angle_ud, int delta)
{
    double desired_angle_ud = current_angle_ud;
    for(int i = 0; i < 100; i++)
    {
        desired_angle_ud = current_angle_ud + delta / 100;
        set_angle_ud(desired_angle_ud);
        usleep(MILDTIME);
    }
}


void GetPosition(int *position_lr, int *position_ud)
{
    int position_lr_temp = 0;
    int position_ud_temp = 0;
    (void)memset_s(g_sendUartBuff, sizeof(g_sendUartBuff) / sizeof(g_sendUartBuff[0]),
            0x0, sizeof(g_sendUartBuff)/sizeof(g_sendUartBuff[0]));
    if (GetUartConfig(UART_RECEIVE_FLAG) == HI_TRUE) 
    {
            HisignallingMsgReceive(GetUartReceiveMsg(), GetUartConfig(UART_RECVIVE_LEN), &position_lr_temp, &position_ud_temp);
            if((position_lr_temp > 0) && (position_ud_temp > 0)){
                //printf("-------------------------");
                *position_lr = position_lr_temp;
                *position_ud = position_ud_temp;
            }
            (void)SetUartRecvFlag(UART_RECV_FALSE);
            ResetUartReceiveMsg();
    }
}

void Hcsr04Init(void)
{
    /*
     * 设置超声波Echo为输入模式
     * 设置GPIO8功能（设置为GPIO功能）
     * Set ultrasonic echo as input mode
     * Set GPIO8 function (set as GPIO function)
     */
    IoSetFunc(IOT_IO_NAME_GPIO_8, IOT_IO_FUNC_GPIO_8_GPIO);
    /*
     * 设置GPIO8为输入方向
     * Set GPIO8 as the input direction
     */
    IoTGpioSetDir(IOT_IO_NAME_GPIO_8, IOT_GPIO_DIR_IN);
    /*
     * 设置GPIO7功能（设置为GPIO功能）
     * Set GPIO7 function (set as GPIO function)
     */
    IoSetFunc(IOT_IO_NAME_GPIO_7, IOT_IO_FUNC_GPIO_7_GPIO);
    /*
     * 设置GPIO7为输出方向
     * Set GPIO7 as the output direction
     */
    IoTGpioSetDir(IOT_IO_NAME_GPIO_7, IOT_GPIO_DIR_OUT);
}
float GetDistance(void)
{
    static unsigned long start_time = 0, time = 0;
    float distance = 0.0;
    IotGpioValue value = IOT_GPIO_VALUE0;
    unsigned int flag = 0;
    /*
     * 设置GPIO7输出低电平
     * 给trig发送至少10us的高电平脉冲，以触发传感器测距
     * Set GPIO7 to output direction
     * Send a high level pulse of at least 10us to the trig to trigger the range measurement of the sensor
     */
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_7, IOT_GPIO_VALUE1);
    hi_udelay(DELAY_US20);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_7, IOT_GPIO_VALUE0);
    /*
     * 计算与障碍物之间的距离
     * Calculate the distance from the obstacle
     */
    while (1) {
        /*
         * 获取GPIO8的输入电平状态
         * Get the input level status of GPIO8
         */
        IoTGpioGetInputVal(IOT_IO_NAME_GPIO_8, &value);
        /*
         * 判断GPIO8的输入电平是否为高电平并且flag为0
         * Judge whether the input level of GPIO8 is high and the flag is 0
         */
        if (value == IOT_GPIO_VALUE1 && flag == 0) {
            /*
             * 获取系统时间
             * get SysTime
             */
            start_time = hi_get_us();
            flag = 1;
        }
        /*
         * 判断GPIO8的输入电平是否为低电平并且flag为1
         * Judge whether the input level of GPIO8 is low and the flag is 1
         */
        if (value == IOT_GPIO_VALUE0 && flag == 1) {
            /*
             * 获取高电平持续时间
             * Get high level duration
             */
            time = hi_get_us() - start_time;
            break;
        }
    }
    /* 计算距离障碍物距离（340米/秒 转换为 0.034厘米/微秒, 2代表去来，两倍距离） */
    /* Calculate the distance from the obstacle */
    /* (340 m/s is converted to 0.034 cm/microsecond 2 represents going and coming, twice the distance) */
    distance = time * 0.034 / 2;
    printf("distance is %0.2f cm\r\n", distance);
    return distance;
}

void Uart2GpioInit(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_11);
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioInit(IOT_IO_NAME_GPIO_12);
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_UART2_RXD);
}
void Uart2Config(void)
{
    uint32_t ret;
    /* Initialize UART configuration, baud rate is 9600, data bit is 8, stop bit is 1, parity is NONE */
    IotUartAttribute uart_attr = {
        .baudRate = 9600,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    ret = IoTUartInit(HI_UART_IDX_2, &uart_attr);
    if (ret != IOT_SUCCESS) {
        printf("Init Uart2 Falied Error No : %d\n", ret);
        return;
    }
    printf("Init Uart2 Success\n");
}
void Communication(){
    char *msg = "hello";
    IoTUartWrite(HI_UART_IDX_2, (unsigned char*)msg, strlen(msg));
    //printf("msg: %s\n", msg);
}
void CarMovement(speed_left, speed_right){
    char speeds[UART_BUFF_SIZE] = {0};
    sprintf(speeds, "FF %u %u %u %u %u", speed_left, speed_right, speed_left, speed_right, (speed_left * 2 + speed_right * 2) % 100);
    IoTUartWrite(HI_UART_IDX_1, (unsigned char*)speeds, strlen(speeds));
    printf("speeds: %s\n", speeds);
}
void CarController(double *delta_angle_lr, double *delta_angle_ud){
    float ObliqueDistance = GetDistance();
    float HorizontalDistance = ObliqueDistance * cos(*delta_angle_ud);
    if(HorizontalDistance > 70){
       
    }
    else{
       
    }
}



hi_void *HisignallingMsgHandle(char *param)
{
    Uart2GpioInit();
    Uart2Config();
    I2CInit();
    Hcsr04Init();
    unsigned char *recBuff = NULL;
    double Ki_lr = 0.001, Kd_lr = 0.01; // 左右转动
    double Ki_ud = 0.001, Kd_ud = 0.01; // 上下转动
    double previous_error_lr = 0, previous_error_ud = 0;
    double integral_lr = 0, integral_ud = 0;
    int current_position_lr = 960, current_position_ud = 480; //中心坐标
    int previous_position_lr = 960, previous_position_ud = 480; 
    int delta_position_lr = 0, delta_position_ud = 0;
    double current_angle_lr, current_angle_ud; // current angle
    
    double calculated_angle_lr, calculated_angle_ud; // angle calculated by PID control
    double delta_angle_lr, delta_angle_ud; // delta angle
    double distance_estimate_lr = 1.0, distance_estimate_ud = 1.0;
    float distance = 0;
    
    unsigned int ret;
    int count = 0;
    // Initialize the PCA9685
    ret = initPCA9685(I2CID);
    current_angle_lr = 90;
    current_angle_ud = 90;
    set_angle_lr(current_angle_lr);
    set_angle_ud(current_angle_ud);
    if (ret != 0) {
        printf("Failed to initialize PCA9685: %u\n", ret);
    }
    
    while (1) {
        if(FOLLOWMODE == GetMasterStatus()){
            Communication();
            // GetPosition(&current_position_lr, &current_position_ud);
            // printf("current_position_lr: %4d, current_position_ud: %4d\n", current_position_lr, current_position_ud);
            // printf("get position success\n");
            // AngleCalculation(current_position_lr, current_position_ud, previous_position_lr, previous_position_ud, &delta_angle_lr, &delta_angle_ud);

            // previous_position_lr = current_position_lr;
            // previous_position_ud = current_position_ud;
            
            // current_angle_lr += delta_angle_lr;
            // current_angle_ud += delta_angle_ud;
            // set_angle_lr(current_angle_lr + delta_angle_lr);
            // set_angle_ud(current_angle_ud + delta_angle_ud);
            // printf("set angle success\n");

            // printf("current_position_lr: %4d, current_position_ud: %4d\n", current_position_lr, current_position_ud);

            TaskMsleep(HISGNALLING_FREE_TASK_TIME);
        }
        
    }
}


hi_u32 HisignalingMsgTask(hi_void)
{
    hi_u32 ret = 0;
    printf("HisignallingMsgTask start\n");
    IoTGpioInit(LED_TEST_GPIO);
    IoTGpioSetDir(LED_TEST_GPIO, IOT_GPIO_DIR_OUT);

    osThreadAttr_t hisignallingAttr = {0};

    hisignallingAttr.stack_size = HISIGNALLING_MSG_TASK_STACK_SIZE;
    hisignallingAttr.priority = HISIGNALLING_MSG_TASK_PRIO;
    hisignallingAttr.name = (hi_char*)"hisignal msg task";

    if (osThreadNew((osThreadFunc_t)HisignallingMsgHandle, NULL, &hisignallingAttr) == NULL) {
        HISIGNALLING_LOG_ERROR("Failed to create hisignaling msg task\r\n");
        return HI_ERR_FAILURE;
    }
    return HI_ERR_SUCCESS;
}
SYS_RUN(HisignalingMsgTask);
