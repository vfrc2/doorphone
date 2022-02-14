cmake_minimum_required(VERSION 3.1)

project(doorphone VERSION 0.1.1)

add_executable(doorphone src/main.c src/app.c src/sip.c)

configure_file(src/version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(doorphone linphone mediastreamer_base)