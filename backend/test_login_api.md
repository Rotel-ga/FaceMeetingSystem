# Face Meeting System 登录和用户管理 API 测试

## 服务器信息
- 后端服务器: http://localhost:18080
- 前端服务器: http://localhost:3000

## 1. 用户登录接口测试

### POST /api/login
测试用户登录功能

#### 请求示例（管理员登录）:
```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

#### 请求示例（普通用户登录）:
```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "user1",
    "password": "password123"
  }'
```

#### 预期响应（成功）:
```json
{
  "success": true,
  "message": "Login successful",
  "data": {
    "id": 1,
    "username": "admin",
    "face_token": "",
    "role": "admin"
  }
}
```

#### 预期响应（失败）:
```json
{
  "success": false,
  "message": "Invalid username or password"
}
```

## 2. 用户管理接口测试

### GET /api/users
获取所有用户列表

```bash
curl -X GET http://localhost:18080/api/users
```

### POST /api/users
创建新用户（管理员功能）

```bash
curl -X POST http://localhost:18080/api/users \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "testpass123",
    "face_token": ""
  }'
```

### PUT /api/users/{id}
更新用户信息

```bash
curl -X PUT http://localhost:18080/api/users/1 \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "newpassword123",
    "face_token": "updated_token"
  }'
```

### DELETE /api/users/{id}
删除用户

```bash
curl -X DELETE http://localhost:18080/api/users/2
```

## 3. 测试步骤建议

### 步骤1: 创建管理员用户
首先创建一个管理员用户用于测试：
```bash
curl -X POST http://localhost:18080/api/users \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

### 步骤2: 测试管理员登录
```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "admin123"
  }'
```

### 步骤3: 创建普通用户
```bash
curl -X POST http://localhost:18080/api/users \
  -H "Content-Type: application/json" \
  -d '{
    "username": "user1",
    "password": "password123"
  }'
```

### 步骤4: 测试普通用户登录
```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "user1",
    "password": "password123"
  }'
```

### 步骤5: 测试错误登录
```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "wronguser",
    "password": "wrongpass"
  }'
```

## 4. Postman 测试集合

### 环境变量设置
- `base_url`: http://localhost:18080
- `frontend_url`: http://localhost:3000

### 测试用例
1. **用户登录 - 成功**
   - Method: POST
   - URL: {{base_url}}/api/login
   - Body: {"username": "admin", "password": "admin123"}
   - 验证: status = 200, success = true

2. **用户登录 - 失败**
   - Method: POST
   - URL: {{base_url}}/api/login
   - Body: {"username": "wrong", "password": "wrong"}
   - 验证: status = 401, success = false

3. **创建用户**
   - Method: POST
   - URL: {{base_url}}/api/users
   - Body: {"username": "newuser", "password": "newpass123"}
   - 验证: status = 201, success = true

4. **获取用户列表**
   - Method: GET
   - URL: {{base_url}}/api/users
   - 验证: status = 200, success = true

5. **更新用户**
   - Method: PUT
   - URL: {{base_url}}/api/users/1
   - Body: {"username": "updated", "password": "updated123"}
   - 验证: status = 200, success = true

6. **删除用户**
   - Method: DELETE
   - URL: {{base_url}}/api/users/2
   - 验证: status = 200, success = true

## 5. 前后端集成测试

### 前端登录页面测试
1. 打开前端页面: http://localhost:3000
2. 使用创建的用户账号登录
3. 验证登录成功后的页面跳转
4. 验证用户角色权限（管理员vs普通用户）

### 前端用户管理页面测试
1. 以管理员身份登录
2. 访问用户管理页面
3. 测试添加、编辑、删除用户功能
4. 验证数据与后端API的同步

## 6. 注意事项

1. **数据库位置**: 数据库文件位于 `backend/build/face_meeting.db`
2. **角色判断**: 当前简单实现，用户名为"admin"的用户自动设为管理员角色
3. **密码安全**: 当前为明文存储，生产环境需要加密
4. **错误处理**: API已包含基本的错误处理和验证
5. **CORS**: 如需前端调用，可能需要配置CORS头

## 7. 下一步开发

1. **会议室管理接口**: 完善会议室的CRUD操作
2. **会议预约接口**: 实现会议预约相关功能
3. **人脸识别集成**: 集成百度人脸识别API
4. **权限验证**: 实现基于Token的身份验证
5. **前端集成**: 修改前端代码调用后端API