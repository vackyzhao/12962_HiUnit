/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * 该文件提供了基于yolov2的手部检测以及基于resnet18的手势识别，属于两个wk串行推理。
 * 该文件提供了手部检测和手势识别的模型加载、模型卸载、模型推理以及AI flag业务处理的API接口。
 * 若一帧图像中出现多个手，我们通过算法将最大手作为目标手送分类网进行推理，
 * 并将目标手标记为绿色，其他手标记为红色。
 *
 * This file provides hand detection based on yolov2 and gesture recognition based on resnet18,
 * which belongs to two wk serial inferences. This file provides API interfaces for model loading,
 * model unloading, model reasoning, and AI flag business processing for hand detection
 * and gesture recognition. If there are multiple hands in one frame of image,
 * we use the algorithm to use the largest hand as the target hand for inference,
 * and mark the target hand as green and the other hands as red.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "sample_comm_nnie.h"
#include "sample_media_ai.h"
#include "ai_infer_process.h"
#include "yolov2_hand_detect.h"
#include "vgs_img.h"
#include "ive_img.h"
#include "misc_util.h"
#include "hisignalling.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HAND_FRM_WIDTH     640
#define HAND_FRM_HEIGHT    384
#define DETECT_OBJ_MAX     32
#define RET_NUM_MAX        4
#define DRAW_RETC_THICK    6    // Draw the width of the line
#define WIDTH_LIMIT        32
#define HEIGHT_LIMIT       32
#define IMAGE_WIDTH        224  // The resolution of the model IMAGE sent to the classification is 224*224
#define IMAGE_HEIGHT       224
#define MODEL_FILE_GESTURE    "/userdata/models/hand_classify/hand_gesture.wk" // darknet framework wk model

static int biggestBoxIndex;
static int bestScoreIndex;
static IVE_IMAGE_S img;
static DetectObjInfo objs[DETECT_OBJ_MAX] = {0};
static RectBox boxs[DETECT_OBJ_MAX] = {0};
double scores[DETECT_OBJ_MAX] = {0};
static RectBox objBoxs[DETECT_OBJ_MAX] = {0};
static RectBox remainingBoxs[DETECT_OBJ_MAX] = {0};
static RectBox cnnBoxs[DETECT_OBJ_MAX] = {0}; // Store the results of the classification network
static RecogNumInfo numInfo[RET_NUM_MAX] = {0};
//static IVE_IMAGE_S imgIn;
//static IVE_IMAGE_S imgDst;
//static VIDEO_FRAME_INFO_S frmIn;
//static VIDEO_FRAME_INFO_S frmDst;
int uartFd = 0;

/*
 * 加载手部检测和手势分类模型
 * Load hand detect and classify model
 */
HI_S32 Yolo2HandDetectResnetClassifyLoad()
{
    SAMPLE_SVP_NNIE_CFG_S *self = NULL;
    HI_S32 ret;

    //ret = CnnCreate(&self, MODEL_FILE_GESTURE);
    //*model = ret < 0 ? 0 : (uintptr_t)self;
    HandDetectInit(); // Initialize the hand detection model
    //SAMPLE_PRT("Load hand detect claasify model success\n");
    /*
     * Uart串口初始化
     * Uart open init
     */
    uartFd = UartOpenInit();
    if (uartFd < 0) {
        printf("uart1 open failed\r\n");
    } else {
        printf("uart1 open successed\r\n");
    }
    ret = 0;
    return ret;
}

/*
 * 卸载手部检测和手势分类模型
 * Unload hand detect and classify model
 */
HI_S32 Yolo2HandDetectResnetClassifyUnload()
{
    //CnnDestroy((SAMPLE_SVP_NNIE_CFG_S*)model);
    HandDetectExit(); // Uninitialize the hand detection model
    close(uartFd);
    SAMPLE_PRT("Unload hand detect claasify model success\n");

    return 0;
}

/*
 * 获得最大的手
 * Get the maximum hand
 */
static HI_S32 GetBiggestHandIndex(RectBox boxs[], int detectNum)
{
    HI_S32 handIndex = 0;
    HI_S32 biggestBoxIndex = handIndex;
    HI_S32 biggestBoxWidth = boxs[handIndex].xmax - boxs[handIndex].xmin + 1;
    HI_S32 biggestBoxHeight = boxs[handIndex].ymax - boxs[handIndex].ymin + 1;
    HI_S32 biggestBoxArea = biggestBoxWidth * biggestBoxHeight;

    for (handIndex = 1; handIndex < detectNum; handIndex++) {
        HI_S32 boxWidth = boxs[handIndex].xmax - boxs[handIndex].xmin + 1;
        HI_S32 boxHeight = boxs[handIndex].ymax - boxs[handIndex].ymin + 1;
        HI_S32 boxArea = boxWidth * boxHeight;
        if (biggestBoxArea < boxArea) {
            biggestBoxArea = boxArea;
            biggestBoxIndex = handIndex;
        }
        biggestBoxWidth = boxs[biggestBoxIndex].xmax - boxs[biggestBoxIndex].xmin + 1;
        biggestBoxHeight = boxs[biggestBoxIndex].ymax - boxs[biggestBoxIndex].ymin + 1;
    }

    if ((biggestBoxWidth == 1) || (biggestBoxHeight == 1) || (detectNum == 0)) {
        biggestBoxIndex = -1;
    }

    return biggestBoxIndex;
}
static HI_S32 GetBestScoresHandIndex(double scores[], int detectNum){
    //select the best scores hand
    HI_S32 handIndex = 0;
    HI_S32 bestScoresIndex = handIndex;
    double bestScore = scores[handIndex];
    for(handIndex = 1; handIndex < detectNum; handIndex++){
        if(bestScore < scores[handIndex]){
            bestScore = scores[handIndex];
            bestScoresIndex = handIndex;
        }
    }
    if((bestScore < 0.75) || (detectNum == 0)){
        bestScoresIndex = -1;
    }
    return bestScoresIndex;
}
/*
 * 手势识别信息
 * Hand gesture recognition info
 */
static void HandDetectFlag(const RecogNumInfo resBuf)
{
    HI_CHAR *gestureName = NULL;
    switch (resBuf.num) {
        case 0u:
            gestureName = "gesture fist";
            UartSendRead(uartFd, FistGesture); // 拳头手势
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 1u:
            gestureName = "gesture indexUp";
            UartSendRead(uartFd, ForefingerGesture); // 食指手势
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 2u:
            gestureName = "gesture OK";
            UartSendRead(uartFd, OkGesture); // OK手势
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 3u:
            gestureName = "gesture palm";
            UartSendRead(uartFd, PalmGesture); // 手掌手势
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 4u:
            gestureName = "gesture yes";
            UartSendRead(uartFd, YesGesture); // yes手势
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 5u:
            gestureName = "gesture pinchOpen";
            UartSendRead(uartFd, ForefingerAndThumbGesture); // 食指 + 大拇指
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        case 6u:
            gestureName = "gesture phoneCall";
            UartSendRead(uartFd, LittleFingerAndThumbGesture); // 大拇指 + 小拇指
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
        default:
            gestureName = "gesture others";
            UartSendRead(uartFd, InvalidGesture); // 无效值
            SAMPLE_PRT("----gesture name----:%s\n", gestureName);
            break;
    }
    SAMPLE_PRT("hand gesture success\n");
}
void packData(unsigned char* cat_data, int center_x, int center_y) {
    cat_data[0] = center_x / 1000;
    cat_data[1] = (center_x % 1000) / 100;
    cat_data[2] = (center_x % 100) / 10;
    cat_data[3] = center_x % 10;
    cat_data[4] = center_y / 1000;
    cat_data[5] = (center_y % 1000) / 100;
    cat_data[6] = (center_y % 100) / 10;
    cat_data[7] = center_y % 10;
}
/*
 * 手部检测和手势分类推理
 * Hand detect and classify calculation
 */
HI_S32 Yolo2HandDetectResnetClassifyCal(VIDEO_FRAME_INFO_S *srcFrm, VIDEO_FRAME_INFO_S *dstFrm)
{
    //SAMPLE_SVP_NNIE_CFG_S *self = (SAMPLE_SVP_NNIE_CFG_S*)model;
    HI_S32 resLen = 0;
    int objNum;
    int ret;
    int num = 0;
    unsigned char cat_data[8] = {0};
    int center_x = 0;
    int center_y = 0;
    int width = 0;
    int height = 0;

    ret = FrmToOrigImg((VIDEO_FRAME_INFO_S*)srcFrm, &img);
    SAMPLE_CHECK_EXPR_RET(ret != HI_SUCCESS, ret, "hand detect for YUV Frm to Img FAIL, ret=%#x\n", ret);

    objNum = HandDetectCal(&img, objs); // Send IMG to the detection net for reasoning
    for (int i = 0; i < objNum; i++) {
        cnnBoxs[i] = objs[i].box;
        RectBox *box = &objs[i].box;
        RectBoxTran(box, HAND_FRM_WIDTH, HAND_FRM_HEIGHT,
            dstFrm->stVFrame.u32Width, dstFrm->stVFrame.u32Height);
        //SAMPLE_PRT("yolo2_out: {%d, %d, %d, %d}\n", box->xmin, box->ymin, box->xmax, box->ymax);
        boxs[i] = *box;
        scores[i] = objs[i].score;
    }
    bestScoreIndex = GetBestScoresHandIndex(scores, objNum);
    biggestBoxIndex = GetBiggestHandIndex(boxs, objNum);
    //SAMPLE_PRT("biggestBoxIndex:%d, objNum:%d\n", biggestBoxIndex, objNum);
    //SAMPLE_PRT("yolo2_final_out: {xmin:%d, ymin:%d, xmaxL:%d, ymax:%d}   {xcenter:%d, ycenter:%d, width:%d, height:%d}\n", boxs[biggestBoxIndex].xmin, boxs[biggestBoxIndex].ymin, boxs[biggestBoxIndex].xmax, boxs[biggestBoxIndex].ymax,(boxs[biggestBoxIndex].xmin+boxs[biggestBoxIndex].xmax)/2,(boxs[biggestBoxIndex].ymin+boxs[biggestBoxIndex].ymax)/2, (boxs[biggestBoxIndex].xmax-boxs[biggestBoxIndex].xmin), (boxs[biggestBoxIndex].ymax-boxs[biggestBoxIndex].ymin));
    // center_x = (boxs[biggestBoxIndex].xmin+boxs[biggestBoxIndex].xmax)/2;
    // center_y = (boxs[biggestBoxIndex].ymin+boxs[biggestBoxIndex].ymax)/2;
    // width = (boxs[biggestBoxIndex].xmax-boxs[biggestBoxIndex].xmin);
    // height = (boxs[biggestBoxIndex].ymax-boxs[biggestBoxIndex].ymin);
    
    //UartSend(uartFd, cat_data, strlen(cat_data));
    /*
     * 当检测到对象时，在DSTFRM中绘制一个矩形
     * When an object is detected, a rectangle is drawnd in the DSTFRM
     */
    // if (biggestBoxIndex >= 0) {
    //     objBoxs[0] = boxs[biggestBoxIndex];
    //     MppFrmDrawRects(dstFrm, objBoxs, 1, RGB888_GREEN, DRAW_RETC_THICK); // Target hand objnum is equal to 1

    //     for (int j = 0; (j < objNum) && (objNum > 1); j++) {
    //         if (j != biggestBoxIndex) {
    //             remainingBoxs[num++] = boxs[j];
    //             /*
    //              * 其他手objnum等于objnum -1
    //              * Others hand objnum is equal to objnum -1
    //              */
    //             MppFrmDrawRects(dstFrm, remainingBoxs, objNum - 1, RGB888_RED, DRAW_RETC_THICK);
    //         }
    //     }
    // }
    if (bestScoreIndex >= 0 && scores[bestScoreIndex] > 0.75) {
        center_x = (boxs[bestScoreIndex].xmin+boxs[bestScoreIndex].xmax)/2;
        center_y = (boxs[bestScoreIndex].ymin+boxs[bestScoreIndex].ymax)/2;
        width = (boxs[bestScoreIndex].xmax-boxs[bestScoreIndex].xmin);
        height = (boxs[bestScoreIndex].ymax-boxs[bestScoreIndex].ymin);
        
        packData(cat_data, center_x, center_y);
        if(HI_SUCCESS != HisignallingMsgSend(uartFd, cat_data, sizeof(cat_data)/sizeof(cat_data[0]))){
            SAMPLE_PRT("HisignallingMsgSend failed\n");
        }
        objBoxs[0] = boxs[bestScoreIndex];
        MppFrmDrawRects(dstFrm, objBoxs, 1, RGB888_GREEN, DRAW_RETC_THICK); // Target hand objnum is equal to 1

        for (int j = 0; (j < objNum) && (objNum > 1); j++) {
            if (j != bestScoreIndex) {
                remainingBoxs[num++] = boxs[j];
                /*
                 * 其他手objnum等于objnum -1
                 * Others hand objnum is equal to objnum -1
                 */
                MppFrmDrawRects(dstFrm, remainingBoxs, objNum - 1, RGB888_RED, DRAW_RETC_THICK);
            }
        }
        printf("bestScore:%f\n", scores[bestScoreIndex]);
    }
    else{
        packData(cat_data, 0, 0);
        if(HI_SUCCESS != HisignallingMsgSend(uartFd, cat_data, sizeof(cat_data)/sizeof(cat_data[0]))){
            SAMPLE_PRT("HisignallingMsgSend failed\n");
        }
    }
    
    ret = 0;
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
