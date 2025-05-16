cmake_minimum_required(VERSION 3.10)
project(C-SmartTrafficLights)

set(CMAKE_C_STANDARD 17)

include_directories(include libs/cJSON)

set(SOURCES src/main.c src/rw_logic.c src/traffic.c libs/cJSON/cJSON.c)

add_executable(main ${SOURCES})
