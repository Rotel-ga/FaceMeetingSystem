#define CROW_USE_ASIO
#define CROW_DISABLE_SSL
#include "crow_all.h"
#include "../database/database_manager.h"
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
        
        crow::SimpleApp app;

        // 基本测试接口
        CROW_ROUTE(app, "/")([](){
            return "Face Meeting System API Server is running!";
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
                
                std::string username = body["username"].s();
                std::string password = body["password"].s();
                std::string face_token = body.has("face_token") ? body["face_token"].s() : std::string();
                
                int user_id = db_manager->create_user(username, password, face_token);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = user_id;
                result["message"] = "User created successfully";
                
                return crow::response(201, result);
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
