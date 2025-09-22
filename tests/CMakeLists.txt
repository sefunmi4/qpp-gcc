cmake_minimum_required(VERSION 3.16)
project(qpp_tests LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../qpp/backend ${CMAKE_CURRENT_BINARY_DIR}/qpp_backend)
target_include_directories(qpp_backend PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../include)

add_library(qpp_quantum_worlds STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../qpp/quantum_worlds.cpp
)
target_include_directories(qpp_quantum_worlds
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/..
)

add_executable(test_worlds test_worlds.cpp)
target_include_directories(test_worlds
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(test_worlds PRIVATE qpp_quantum_worlds)
add_test(NAME quantum_worlds COMMAND test_worlds)

option(QPP_ENABLE_HARDWARE_TESTS "Build tests requiring QPU backend" OFF)

enable_testing()

if(QPP_ENABLE_HARDWARE_TESTS)
    add_executable(test_qpu_backend test_qpu_backend.cpp)
    target_include_directories(test_qpu_backend PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
    target_link_libraries(test_qpu_backend PRIVATE qpp_backend)
    target_compile_definitions(test_qpu_backend PRIVATE QPP_ENABLE_QPU=1)
    add_test(NAME qpu_backend COMMAND test_qpu_backend)
endif()

