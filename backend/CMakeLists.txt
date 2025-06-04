cmake_minimum_required(VERSION 3.20)
project(FaceMeetingServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加头文件路径（Crow 和 Asio）
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找源文件
file(GLOB SRC_FILES src/*.cpp)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 链接库（无需使用 -l 前缀）
target_link_libraries(${PROJECT_NAME}
    ws2_32      # Windows Socket API
    mswsock     # Windows Socket Extensions: AcceptEx 等
    ssl         # OpenSSL
    crypto      # OpenSSL
)
