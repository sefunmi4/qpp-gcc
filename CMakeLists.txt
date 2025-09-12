cmake_minimum_required(VERSION 3.20)
project(qpp VERSION 0.1.0 LANGUAGES CXX)

# your targets here
add_executable(qpp src/main.cpp)  # example

# ---------- Install rules ----------
# Binaries
install(TARGETS qpp RUNTIME DESTINATION bin)
# Headers (if any installable API)
# install(DIRECTORY include/ DESTINATION include)

# ---------- CPack package config ----------
set(CPACK_PACKAGE_NAME "qpp")
set(CPACK_PACKAGE_VENDOR "Architecture of Homosapiens")
set(CPACK_PACKAGE_CONTACT "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Q++: quantum–classical compiler and simulator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

# Source / generic
set(CPACK_GENERATOR "TGZ;ZIP")

# Linux .deb (starter)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Architecture of Homosapiens")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
list(APPEND CPACK_GENERATOR "DEB")

# Windows: leave ZIP for now; installer can come later
# macOS: use ZIP/TGZ; Homebrew will build from tarball

include(CPack)
