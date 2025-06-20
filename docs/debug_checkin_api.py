#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
人脸识别签到API调试脚本
用于测试和调试 /api/checkin 接口的数据格式问题
"""

import requests
import json
import base64
import os

# API配置
API_BASE_URL = "http://localhost:18080"
CHECKIN_URL = f"{API_BASE_URL}/api/checkin"

# 百度API访问令牌
ACCESS_TOKEN = "24.1ad997f6ce51decc95501881db688263.2592000.1752998641.282335-119289764"

def load_test_image():
    """加载测试图片并转换为base64"""
    # 尝试加载测试图片
    test_image_paths = [
        "../backend/assets/user1.jpg",
        "../backend/assets/admin.png",
        "../backend/assets/IMG_6827.JPG"
    ]
    
    for image_path in test_image_paths:
        if os.path.exists(image_path):
            with open(image_path, "rb") as f:
                image_data = f.read()
                base64_data = base64.b64encode(image_data).decode('utf-8')
                print(f"已加载测试图片: {image_path}")
                print(f"Base64数据长度: {len(base64_data)}")
                return base64_data
    
    print("未找到测试图片")
    return None

def test_checkin_api():
    """测试签到API"""
    print("=== 人脸识别签到API调试 ===")
    
    # 加载测试图片
    image_base64 = load_test_image()
    if not image_base64:
        print("错误: 无法加载测试图片")
        return
    
    # 构建请求数据
    request_data = {
        "image": image_base64,
        "access_token": ACCESS_TOKEN
    }
    
    print(f"\n请求URL: {CHECKIN_URL}")
    print(f"请求数据键: {list(request_data.keys())}")
    print(f"图片数据长度: {len(request_data['image'])}")
    print(f"访问令牌: {request_data['access_token'][:20]}...")
    
    try:
        # 发送请求
        print("\n发送API请求...")
        response = requests.post(
            CHECKIN_URL,
            json=request_data,
            headers={
                "Content-Type": "application/json"
            },
            timeout=30
        )
        
        print(f"\n响应状态码: {response.status_code}")
        print(f"响应头: {dict(response.headers)}")
        
        # 解析响应
        try:
            result = response.json()
            print(f"\n响应JSON:")
            print(json.dumps(result, indent=2, ensure_ascii=False))
            
            if response.status_code == 200:
                print("\n✅ 签到成功!")
                if 'data' in result:
                    data = result['data']
                    print(f"用户: {data.get('username', 'N/A')}")
                    print(f"会议: {data.get('meeting_topic', 'N/A')}")
                    print(f"房间: {data.get('room_name', 'N/A')}")
                    print(f"识别分数: {data.get('recognition_score', 'N/A')}")
            else:
                print(f"\n❌ 签到失败 (状态码: {response.status_code})")
                if 'message' in result:
                    print(f"错误信息: {result['message']}")
                if 'error_code' in result:
                    print(f"错误代码: {result['error_code']}")
                if 'error_msg' in result:
                    print(f"详细错误: {result['error_msg']}")
                if 'baidu_error' in result:
                    print(f"百度API错误: {result['baidu_error']}")
                    
        except json.JSONDecodeError as e:
            print(f"\n❌ 响应不是有效的JSON: {e}")
            print(f"原始响应: {response.text}")
            
    except requests.exceptions.ConnectionError:
        print("\n❌ 连接错误: 无法连接到后端服务器")
        print("请确保后端服务器已启动在 http://localhost:18080")
    except requests.exceptions.Timeout:
        print("\n❌ 请求超时")
    except Exception as e:
        print(f"\n❌ 请求异常: {e}")

def test_server_status():
    """测试服务器状态"""
    print("\n=== 服务器状态检查 ===")
    try:
        # 测试基本连接
        response = requests.get(f"{API_BASE_URL}/api/users", timeout=5)
        print(f"服务器状态: {response.status_code}")
        if response.status_code == 200:
            print("✅ 后端服务器运行正常")
        else:
            print(f"⚠️ 服务器响应异常: {response.status_code}")
    except Exception as e:
        print(f"❌ 无法连接到服务器: {e}")

if __name__ == "__main__":
    test_server_status()
    test_checkin_api()
    
    print("\n=== 调试建议 ===")
    print("1. 检查后端服务器日志中的 'Baidu API Response' 输出")
    print("2. 确认百度API访问令牌是否有效")
    print("3. 检查人脸库中是否有对应的用户数据")
    print("4. 确认当前时间是否有进行中的会议")
    print("5. 查看浏览器开发者工具的网络请求详情")