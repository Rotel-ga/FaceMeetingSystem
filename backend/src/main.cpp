#define CROW_USE_ASIO
#define CROW_DISABLE_SSL
#include "crow_all.h"
#include "../database/database_manager.h"
#include "../include/cors.h"
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>

std::unique_ptr<DatabaseManager> db_manager;

// libcurl回调函数，用于接收HTTP响应数据
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 调用百度人脸识别API获取face_token
std::string callBaiduFaceAPI(const std::string& image_base64, const std::string& access_token) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        // 构建请求URL
        std::string url = "https://aip.baidubce.com/rest/2.0/face/v3/detect?access_token=" + access_token;
        
        // 构建请求体
        Json::Value json_data;
        json_data["image"] = image_base64;
        json_data["image_type"] = "BASE64";
        json_data["face_type"] = "LIVE";
        json_data["max_face_num"] = 1;
        
        Json::StreamWriterBuilder builder;
        std::string json_string = Json::writeString(builder, json_data);
        
        // 设置curl选项
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // 设置请求头
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 清理
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if(res == CURLE_OK) {
            // 解析响应
            Json::Value response;
            Json::Reader reader;
            if(reader.parse(readBuffer, response)) {
                if(response.isMember("result") && response["result"].isMember("face_list") && 
                   response["result"]["face_list"].isArray() && response["result"]["face_list"].size() > 0) {
                    return response["result"]["face_list"][0]["face_token"].asString();
                }
            }
        }
    }
    
    return ""; // 返回空字符串表示失败
}

// 调用百度人脸搜索API进行人脸识别
std::string callBaiduFaceSearchAPI(const std::string& image_base64, const std::string& access_token) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        // 构建请求URL
        std::string url = "https://aip.baidubce.com/rest/2.0/face/v3/search?access_token=" + access_token;
        
        // 构建请求体
        Json::Value json_data;
        json_data["image"] = image_base64;
        json_data["image_type"] = "BASE64";
        json_data["group_id_list"] = "test";
        json_data["quality_control"] = "LOW";
        json_data["liveness_control"] = "NONE";
        json_data["match_threshold"] = 60;
        
        Json::StreamWriterBuilder builder;
        std::string json_string = Json::writeString(builder, json_data);
        
        // 设置curl选项
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // 设置请求头
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 清理
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if(res == CURLE_OK) {
            return readBuffer;
        }
    }
    
    return ""; // 返回空字符串表示失败
}

// 获取当前时间字符串
std::string getCurrentTimeString() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// 检查当前时间是否在会议时间范围内
bool isWithinMeetingTime(const std::string& meeting_start, const std::string& meeting_end) {
    std::string current_time = getCurrentTimeString();
    return current_time >= meeting_start && current_time <= meeting_end;
}

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

        // 根据ID获取用户信息
        CROW_ROUTE(app, "/api/users/<int>").methods("GET"_method)([](const crow::request& req, int user_id){
            try {
                auto user = db_manager->get_user_by_id(user_id);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["data"]["id"] = user.id;
                result["data"]["username"] = user.username;
                // 不返回密码和face_token等敏感信息
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "User not found";
                return crow::response(404, error);
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

        // 人脸注册接口
        CROW_ROUTE(app, "/api/face/register").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                if (!body.has("user_id") || !body.has("image")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "user_id and image are required";
                    return crow::response(400, error);
                }
                
                int user_id = body["user_id"].i();
                std::string image_base64 = body["image"].s();
                
                // 检查用户是否存在
                User user;
                try {
                    user = db_manager->get_user_by_id(user_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "User not found";
                    return crow::response(404, error);
                }
                
                // 调用百度人脸识别API获取face_token
                std::string access_token = "24.1ad997f6ce51decc95501881db688263.2592000.1752998641.282335-119289764";
                std::string face_token = callBaiduFaceAPI(image_base64, access_token);
                
                if (face_token.empty()) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to register face with Baidu API";
                    return crow::response(500, error);
                }
                
                // 更新用户的face_token
                bool success = db_manager->update_user(user_id, user.username, user.password, face_token);
                
                if (success) {
                    crow::json::wvalue result;
                    result["success"] = true;
                    result["data"]["face_token"] = face_token;
                    result["message"] = "Face registered successfully";
                    return crow::response(200, result);
                } else {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to update user face_token";
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

        // 根据ID获取会议室
        CROW_ROUTE(app, "/api/rooms/<int>").methods("GET"_method)([](const crow::request& req, int room_id){
            try {
                auto room = db_manager->get_room_by_id(room_id);
                
                crow::json::wvalue result;
                result["success"] = true;
                crow::json::wvalue room_json;
                room_json["id"] = room.id;
                room_json["name"] = room.name;
                result["data"] = std::move(room_json);
                
                return crow::response(200, result);
            } catch (const std::exception& e) {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "Room not found";
                return crow::response(404, error);
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

        // 根据会议ID获取签到记录
        CROW_ROUTE(app, "/api/checkins/meeting/<int>").methods("GET"_method)([](const crow::request& req, int meeting_id){
            try {
                auto checkins = db_manager->get_checkins_by_meeting(meeting_id);
                crow::json::wvalue result;
                result["success"] = true;
                result["data"] = crow::json::wvalue::list();
                
                for (size_t i = 0; i < checkins.size(); ++i) {
                    crow::json::wvalue checkin_json;
                    checkin_json["id"] = checkins[i].id;
                    checkin_json["user_id"] = checkins[i].user_id;
                    checkin_json["meeting_id"] = checkins[i].meeting_id;
                    checkin_json["checkin_time"] = checkins[i].checkin_time;
                    
                    // 获取用户信息
                    try {
                        auto user = db_manager->get_user_by_id(checkins[i].user_id);
                        checkin_json["username"] = user.username;
                    } catch (...) {
                        checkin_json["username"] = "Unknown";
                    }
                    
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

        // 人脸识别签到接口
        CROW_ROUTE(app, "/api/checkin").methods("POST"_method)([](const crow::request& req){
            try {
                auto body = crow::json::load(req.body);
                if (!body) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid JSON";
                    return crow::response(400, error);
                }
                
                // 检查必需参数
                if (!body.has("image") || !body.has("access_token")) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Missing required parameters: image, access_token";
                    return crow::response(400, error);
                }
                
                std::string image_base64 = body["image"].s();
                std::string access_token = body["access_token"].s();
                
                // 调用百度人脸搜索API
                std::string api_response = callBaiduFaceSearchAPI(image_base64, access_token);
                
                if (api_response.empty()) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to call Baidu Face API";
                    return crow::response(500, error);
                }
                
                // 添加调试日志
                std::cout << "Baidu API Response: " << api_response << std::endl;
                
                // 解析百度API响应
                Json::Value response;
                Json::Reader reader;
                if (!reader.parse(api_response, response)) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Failed to parse Baidu API response";
                    error["raw_response"] = api_response;
                    return crow::response(500, error);
                }
                
                // 检查百度API是否返回错误（error_code为0表示成功）
                if (response.isMember("error_code") && response["error_code"].asInt() != 0) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Baidu API error";
                    error["error_code"] = response["error_code"].asInt();
                    if (response.isMember("error_msg")) {
                        error["error_msg"] = response["error_msg"].asString();
                    }
                    return crow::response(400, error);
                }
                
                // 检查API调用是否成功
                if (!response.isMember("result") || !response["result"].isMember("user_list") || 
                    !response["result"]["user_list"].isArray() || response["result"]["user_list"].size() == 0) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Face not recognized or no matching user found";
                    if (response.isMember("error_msg")) {
                        error["baidu_error"] = response["error_msg"].asString();
                    }
                    return crow::response(404, error);
                }
                
                // 获取识别到的用户ID和匹配分数
                std::string recognized_user_id = response["result"]["user_list"][0]["user_id"].asString();
                float score = response["result"]["user_list"][0]["score"].asFloat();
                
                // 检查匹配分数是否达到阈值
                if (score < 60.0) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Face recognition confidence too low";
                    error["score"] = score;
                    return crow::response(400, error);
                }
                
                // 将字符串user_id转换为整数
                int user_id;
                try {
                    user_id = std::stoi(recognized_user_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "Invalid user_id format from face recognition";
                    return crow::response(400, error);
                }
                
                // 验证用户是否存在
                try {
                    auto user = db_manager->get_user_by_id(user_id);
                } catch (const std::exception& e) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "User not found in database";
                    return crow::response(404, error);
                }
                
                // 获取当前时间
                std::string current_time = getCurrentTimeString();
                
                // 查找当前时间段内的会议
                auto all_meetings = db_manager->get_all_meetings();
                std::vector<Meeting> current_meetings;
                
                for (const auto& meeting : all_meetings) {
                    if (meeting.status == "approved" && isWithinMeetingTime(meeting.time_start, meeting.time_end)) {
                        current_meetings.push_back(meeting);
                    }
                }
                
                if (current_meetings.empty()) {
                    crow::json::wvalue error;
                    error["success"] = false;
                    error["message"] = "No active meetings found at current time";
                    return crow::response(400, error);
                }
                
                // 如果有多个会议，选择第一个（可以根据需要调整逻辑）
                Meeting target_meeting = current_meetings[0];
                
                // 检查用户是否已经签到过
                auto existing_checkins = db_manager->get_checkins_by_meeting(target_meeting.id);
                for (const auto& checkin : existing_checkins) {
                    if (checkin.user_id == user_id) {
                        crow::json::wvalue error;
                        error["success"] = false;
                        error["message"] = "User has already checked in for this meeting";
                        return crow::response(400, error);
                    }
                }
                
                // 创建签到记录
                int checkin_id = db_manager->create_checkin(user_id, target_meeting.id, current_time);
                
                // 获取用户信息用于返回
                auto user = db_manager->get_user_by_id(user_id);
                auto room = db_manager->get_room_by_id(target_meeting.room_id);
                
                crow::json::wvalue result;
                result["success"] = true;
                result["message"] = "Check-in successful";
                result["data"]["checkin_id"] = checkin_id;
                result["data"]["user_id"] = user_id;
                result["data"]["username"] = user.username;
                result["data"]["meeting_id"] = target_meeting.id;
                result["data"]["meeting_topic"] = target_meeting.topic;
                result["data"]["room_name"] = room.name;
                result["data"]["checkin_time"] = current_time;
                result["data"]["recognition_score"] = score;
                
                return crow::response(200, result);
                
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
