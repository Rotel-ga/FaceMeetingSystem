#pragma once

#include "../include/sqlite_modern_cpp.h"
#include <string>
#include <vector>
#include <memory>

struct User {
    int id;
    std::string username;
    std::string password;
    std::string face_token;
};

struct Meeting {
    int id;
    int room_id;
    std::string topic;
    std::string time_start;
    std::string time_end;
    int user_id;
};

struct Room {
    int id;
    std::string name;
};

struct CheckIn {
    int id;
    int user_id;
    int meeting_id;
    std::string checkin_time;
};

class DatabaseManager {
private:
    std::unique_ptr<sqlite::database> db;
    
public:
    explicit DatabaseManager(const std::string& db_path);
    ~DatabaseManager() = default;
    
    // 初始化数据库表
    void initialize_tables();
    
    // 用户操作
    int create_user(const std::string& username, const std::string& password, const std::string& face_token = "");
    User get_user_by_id(int id);
    User get_user_by_username(const std::string& username);
    std::vector<User> get_all_users();
    bool update_user(int id, const std::string& username, const std::string& password, const std::string& face_token);
    bool delete_user(int id);
    
    // 会议室操作
    int create_room(const std::string& name);
    Room get_room_by_id(int id);
    std::vector<Room> get_all_rooms();
    bool update_room(int id, const std::string& name);
    bool delete_room(int id);
    
    // 会议操作
    int create_meeting(int room_id, const std::string& topic, const std::string& time_start, 
                      const std::string& time_end, int user_id);
    Meeting get_meeting_by_id(int id);
    std::vector<Meeting> get_meetings_by_room(int room_id);
    std::vector<Meeting> get_meetings_by_user(int user_id);
    std::vector<Meeting> get_all_meetings();
    bool update_meeting(int id, int room_id, const std::string& topic, 
                       const std::string& time_start, const std::string& time_end, int user_id);
    bool delete_meeting(int id);
    
    // 签到操作
    int create_checkin(int user_id, int meeting_id, const std::string& checkin_time);
    CheckIn get_checkin_by_id(int id);
    std::vector<CheckIn> get_checkins_by_meeting(int meeting_id);
    std::vector<CheckIn> get_checkins_by_user(int user_id);
    std::vector<CheckIn> get_all_checkins();
    bool delete_checkin(int id);
};