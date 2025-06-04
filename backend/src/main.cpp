#define CROW_USE_ASIO
#define CROW_DISABLE_SSL
#include "crow_all.h"

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello from Crow!";
    });

    CROW_ROUTE(app, "/json")([]{
        crow::json::wvalue x;
        x["message"] = "This is JSON response!";
        return x;
    });

    app.port(18080).multithreaded().run();
}
