你已成功完成了前端页面开发及后端数据库、REST API 的关键基础工作，接下来正式进入 **阶段三：接口设计与实现（REST API）** 的核心开发阶段。

### ✅ 已完成基础

- 数据库结构 ✔️
- SQLite3 封装 + CRUD 接口 ✔️
- CMake 配置和 Crow 框架 ✔️
- Vue 前端页面 ✔️
- 基本 REST API 接入 ✔️

------

## 📌 阶段三目标：RESTful 接口设计与实现

### 一、总目标

构建与前端页面功能一一对应的 RESTful API 接口，完成数据交互、后端逻辑处理与百度人脸识别集成。

------

### 🧩 接口开发任务清单（共 9 类）

#### 1️⃣ 用户注册与登录接口

-  `POST /api/register` ：注册新用户
-  `POST /api/login` ：验证用户名密码，返回状态与用户信息
-  （可选）Token 登录态机制（前期可先跳过）

#### 2️⃣ 上传人脸图像接口

-  `POST /api/face/register` ：上传图片并注册 face_token（调用百度API）
-  `POST /api/face/recognize` ：上传图片并识别人脸，返回匹配结果

#### 3️⃣ 用户管理接口（管理员）

-  `GET /api/users` ：获取所有用户列表
-  `POST /api/users` ：添加用户
-  `PUT /api/users/:id` ：编辑用户信息
-  `DELETE /api/users/:id` ：删除用户

#### 4️⃣ 会议室管理接口

-  `GET /api/rooms` ：获取所有会议室
-  `POST /api/rooms` ：添加会议室
-  `PUT /api/rooms/:id` ：修改会议室
-  `DELETE /api/rooms/:id` ：删除会议室

#### 5️⃣ 查询会议室接口

-  `GET /api/rooms/available?start=xxx&end=xxx` ：根据时间段查询可用会议室

#### 6️⃣ 预约会议接口

-  `POST /api/bookings` ：提交预约申请
-  `GET /api/bookings/:user_id` ：查询某用户预约记录

#### 7️⃣ 我的预约管理

-  `DELETE /api/bookings/:id` ：取消预约

#### 8️⃣ 管理员审核接口

-  `GET /api/bookings/pending` ：获取所有未审核预约
-  `PUT /api/bookings/:id/approve` ：审批通过
-  `PUT /api/bookings/:id/reject` ：拒绝预约

#### 9️⃣ 签到接口

-  `POST /api/checkin` ：上传签到照片 → 百度识别 → 保存签到记录
-  `GET /api/checkins?meeting_id=xxx` ：查询某次会议签到记录

------

### ✍️ 技术建议

- 所有 JSON 接口返回结构统一：包含 `code`，`message`，`data`
- 使用 Crow 支持的 `crow::json::rvalue` 解析请求体
- 百度接口封装建议集中在 `baidu_face.cpp` / `baidu_client.cpp`

------

### ✅ 阶段三输出成果

| 类型     | 文件或接口                     |
| -------- | ------------------------------ |
| API接口  | `main.cpp` or `routes/*.cpp`   |
| API文档  | Markdown接口文档 or Swagger    |
| API测试  | Postman测试用例或截图          |
| 接口日志 | 可选，输出在控制台或日志文件中 |

