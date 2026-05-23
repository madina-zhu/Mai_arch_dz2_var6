cmake_minimum_required(VERSION 3.12)
project(delivery_api)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JWT
include(FetchContent)
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.2
)
FetchContent_MakeAvailable(jwt-cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(userver REQUIRED COMPONENTS core postgresql mongo redis rabbitmq)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    userver::core
    userver::postgresql
    userver::mongo
    userver::redis
    userver::rabbitmq
    jwt-cpp::jwt-cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-missing-field-initializers
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY configs/ DESTINATION ${CMAKE_INSTALL_PREFIX}/configs)