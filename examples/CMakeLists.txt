cmake_minimum_required(VERSION 3.12)
project(qpp_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(pbool_demo pbool_demo.cpp)
add_executable(qstruct_demo qstruct_demo.cpp)
add_executable(scheduler_demo scheduler_demo.cpp)
add_executable(bell_state bell_state.cpp ../qpp/backend/LocalSimBackend.cpp)
add_executable(hardware_demo hardware_demo.cpp ../qpp/backend/LocalSimBackend.cpp ../qpp/backend/QPUBackend.cpp)
