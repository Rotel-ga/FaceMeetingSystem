# 人脸识别签到API文档

## 接口概述

本文档描述了人脸识别签到系统的API接口，该接口通过调用百度人脸识别API实现自动签到功能。

## API接口

### 人脸识别签到

**接口地址：** `POST /api/checkin`

**功能描述：** 通过上传人脸图像进行身份识别，并自动完成会议签到

#### 请求参数

**Content-Type:** `application/json`

**请求体参数：**

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| image | string | 是 | Base64编码的人脸图像数据（不包含data:image前缀） |
| access_token | string | 是 | 百度人脸识别API的访问令牌 |

**请求示例：**

```json
{
  "image": "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAYEBQYFBAYGBQYHBwYIChAKCgkJChQODwwQFxQYGBcUFhYaHSUfGhsjHBYWICwgIyYnKSopGR8tMC0oMCUoKSj/2wBDAQcHBwoIChMKChMoGhYaKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCj/wAARCAABAAEDASIAAhEBAxEB/8QAFQABAQAAAAAAAAAAAAAAAAAAAAv/xAAUEAEAAAAAAAAAAAAAAAAAAAAA/8QAFQEBAQAAAAAAAAAAAAAAAAAAAAX/xAAUEQEAAAAAAAAAAAAAAAAAAAAA/9oADAMBAAIRAxEAPwCdABmX/9k=",
  "access_token": "your_baidu_access_token_here"
}
```

#### 响应参数

**成功响应 (200)：**

```json
{
  "success": true,
  "message": "Check-in successful",
  "data": {
    "checkin_id": 123,
    "user_id": 1,
    "username": "张三",
    "meeting_id": 45,
    "meeting_topic": "项目讨论会",
    "room_name": "会议室A",
    "checkin_time": "2024-01-15 14:30:25",
    "recognition_score": 95.6
  }
}
```

**错误响应：**

1. **参数错误 (400)：**
```json
{
  "success": false,
  "message": "Missing required parameters: image, access_token"
}
```

2. **人脸识别失败 (404)：**
```json
{
  "success": false,
  "message": "Face not recognized or no matching user found",
  "baidu_error": "详细错误信息"
}
```

3. **识别置信度过低 (400)：**
```json
{
  "success": false,
  "message": "Face recognition confidence too low",
  "score": 65.2
}
```

4. **无活跃会议 (400)：**
```json
{
  "success": false,
  "message": "No active meetings found at current time"
}
```

5. **重复签到 (400)：**
```json
{
  "success": false,
  "message": "User has already checked in for this meeting"
}
```

6. **用户不存在 (404)：**
```json
{
  "success": false,
  "message": "User not found in database"
}
```

## 业务流程

1. **图像上传：** 会议室终端拍摄人脸照片并转换为Base64格式
2. **人脸识别：** 调用百度人脸识别API进行1:N搜索
3. **身份验证：** 根据返回的user_id匹配数据库用户
4. **会议检查：** 验证当前时间是否有活跃的已审核会议
5. **重复检查：** 确认用户未重复签到
6. **记录创建：** 成功创建签到记录

## 技术要求

### 图像要求
- **格式：** 支持PNG、JPG、JPEG、BMP（不支持GIF）
- **大小：** Base64编码后不超过2M
- **分辨率：** 建议小于1920×1080
- **质量：** 人脸清晰可见，光线充足

### 人脸库配置
- **group_id：** "test"
- **匹配阈值：** 80分
- **质量控制：** NORMAL
- **活体检测：** LOW

### 时间逻辑
- 只能在会议开始时间到结束时间范围内签到
- 只有状态为"approved"的会议允许签到
- 如有多个同时进行的会议，默认选择第一个

## 错误处理

### 常见错误及解决方案

1. **"Failed to call Baidu Face API"**
   - 检查网络连接
   - 验证access_token是否有效
   - 确认百度API服务状态

2. **"Face not recognized"**
   - 确保人脸图像清晰
   - 检查用户是否已在人脸库中注册
   - 验证group_id配置

3. **"Face recognition confidence too low"**
   - 重新拍摄更清晰的照片
   - 确保光线充足
   - 检查人脸库中的注册照片质量

4. **"No active meetings found"**
   - 确认当前时间在会议时间范围内
   - 检查会议状态是否为"approved"
   - 验证会议时间格式

## 安全考虑

- access_token应妥善保管，避免泄露
- 建议定期更新access_token
- 图像数据传输建议使用HTTPS
- 人脸识别阈值可根据安全需求调整

## 测试建议

1. 使用真实人脸图像进行测试
2. 测试不同光线条件下的识别效果
3. 验证边界情况（会议开始前/后、重复签到等）
4. 测试网络异常情况的处理

## 版本信息

- **API版本：** v1.0
- **文档更新时间：** 2024-01-15
- **百度人脸识别API版本：** v3