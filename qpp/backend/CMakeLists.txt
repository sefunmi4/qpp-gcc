add_library(qpp_backend
    LocalSimBackend.cpp
    QPUBackend.cpp
)

target_include_directories(qpp_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

option(QPP_USE_VENDOR_SDK "Link against vendor provided SDK" OFF)
if(QPP_USE_VENDOR_SDK)
    find_package(VendorSDK REQUIRED)
    target_link_libraries(qpp_backend PRIVATE VendorSDK::VendorSDK)
    target_compile_definitions(qpp_backend PRIVATE QPP_USE_VENDOR_SDK)
endif()

