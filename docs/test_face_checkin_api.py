#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
人脸识别签到API测试脚本

使用说明：
1. 安装依赖：pip install requests
2. 修改配置参数（API_BASE_URL, ACCESS_TOKEN等）
3. 准备测试图片并转换为base64
4. 运行脚本：python test_face_checkin_api.py
"""

import requests
import json
import base64
import os
from datetime import datetime

# 配置参数
API_BASE_URL = "http://localhost:18080"
ACCESS_TOKEN = "your_baidu_access_token_here"  # 请替换为实际的百度API访问令牌
TEST_IMAGE_PATH = "test_face.jpg"  # 测试图片路径

def image_to_base64(image_path):
    """
    将图片文件转换为base64字符串
    """
    try:
        with open(image_path, "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read()).decode('utf-8')
            return encoded_string
    except FileNotFoundError:
        print(f"错误：找不到图片文件 {image_path}")
        return None
    except Exception as e:
        print(f"错误：读取图片文件失败 - {e}")
        return None

def test_face_checkin(image_base64, access_token):
    """
    测试人脸识别签到API
    """
    url = f"{API_BASE_URL}/api/checkin"
    
    payload = {
        "image": image_base64,
        "access_token": access_token
    }
    
    headers = {
        "Content-Type": "application/json"
    }
    
    try:
        print(f"正在调用API: {url}")
        print(f"请求时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("正在发送请求...")
        
        response = requests.post(url, json=payload, headers=headers, timeout=30)
        
        print(f"响应状态码: {response.status_code}")
        print(f"响应头: {dict(response.headers)}")
        
        try:
            response_data = response.json()
            print("响应内容:")
            print(json.dumps(response_data, indent=2, ensure_ascii=False))
            
            if response.status_code == 200 and response_data.get("success"):
                print("\n✅ 签到成功！")
                data = response_data.get("data", {})
                print(f"用户: {data.get('username')}")
                print(f"会议: {data.get('meeting_topic')}")
                print(f"会议室: {data.get('room_name')}")
                print(f"签到时间: {data.get('checkin_time')}")
                print(f"识别分数: {data.get('recognition_score')}")
            else:
                print("\n❌ 签到失败！")
                print(f"错误信息: {response_data.get('message', '未知错误')}")
                
        except json.JSONDecodeError:
            print("响应内容（非JSON格式）:")
            print(response.text)
            
    except requests.exceptions.Timeout:
        print("❌ 请求超时")
    except requests.exceptions.ConnectionError:
        print("❌ 连接错误，请检查服务器是否运行")
    except Exception as e:
        print(f"❌ 请求失败: {e}")

def test_server_status():
    """
    测试服务器状态
    """
    try:
        url = f"{API_BASE_URL}/"
        response = requests.get(url, timeout=5)
        if response.status_code == 200:
            print("✅ 服务器运行正常")
            print(f"服务器响应: {response.text}")
            return True
        else:
            print(f"❌ 服务器响应异常，状态码: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ 无法连接到服务器: {e}")
        return False

def main():
    """
    主函数
    """
    print("=" * 50)
    print("人脸识别签到API测试脚本")
    print("=" * 50)
    
    # 检查服务器状态
    print("\n1. 检查服务器状态...")
    if not test_server_status():
        print("请确保后端服务器正在运行")
        return
    
    # 检查访问令牌
    print("\n2. 检查配置...")
    if ACCESS_TOKEN == "your_baidu_access_token_here":
        print("❌ 请先配置百度API访问令牌")
        print("请在脚本中修改 ACCESS_TOKEN 变量")
        return
    
    # 检查测试图片
    print("\n3. 准备测试图片...")
    if not os.path.exists(TEST_IMAGE_PATH):
        print(f"❌ 找不到测试图片: {TEST_IMAGE_PATH}")
        print("请准备一张人脸照片并修改 TEST_IMAGE_PATH 变量")
        return
    
    # 转换图片为base64
    print(f"正在读取图片: {TEST_IMAGE_PATH}")
    image_base64 = image_to_base64(TEST_IMAGE_PATH)
    if not image_base64:
        return
    
    print(f"图片大小: {len(image_base64)} 字符")
    print(f"Base64前缀: {image_base64[:50]}...")
    
    # 执行测试
    print("\n4. 执行人脸识别签到测试...")
    test_face_checkin(image_base64, ACCESS_TOKEN)
    
    print("\n=" * 50)
    print("测试完成")
    print("=" * 50)

if __name__ == "__main__":
    main()