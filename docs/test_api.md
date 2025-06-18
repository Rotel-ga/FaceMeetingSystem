# Face Meeting System API 测试文档

## 服务器状态
服务器已成功启动在 http://localhost:18080

## API 接口测试

### 1. 基本测试
```bash
curl http://localhost:18080/
```
预期返回: "Face Meeting System API Server is running!"

### 2. 创建会议室
```bash
curl -X POST http://localhost:18080/api/rooms \
  -H "Content-Type: application/json" \
  -d '{"name":"会议室A"}'
```

### 3. 获取所有会议室
```bash
curl http://localhost:18080/api/rooms
```

### 4. 创建用户
```bash
curl -X POST http://localhost:18080/api/users \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"123456"}'
```

### 5. 获取所有用户
```bash
curl http://localhost:18080/api/users
```

### 6. 创建会议
```bash
curl -X POST http://localhost:18080/api/meetings \
  -H "Content-Type: application/json" \
  -d '{"room_id":1,"topic":"项目启动会议","time_start":"2024-01-15 09:00:00","time_end":"2024-01-15 11:00:00","user_id":1}'
```

### 7. 获取所有会议
```bash
curl http://localhost:18080/api/meetings
```

### 8. 创建签到记录
```bash
curl -X POST http://localhost:18080/api/checkins \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"meeting_id":1,"checkin_time":"2024-01-15 08:55:00"}'
```

### 9. 获取所有签到记录
```bash
curl http://localhost:18080/api/checkins
```

## 数据库表结构

### 用户表 (users)
- id: INTEGER PRIMARY KEY AUTOINCREMENT
- username: TEXT NOT NULL UNIQUE
- password: TEXT NOT NULL
- face_token: TEXT DEFAULT ''

### 会议室表 (rooms)
- id: INTEGER PRIMARY KEY AUTOINCREMENT
- name: TEXT NOT NULL UNIQUE

### 会议表 (meetings)
- id: INTEGER PRIMARY KEY AUTOINCREMENT
- room_id: INTEGER NOT NULL
- topic: TEXT NOT NULL
- time_start: TEXT NOT NULL
- time_end: TEXT NOT NULL
- user_id: INTEGER NOT NULL

### 签到记录表 (checkins)
- id: INTEGER PRIMARY KEY AUTOINCREMENT
- user_id: INTEGER NOT NULL
- meeting_id: INTEGER NOT NULL
- checkin_time: TEXT NOT NULL

## 阶段二完成情况

✅ **已完成的任务:**
1. 设计数据库结构 - 完成
2. 在 `database/` 编写 SQLite 操作封装类 - 完成
3. 编写用户增/删/查/改接口 - 完成
4. 封装会议、会议室、签到等相关表操作函数 - 完成
5. 编写数据库初始化脚本 - 完成
6. 更新 CMakeLists.txt 链接 SQLite3 库 - 完成
7. 集成到 Crow 服务器并提供 REST API - 完成

## 文件结构
```
backend/
├── include/
│   └── sqlite_modern_cpp.h          # SQLite 封装库
├── database/
│   ├── database_manager.h            # 数据库管理器头文件
│   ├── database_manager.cpp          # 数据库管理器实现
│   └── test_init_database.cpp        # 数据库测试脚本
├── src/
│   └── main.cpp                      # 主服务器程序
├── CMakeLists.txt                    # 构建配置
└── build/
    ├── FaceMeetingServer.exe         # 编译后的可执行文件
    └── face_meeting.db               # SQLite 数据库文件
```