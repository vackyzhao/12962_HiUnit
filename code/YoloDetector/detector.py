# -*- coding: gbk -*-

import sys, os
sys.path.append(os.path.join(os.getcwd(),'python/'))

import darknet as dn
import pdb
import cv2

def draw_box(image_path, detections):
    # 加载图像
    image = cv2.imread(image_path)
    
    # 遍历每个检测结果
    for detection in detections:
        # 提取出类别名、置信度和边界框参数
        class_name, confidence, (x_center, y_center, width, height) = detection

        # 转换边界框参数为左上角和右下角的坐标
        x_min = int(x_center - width / 2)
        y_min = int(y_center - height / 2)
        x_max = int(x_center + width / 2)
        y_max = int(y_center + height / 2)

        # 画出边界框
        cv2.rectangle(image, (x_min, y_min), (x_max, y_max), (0, 255, 0), 2)

        # 添加类别名和置信度标签
        cv2.putText(image, "{} [{:.2f}]".format(class_name.decode(), confidence), 
                    (x_min, y_min - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, 
                    (0, 255, 0), 2)

    base_name = os.path.basename(image_path)
    result_path = os.path.join("results", base_name)
    cv2.imwrite(result_path, image)


if __name__ == "__main__":

  dn.set_gpu(0)
  net = dn.load_net(b"/home/nicta100-s16/ai/framework/darknet-master/cfg/darknet53.cfg", b"/home/nicta100-s16/ai/framework/darknet-master/backup2/darknet53_400000.weights", 0)
  meta = dn.load_meta(b"/home/nicta100-s16/ai/framework/darknet-master/hand.data")
  
  """
  r = dn.detect(net, meta, b"/home/nicta100-s16/ai/framework/darknet-master/dog.jpg")
  print(r)
  draw_box("/home/nicta100-s16/ai/framework/darknet-master/dog.jpg",r)
  # And then down here you could detect a lot more images like
  """
  r = dn.detect(net, meta, b"/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000004795.jpg")
  print(r)
  draw_box("/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000004795.jpg",r)
  r = dn.detect(net, meta, b"/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000010363.jpg")
  print(r)
  draw_box("/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000010363.jpg",r)
  r = dn.detect(net, meta, b"/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000014007.jpg")
  print(r)
  draw_box("/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000014007.jpg",r)
  r = dn.detect(net, meta, b"/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000014831.jpg")
  print(r)
  draw_box("/home/nicta100-s16/ai/cat_dataset/test_dataset/images/000000014831.jpg",r)
  print("finished!")
  


