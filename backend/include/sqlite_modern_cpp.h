//关于sqlite3的封装
#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

namespace sqlite {

class database_error : public std::runtime_error {
public:
    explicit database_error(const std::string& msg) : std::runtime_error(msg) {}
};

class database {
private:
    sqlite3* db;
    
public:
    explicit database(const std::string& db_name) {
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string error = "Cannot open database: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            throw database_error(error);
        }
    }
    
    ~database() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // 禁用拷贝构造和赋值
    database(const database&) = delete;
    database& operator=(const database&) = delete;
    
    // 执行SQL语句
    void execute(const std::string& sql) {
        char* error_msg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_msg);
        if (rc != SQLITE_OK) {
            std::string error = "SQL error: " + std::string(error_msg);
            sqlite3_free(error_msg);
            throw database_error(error);
        }
    }
    
    // 查询并返回结果
    std::vector<std::vector<std::string>> query(const std::string& sql) {
        std::vector<std::vector<std::string>> results;
        
        auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
            auto* results = static_cast<std::vector<std::vector<std::string>>*>(data);
            std::vector<std::string> row;
            for (int i = 0; i < argc; i++) {
                row.push_back(argv[i] ? argv[i] : "");
            }
            results->push_back(row);
            return 0;
        };
        
        char* error_msg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), callback, &results, &error_msg);
        if (rc != SQLITE_OK) {
            std::string error = "SQL error: " + std::string(error_msg);
            sqlite3_free(error_msg);
            throw database_error(error);
        }
        
        return results;
    }
    
    // 获取最后插入的行ID
    long long last_insert_rowid() {
        return sqlite3_last_insert_rowid(db);
    }
};

} // namespace sqlite