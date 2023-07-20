import os

# 替换为你想要遍历的文件夹路径
folder_path = 'D:/dataset/image_wk'
folder_linux_path = '/home/nicta100-s16/ai/cat_dataset/training_dataset/images'

# 新建的txt文件名
output_file_name = 'image_paths.txt'

# 支持的图片文件扩展名
image_extensions = ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.tiff']

# 获取文件夹中的所有文件
file_list = os.listdir(folder_path)

# 筛选出图片文件
image_files = [file for file in file_list if os.path.splitext(file)[1].lower() in image_extensions]

# 打开新的txt文件，并以追加模式写入图片路径
with open(output_file_name, 'a') as output_file:
    for image_file in image_files:
        # 使用预设的服务器路径拼接文件名
        image_path = os.path.join(folder_path, image_file)
        # 将路径中的反斜杠（\）替换为正斜杠（/）
        image_path = image_path.replace('\\', '/')
        # 将路径写入txt文件
        output_file.write(image_path + '\n')

print('所有图片的绝对路径已保存到', output_file_name)
"""

import os

# 替换为你想要遍历的文件夹路径
folder_path = 'D:/dataset/coco/coco_yolo/coco_yolo_cat/training_dataset/labels'

# 支持的文本文件扩展名
text_extensions = ['.txt']

# 获取文件夹中的所有文件
file_list = os.listdir(folder_path)

# 筛选出文本文件
text_files = [file for file in file_list if os.path.splitext(file)[1].lower() in text_extensions]

# 遍历文本文件
for text_file in text_files:
    # 获取文件的绝对路径
    file_path = os.path.join(folder_path, text_file)

    # 读取文件内容
    with open(file_path, 'r') as input_file:
        file_content = input_file.readlines()

    # 将开头的"15"替换为"0"
    modified_content = [line.replace('15', '0', 1) if line.startswith('15') else line for line in file_content]

    # 将修改后的内容写回文件
    with open(file_path, 'w') as output_file:
        output_file.writelines(modified_content)

print('已完成对所有txt文件的处理。')
"""
