# 如影随形——宠物智能跟随与远程互动小车
1. 通过3516进行目标检测，将输出的框的中心左边发送至3861，使其根据算法控制云台，令3516时刻对准宠物
2. 3516作为RTSP服务器，将实时视频流推流
3. 微信小程序进行模式切换及远程控制与互动
4. 根据各传感器信息指定跟随策略，使用户远程“近距离观察宠物”

目录：
```
- DataProcessing/
    - coco_classes_extract.py
    - ToList.py
    - README.txt

- taurus/
    - scenario/
        - hand_classify/
            - hand_classify.c
            - hand_classify.h
            - yolov2_hand_detect.c
            - yolov2_hand_detect.h
        
    - smp/
        - sample_comm_venc.c
        - sample_ai_main.cpp
        - sample_media_ai.c
        - sample_media_ai.h
        - sample_rtsp_main.c
    - README.txt
- pegasus/
    - control_demo/
        - app_demo_iot.c
        - hisignalling_protocol.c
        - hisignalling_protocol.h
- Wechat_MiniProgram/
    - TencentCloud/
        - TencentCloud.js
        - TencentCloud.json
        - TencentCloud.wxml
        - TencentCloud.wxss
- YoloDetector/
    - detector.py
```