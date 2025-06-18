#pragma once
#include "crow_all.h"

struct CORS
{
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&)
    {
        res.add_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        
        // 对预检请求（OPTIONS）直接返回 200
        if (req.method == crow::HTTPMethod::Options)
        {
            res.code = 200;
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&)
    {
        // 可以在这里添加额外 headers，如果需要
    }
};