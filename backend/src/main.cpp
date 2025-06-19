#define CROW_USE_ASIO
#define CROW_DISABLE_SSL
#include "crow_all.h"
#include "../database/database_manager.h"
#include "../include/cors.h"
#include <memory>
#include <iostream>

std::unique_ptr<DatabaseManager> db_manager;

int main()
{
    try {
        // 初始化数据库
        db_manager = std::make_unique<DatabaseManager>("face_meeting.db");
        db_manager->initialize_tables();
        std::cout << "数据库初始化成功!" << std::endl;
        
        crow::App<CORS> app;

        // 基本测试接口
        CROW_ROUTE(app, "/")([](){
            return "Face Meeting System API Server is running!";
        });



        // 用户登录接口
        CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("username") || !body.has("password")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Username and password are required";
                    return crow::response(400, error);
                }
                
                std::string username = body["username"].s();
                std::string password = body["password"].s();
                
                try {
                    // 根据用户名查找用户
                    User user = db_manager->get_user_by_username(username);
                    
                    // 验证密码
                    if (user.password == password) {
                        // 登录成功
                        crow::json::wvalue result;
                        result["success"] = true;
                        result["message"] = "Login successful";
                        result["data"]["id"] = user.id;
                        result["data"]["username"] = user.username;
                        result["data"]["face_token"] = user.face_token;
                        
                        // 根据用户名判断角色（简单实现：admin用户为管理员）
                        if (username == "admin") {
                            result["data"]["role"] = "admin";
                        } else {
                            result["data"]["role"] = "user";
                        }
                        
                        return crow::response(200, result);
                    } else {
                        // 密码错误
                        crow::json::wvalue error;
                        error["success"] = false;
                        error["message"] = "Invalid username or password";
                        return crow::response(401, error);
                    }
                } catch (const std::exception& e) {
                    // 用户不存在或其他数据库错误
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid username or password";
                    return crow::response(401, error);
                }
                
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "Internal server error";
                return crow::response(500, error);
            }
        });



        // 获取所有用户
        CROW_ROUTE(app, "/api/users").methods("GET"_method)([](const crow::request& req){
            try {
                auto users = db_manager->get_all_users();
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < users.size(); ++i) {
                    crow::json::wvalue user_json;
                    user_json["id"] = users[i].id;
                    user_json["username"] = users[i].username;
                    user_json["face_token"] = users[i].face_token;
                    result["data"][i] = std::move(user_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 创建用户
        CROW_ROUTE(app, "/api/users").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("username") || !body.has("password")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Username and password are required";
                    return crow::response(400, error);
                }
                
                std::string username = body["username"].s();
                std::string password = body["password"].s();
                std::string face_token = body.has("face_token") ? body["face_token"].s() : std::string();
                
                // 检查用户名是否已存在
                try {
                    db_manager->get_user_by_username(username);
                    // 如果没有抛出异常，说明用户已存在
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Username already exists";
                    return crow::response(409, error);
                } catch (const std::exception& e) {
                    // 用户不存在，可以创建
                }
                
                int user_id = db_manager->create_user(username, password, face_token);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = user_id;
                result["data"]["username"] = username;
                result["message"] = "User created successfully";
                
                return crow::response(201, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 更新用户
        CROW_ROUTE(app, "/api/users/<int>").methods("PUT"_method)([](const crow::request& req, int user_id){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("username") || !body.has("password")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Username and password are required";
                    return crow::response(400, error);
                }
                
                std::string username = body["username"].s();
                std::string password = body["password"].s();
                std::string face_token = body.has("face_token") ? body["face_token"].s() : std::string();
                
                // 检查用户是否存在
                try {
                    db_manager->get_user_by_id(user_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "User not found";
                    return crow::response(404, error);
                }
                
                bool success = db_manager->update_user(user_id, username, password, face_token);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["message"] = "User updated successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to update user";
                    return crow::response(500, error);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 删除用户
        CROW_ROUTE(app, "/api/users/<int>").methods("DELETE"_method)([](const crow::request& req, int user_id){
            try {
                // 检查用户是否存在
                try {
                    db_manager->get_user_by_id(user_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "User not found";
                    return crow::response(404, error);
                }
                
                bool success = db_manager->delete_user(user_id);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["message"] = "User deleted successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to delete user";
                    return crow::response(500, error);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 获取所有会议室
        CROW_ROUTE(app, "/api/rooms").methods("GET"_method)([](const crow::request& req){
            try {
                auto rooms = db_manager->get_all_rooms();
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < rooms.size(); ++i) {
                    crow::json::wvalue room_json;
                    room_json["id"] = rooms[i].id;
                    room_json["name"] = rooms[i].name;
                    result["data"][i] = std::move(room_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 创建会议室
        CROW_ROUTE(app, "/api/rooms").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                std::string name = body["name"].s();
                int room_id = db_manager->create_room(name);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = room_id;
                result["message"] = "Room created successfully";
                
                return crow::response(201, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 更新会议室
        CROW_ROUTE(app, "/api/rooms/<int>").methods("PUT"_method)([](const crow::request& req, int room_id){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("name")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Room name is required";
                    return crow::response(400, error);
                }
                
                std::string name = body["name"].s();
                
                // 检查会议室是否存在
                try {
                    db_manager->get_room_by_id(room_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Room not found";
                    return crow::response(404, error);
                }
                
                bool success = db_manager->update_room(room_id, name);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["message"] = "Room updated successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to update room";
                    return crow::response(500, error);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 删除会议室
        CROW_ROUTE(app, "/api/rooms/<int>").methods("DELETE"_method)([](const crow::request& req, int room_id){
            try {
                // 检查会议室是否存在
                try {
                    db_manager->get_room_by_id(room_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Room not found";
                    return crow::response(404, error);
                }
                
                bool success = db_manager->delete_room(room_id);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["message"] = "Room deleted successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to delete room";
                    return crow::response(500, error);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 获取所有会议
        CROW_ROUTE(app, "/api/meetings").methods("GET"_method)([](const crow::request& req){
            try {
                auto meetings = db_manager->get_all_meetings();
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < meetings.size(); ++i) {
                    crow::json::wvalue meeting_json;
                    meeting_json["id"] = meetings[i].id;
                    meeting_json["room_id"] = meetings[i].room_id;
                    meeting_json["topic"] = meetings[i].topic;
                    meeting_json["time_start"] = meetings[i].time_start;
                    meeting_json["time_end"] = meetings[i].time_end;
                    meeting_json["user_id"] = meetings[i].user_id;
                    meeting_json["status"] = meetings[i].status;
                    result["data"][i] = std::move(meeting_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 创建会议
        CROW_ROUTE(app, "/api/meetings").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                int room_id = body["room_id"].i();
                std::string topic = body["topic"].s();
                std::string time_start = body["time_start"].s();
                std::string time_end = body["time_end"].s();
                int user_id = body["user_id"].i();
                
                int meeting_id = db_manager->create_meeting(room_id, topic, time_start, time_end, user_id);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = meeting_id;
                result["message"] = "Meeting created successfully";
                
                return crow::response(201, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 更新会议
        CROW_ROUTE(app, "/api/meetings/<int>").methods("PUT"_method)([](const crow::request& req, int meeting_id){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                int room_id = body["room_id"].i();
                std::string topic = body["topic"].s();
                std::string time_start = body["time_start"].s();
                std::string time_end = body["time_end"].s();
                int user_id = body["user_id"].i();
                
                bool success = db_manager->update_meeting(meeting_id, room_id, topic, time_start, time_end, user_id);
                
                crow::json::wvalue result;
                if (success) {
                    result["success"] = true;
                    result["message"] = "Meeting updated successfully";
                    return crow::response(200, result);
                } else {
                    result["success"] = false;
                    result["message"] = "Meeting not found";
                    return crow::response(404, result);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 删除会议
        CROW_ROUTE(app, "/api/meetings/<int>").methods("DELETE"_method)([](const crow::request& req, int meeting_id){
            try {
                bool success = db_manager->delete_meeting(meeting_id);
                
                crow::json::wvalue result;
                if (success) {
                    result["success"] = true;
                    result["message"] = "Meeting deleted successfully";
                    return crow::response(200, result);
                } else {
                    result["success"] = false;
                    result["message"] = "Meeting not found";
                    return crow::response(404, result);
                }
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 根据会议室ID和日期查询会议
        CROW_ROUTE(app, "/api/meetings/room/<int>").methods("GET"_method)([](const crow::request& req, int room_id){
            try {
                std::string date = req.url_params.get("date") ? req.url_params.get("date") : "";
                
                auto meetings = db_manager->get_meetings_by_room(room_id);
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                size_t count = 0;
                for (const auto& meeting : meetings) {
                    // 如果指定了日期，则过滤
                    if (!date.empty()) {
                        std::string meeting_date = meeting.time_start.substr(0, 10); // 提取日期部分 YYYY-MM-DD
                        if (meeting_date != date) {
                            continue;
                        }
                    }
                    
                    crow::json::wvalue meeting_json;
                    meeting_json["id"] = meeting.id;
                    meeting_json["room_id"] = meeting.room_id;
                    meeting_json["topic"] = meeting.topic;
                    meeting_json["time_start"] = meeting.time_start;
                    meeting_json["time_end"] = meeting.time_end;
                    meeting_json["user_id"] = meeting.user_id;
                    meeting_json["status"] = meeting.status;
                    result["data"][count++] = std::move(meeting_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 根据用户ID查询会议
        CROW_ROUTE(app, "/api/meetings/user/<int>").methods("GET"_method)([](const crow::request& req, int user_id){
            try {
                auto meetings = db_manager->get_meetings_by_user(user_id);
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < meetings.size(); ++i) {
                    crow::json::wvalue meeting_json;
                    meeting_json["id"] = meetings[i].id;
                    meeting_json["room_id"] = meetings[i].room_id;
                    meeting_json["topic"] = meetings[i].topic;
                    meeting_json["time_start"] = meetings[i].time_start;
                    meeting_json["time_end"] = meetings[i].time_end;
                    meeting_json["user_id"] = meetings[i].user_id;
                    meeting_json["status"] = meetings[i].status;
                    result["data"][i] = std::move(meeting_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 管理员获取待审核会议
        CROW_ROUTE(app, "/api/admin/meetings/pending").methods("GET"_method)([](const crow::request& req){
            try {
                auto meetings = db_manager->get_meetings_by_status("pending");
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < meetings.size(); ++i) {
                    crow::json::wvalue meeting_json;
                    meeting_json["id"] = meetings[i].id;
                    meeting_json["room_id"] = meetings[i].room_id;
                    meeting_json["topic"] = meetings[i].topic;
                    meeting_json["time_start"] = meetings[i].time_start;
                    meeting_json["time_end"] = meetings[i].time_end;
                    meeting_json["user_id"] = meetings[i].user_id;
                    meeting_json["status"] = meetings[i].status;
                    result["data"][i] = std::move(meeting_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 管理员审核会议（批准或拒绝）
        CROW_ROUTE(app, "/api/admin/meetings/<int>/review").methods("PUT"_method)([](const crow::request& req, int meeting_id){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("status")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Status is required";
                    return crow::response(400, error);
                }
                
                std::string status = body["status"].s();
                
                // 验证状态值
                if (status != "approved" && status != "rejected") {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid status. Must be 'approved' or 'rejected'";
                    return crow::response(400, error);
                }
                
                bool success = db_manager->update_meeting_status(meeting_id, status);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["message"] = "Meeting status updated successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to update meeting status";
                    return crow::response(500, error);
                }
                
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 管理员获取所有会议（按状态筛选）
        CROW_ROUTE(app, "/api/admin/meetings").methods("GET"_method)([](const crow::request& req){
            try {
                std::string status = req.url_params.get("status") ? req.url_params.get("status") : "";
                
                std::vector<Meeting> meetings;
                if (status.empty()) {
                    meetings = db_manager->get_all_meetings();
                } else {
                    meetings = db_manager->get_meetings_by_status(status);
                }
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < meetings.size(); ++i) {
                    crow::json::wvalue meeting_json;
                    meeting_json["id"] = meetings[i].id;
                    meeting_json["room_id"] = meetings[i].room_id;
                    meeting_json["topic"] = meetings[i].topic;
                    meeting_json["time_start"] = meetings[i].time_start;
                    meeting_json["time_end"] = meetings[i].time_end;
                    meeting_json["user_id"] = meetings[i].user_id;
                    meeting_json["status"] = meetings[i].status;
                    result["data"][i] = std::move(meeting_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 获取所有签到记录
        CROW_ROUTE(app, "/api/checkins").methods("GET"_method)([](const crow::request& req){
            try {
                auto checkins = db_manager->get_all_checkins();
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < checkins.size(); ++i) {
                    crow::json::wvalue checkin_json;
                    checkin_json["id"] = checkins[i].id;
                    checkin_json["user_id"] = checkins[i].user_id;
                    checkin_json["meeting_id"] = checkins[i].meeting_id;
                    checkin_json["checkin_time"] = checkins[i].checkin_time;
                    result["data"][i] = std::move(checkin_json);
                }
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        // 创建签到记录
        CROW_ROUTE(app, "/api/checkins").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                int user_id = body["user_id"].i();
                int meeting_id = body["meeting_id"].i();
                std::string checkin_time = body["checkin_time"].s();
                
                int checkin_id = db_manager->create_checkin(user_id, meeting_id, checkin_time);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = checkin_id;
                result["message"] = "CheckIn created successfully";
                
                return crow::response(201, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = e.what();
                return crow::response(500, error);
            }
        });

        std::cout << "Face Meeting System API Server starting on port 18080..." << std::endl;
        app.port(18080).multithreaded().run();
        
    } catch (const std::exception& e) {
        std::cerr << "服务器启动失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
