cmake_minimum_required(VERSION 3.12)

# Allow building sources with `.qpp` extension as C++ while retaining existing
# extensions for backward compatibility. This must be set before enabling the
# C++ language.
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS qpp;${CMAKE_CXX_SOURCE_FILE_EXTENSIONS})

project(qpp_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

function(add_example target)
    set(sources ${ARGN})
    set(src_base ${CMAKE_CURRENT_SOURCE_DIR}/${target})
    if(EXISTS ${src_base}.qpp)
        list(PREPEND sources ${src_base}.qpp)
        set_source_files_properties(${src_base}.qpp PROPERTIES LANGUAGE CXX COMPILE_FLAGS "-x c++")
    else()
        list(PREPEND sources ${src_base}.cpp)
    endif()
    add_executable(${target} ${sources})
endfunction()

add_example(pbool_demo)
add_example(qstruct_demo)
add_example(scheduler_demo)
add_example(bell_state ../qpp/backend/LocalSimBackend.cpp)
add_example(hardware_demo ../qpp/backend/LocalSimBackend.cpp ../qpp/backend/QPUBackend.cpp)
