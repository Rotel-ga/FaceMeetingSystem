#include "database_manager.h"
#include <iostream>
#include <stdexcept>

void init_sample_data(DatabaseManager& db_manager) {
    try {
        // 创建示例会议室
        int room1_id = db_manager.create_room("会议室A");
        int room2_id = db_manager.create_room("会议室B");
        int room3_id = db_manager.create_room("会议室C");
        
        std::cout << "创建会议室成功: " << room1_id << ", " << room2_id << ", " << room3_id << std::endl;
        
        // 创建示例用户
        int user1_id = db_manager.create_user("admin", "123456", "");
        int user2_id = db_manager.create_user("zhangsan", "password", "");
        int user3_id = db_manager.create_user("lisi", "123123", "");
        
        std::cout << "创建用户成功: " << user1_id << ", " << user2_id << ", " << user3_id << std::endl;
        
        // 创建示例会议
        int meeting1_id = db_manager.create_meeting(room1_id, "项目启动会议", "2024-01-15 09:00:00", "2024-01-15 11:00:00", user1_id);
        int meeting2_id = db_manager.create_meeting(room2_id, "技术讨论会", "2024-01-15 14:00:00", "2024-01-15 16:00:00", user2_id);
        int meeting3_id = db_manager.create_meeting(room1_id, "周例会", "2024-01-16 10:00:00", "2024-01-16 12:00:00", user3_id);
        
        std::cout << "创建会议成功: " << meeting1_id << ", " << meeting2_id << ", " << meeting3_id << std::endl;
        
        // 创建示例签到记录
        int checkin1_id = db_manager.create_checkin(user1_id, meeting1_id, "2024-01-15 08:55:00");
        int checkin2_id = db_manager.create_checkin(user2_id, meeting2_id, "2024-01-15 13:58:00");
        
        std::cout << "创建签到记录成功: " << checkin1_id << ", " << checkin2_id << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "初始化示例数据失败: " << e.what() << std::endl;
    }
}

void test_database_operations(DatabaseManager& db_manager) {
    try {
        // 测试查询所有用户
        std::cout << "\n=== 所有用户 ===" << std::endl;
        auto users = db_manager.get_all_users();
        for (const auto& user : users) {
            std::cout << "ID: " << user.id << ", 用户名: " << user.username 
                     << ", 密码: " << user.password << ", Face Token: " << user.face_token << std::endl;
        }
        
        // 测试查询所有会议室
        std::cout << "\n=== 所有会议室 ===" << std::endl;
        auto rooms = db_manager.get_all_rooms();
        for (const auto& room : rooms) {
            std::cout << "ID: " << room.id << ", 名称: " << room.name << std::endl;
        }
        
        // 测试查询所有会议
        std::cout << "\n=== 所有会议 ===" << std::endl;
        auto meetings = db_manager.get_all_meetings();
        for (const auto& meeting : meetings) {
            std::cout << "ID: " << meeting.id << ", 会议室ID: " << meeting.room_id 
                     << ", 主题: " << meeting.topic << ", 开始时间: " << meeting.time_start 
                     << ", 结束时间: " << meeting.time_end << ", 用户ID: " << meeting.user_id << std::endl;
        }
        
        // 测试查询所有签到记录
        std::cout << "\n=== 所有签到记录 ===" << std::endl;
        auto checkins = db_manager.get_all_checkins();
        for (const auto& checkin : checkins) {
            std::cout << "ID: " << checkin.id << ", 用户ID: " << checkin.user_id 
                     << ", 会议ID: " << checkin.meeting_id << ", 签到时间: " << checkin.checkin_time << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "测试数据库操作失败: " << e.what() << std::endl;
    }
}

int main() {
    try {
        // 创建数据库管理器
        DatabaseManager db_manager("../face_meeting.db");
        
        std::cout << "正在初始化数据库表..." << std::endl;
        db_manager.initialize_tables();
        std::cout << "数据库表初始化完成!" << std::endl;
        
        std::cout << "\n正在插入示例数据..." << std::endl;
        init_sample_data(db_manager);
        std::cout << "示例数据插入完成!" << std::endl;
        
        std::cout << "\n开始测试数据库操作..." << std::endl;
        test_database_operations(db_manager);
        std::cout << "\n数据库操作测试完成!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}