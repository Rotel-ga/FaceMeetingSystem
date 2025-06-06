#include "database_manager.h"
#include <sstream>
#include <stdexcept>

DatabaseManager::DatabaseManager(const std::string& db_path) {
    try {
        db = std::make_unique<sqlite::database>(db_path);
    } catch (const sqlite::database_error& e) {
        throw std::runtime_error("Failed to open database: " + std::string(e.what()));
    }
}

void DatabaseManager::initialize_tables() {
    // 创建用户表
    std::string create_user_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            face_token TEXT DEFAULT ''
        )
    )";
    
    // 创建会议室表
    std::string create_room_table = R"(
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        )
    )";
    
    // 创建会议表
    std::string create_meeting_table = R"(
        CREATE TABLE IF NOT EXISTS meetings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_id INTEGER NOT NULL,
            topic TEXT NOT NULL,
            time_start TEXT NOT NULL,
            time_end TEXT NOT NULL,
            user_id INTEGER NOT NULL,
            FOREIGN KEY (room_id) REFERENCES rooms(id),
            FOREIGN KEY (user_id) REFERENCES users(id)
        )
    )";
    
    // 创建签到表
    std::string create_checkin_table = R"(
        CREATE TABLE IF NOT EXISTS checkins (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            meeting_id INTEGER NOT NULL,
            checkin_time TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id),
            FOREIGN KEY (meeting_id) REFERENCES meetings(id)
        )
    )";
    
    db->execute(create_user_table);
    db->execute(create_room_table);
    db->execute(create_meeting_table);
    db->execute(create_checkin_table);
}

// 用户操作实现
int DatabaseManager::create_user(const std::string& username, const std::string& password, const std::string& face_token) {
    std::string sql = "INSERT INTO users (username, password, face_token) VALUES ('" + 
                     username + "', '" + password + "', '" + face_token + "')";
    db->execute(sql);
    return static_cast<int>(db->last_insert_rowid());
}

User DatabaseManager::get_user_by_id(int id) {
    std::string sql = "SELECT id, username, password, face_token FROM users WHERE id = " + std::to_string(id);
    auto results = db->query(sql);
    
    if (results.empty()) {
        throw std::runtime_error("User not found");
    }
    
    User user;
    user.id = std::stoi(results[0][0]);
    user.username = results[0][1];
    user.password = results[0][2];
    user.face_token = results[0][3];
    return user;
}

User DatabaseManager::get_user_by_username(const std::string& username) {
    std::string sql = "SELECT id, username, password, face_token FROM users WHERE username = '" + username + "'";
    auto results = db->query(sql);
    
    if (results.empty()) {
        throw std::runtime_error("User not found");
    }
    
    User user;
    user.id = std::stoi(results[0][0]);
    user.username = results[0][1];
    user.password = results[0][2];
    user.face_token = results[0][3];
    return user;
}

std::vector<User> DatabaseManager::get_all_users() {
    std::string sql = "SELECT id, username, password, face_token FROM users";
    auto results = db->query(sql);
    
    std::vector<User> users;
    for (const auto& row : results) {
        User user;
        user.id = std::stoi(row[0]);
        user.username = row[1];
        user.password = row[2];
        user.face_token = row[3];
        users.push_back(user);
    }
    return users;
}

bool DatabaseManager::update_user(int id, const std::string& username, const std::string& password, const std::string& face_token) {
    std::string sql = "UPDATE users SET username = '" + username + "', password = '" + 
                     password + "', face_token = '" + face_token + "' WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

bool DatabaseManager::delete_user(int id) {
    std::string sql = "DELETE FROM users WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

// 会议室操作实现
int DatabaseManager::create_room(const std::string& name) {
    std::string sql = "INSERT INTO rooms (name) VALUES ('" + name + "')";
    db->execute(sql);
    return static_cast<int>(db->last_insert_rowid());
}

Room DatabaseManager::get_room_by_id(int id) {
    std::string sql = "SELECT id, name FROM rooms WHERE id = " + std::to_string(id);
    auto results = db->query(sql);
    
    if (results.empty()) {
        throw std::runtime_error("Room not found");
    }
    
    Room room;
    room.id = std::stoi(results[0][0]);
    room.name = results[0][1];
    return room;
}

std::vector<Room> DatabaseManager::get_all_rooms() {
    std::string sql = "SELECT id, name FROM rooms";
    auto results = db->query(sql);
    
    std::vector<Room> rooms;
    for (const auto& row : results) {
        Room room;
        room.id = std::stoi(row[0]);
        room.name = row[1];
        rooms.push_back(room);
    }
    return rooms;
}

bool DatabaseManager::update_room(int id, const std::string& name) {
    std::string sql = "UPDATE rooms SET name = '" + name + "' WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

bool DatabaseManager::delete_room(int id) {
    std::string sql = "DELETE FROM rooms WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

// 会议操作实现
int DatabaseManager::create_meeting(int room_id, const std::string& topic, const std::string& time_start, 
                                   const std::string& time_end, int user_id) {
    std::string sql = "INSERT INTO meetings (room_id, topic, time_start, time_end, user_id) VALUES (" +
                     std::to_string(room_id) + ", '" + topic + "', '" + time_start + "', '" + 
                     time_end + "', " + std::to_string(user_id) + ")";
    db->execute(sql);
    return static_cast<int>(db->last_insert_rowid());
}

Meeting DatabaseManager::get_meeting_by_id(int id) {
    std::string sql = "SELECT id, room_id, topic, time_start, time_end, user_id FROM meetings WHERE id = " + std::to_string(id);
    auto results = db->query(sql);
    
    if (results.empty()) {
        throw std::runtime_error("Meeting not found");
    }
    
    Meeting meeting;
    meeting.id = std::stoi(results[0][0]);
    meeting.room_id = std::stoi(results[0][1]);
    meeting.topic = results[0][2];
    meeting.time_start = results[0][3];
    meeting.time_end = results[0][4];
    meeting.user_id = std::stoi(results[0][5]);
    return meeting;
}

std::vector<Meeting> DatabaseManager::get_meetings_by_room(int room_id) {
    std::string sql = "SELECT id, room_id, topic, time_start, time_end, user_id FROM meetings WHERE room_id = " + std::to_string(room_id);
    auto results = db->query(sql);
    
    std::vector<Meeting> meetings;
    for (const auto& row : results) {
        Meeting meeting;
        meeting.id = std::stoi(row[0]);
        meeting.room_id = std::stoi(row[1]);
        meeting.topic = row[2];
        meeting.time_start = row[3];
        meeting.time_end = row[4];
        meeting.user_id = std::stoi(row[5]);
        meetings.push_back(meeting);
    }
    return meetings;
}

std::vector<Meeting> DatabaseManager::get_meetings_by_user(int user_id) {
    std::string sql = "SELECT id, room_id, topic, time_start, time_end, user_id FROM meetings WHERE user_id = " + std::to_string(user_id);
    auto results = db->query(sql);
    
    std::vector<Meeting> meetings;
    for (const auto& row : results) {
        Meeting meeting;
        meeting.id = std::stoi(row[0]);
        meeting.room_id = std::stoi(row[1]);
        meeting.topic = row[2];
        meeting.time_start = row[3];
        meeting.time_end = row[4];
        meeting.user_id = std::stoi(row[5]);
        meetings.push_back(meeting);
    }
    return meetings;
}

std::vector<Meeting> DatabaseManager::get_all_meetings() {
    std::string sql = "SELECT id, room_id, topic, time_start, time_end, user_id FROM meetings";
    auto results = db->query(sql);
    
    std::vector<Meeting> meetings;
    for (const auto& row : results) {
        Meeting meeting;
        meeting.id = std::stoi(row[0]);
        meeting.room_id = std::stoi(row[1]);
        meeting.topic = row[2];
        meeting.time_start = row[3];
        meeting.time_end = row[4];
        meeting.user_id = std::stoi(row[5]);
        meetings.push_back(meeting);
    }
    return meetings;
}

bool DatabaseManager::update_meeting(int id, int room_id, const std::string& topic, 
                                   const std::string& time_start, const std::string& time_end, int user_id) {
    std::string sql = "UPDATE meetings SET room_id = " + std::to_string(room_id) + 
                     ", topic = '" + topic + "', time_start = '" + time_start + 
                     "', time_end = '" + time_end + "', user_id = " + std::to_string(user_id) + 
                     " WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

bool DatabaseManager::delete_meeting(int id) {
    std::string sql = "DELETE FROM meetings WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}

// 签到操作实现
int DatabaseManager::create_checkin(int user_id, int meeting_id, const std::string& checkin_time) {
    std::string sql = "INSERT INTO checkins (user_id, meeting_id, checkin_time) VALUES (" +
                     std::to_string(user_id) + ", " + std::to_string(meeting_id) + ", '" + checkin_time + "')";
    db->execute(sql);
    return static_cast<int>(db->last_insert_rowid());
}

CheckIn DatabaseManager::get_checkin_by_id(int id) {
    std::string sql = "SELECT id, user_id, meeting_id, checkin_time FROM checkins WHERE id = " + std::to_string(id);
    auto results = db->query(sql);
    
    if (results.empty()) {
        throw std::runtime_error("CheckIn not found");
    }
    
    CheckIn checkin;
    checkin.id = std::stoi(results[0][0]);
    checkin.user_id = std::stoi(results[0][1]);
    checkin.meeting_id = std::stoi(results[0][2]);
    checkin.checkin_time = results[0][3];
    return checkin;
}

std::vector<CheckIn> DatabaseManager::get_checkins_by_meeting(int meeting_id) {
    std::string sql = "SELECT id, user_id, meeting_id, checkin_time FROM checkins WHERE meeting_id = " + std::to_string(meeting_id);
    auto results = db->query(sql);
    
    std::vector<CheckIn> checkins;
    for (const auto& row : results) {
        CheckIn checkin;
        checkin.id = std::stoi(row[0]);
        checkin.user_id = std::stoi(row[1]);
        checkin.meeting_id = std::stoi(row[2]);
        checkin.checkin_time = row[3];
        checkins.push_back(checkin);
    }
    return checkins;
}

std::vector<CheckIn> DatabaseManager::get_checkins_by_user(int user_id) {
    std::string sql = "SELECT id, user_id, meeting_id, checkin_time FROM checkins WHERE user_id = " + std::to_string(user_id);
    auto results = db->query(sql);
    
    std::vector<CheckIn> checkins;
    for (const auto& row : results) {
        CheckIn checkin;
        checkin.id = std::stoi(row[0]);
        checkin.user_id = std::stoi(row[1]);
        checkin.meeting_id = std::stoi(row[2]);
        checkin.checkin_time = row[3];
        checkins.push_back(checkin);
    }
    return checkins;
}

std::vector<CheckIn> DatabaseManager::get_all_checkins() {
    std::string sql = "SELECT id, user_id, meeting_id, checkin_time FROM checkins";
    auto results = db->query(sql);
    
    std::vector<CheckIn> checkins;
    for (const auto& row : results) {
        CheckIn checkin;
        checkin.id = std::stoi(row[0]);
        checkin.user_id = std::stoi(row[1]);
        checkin.meeting_id = std::stoi(row[2]);
        checkin.checkin_time = row[3];
        checkins.push_back(checkin);
    }
    return checkins;
}

bool DatabaseManager::delete_checkin(int id) {
    std::string sql = "DELETE FROM checkins WHERE id = " + std::to_string(id);
    try {
        db->execute(sql);
        return true;
    } catch (const sqlite::database_error&) {
        return false;
    }
}