cmake_minimum_required(VERSION 3.20)

# Determine project version from environment or git tag, defaulting to 0.1.6
if(DEFINED ENV{QPP_VERSION})
  set(QPP_VERSION $ENV{QPP_VERSION})
else()
  execute_process(
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE QPP_VERSION_FROM_GIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(QPP_VERSION_FROM_GIT)
    string(REGEX REPLACE "^v" "" QPP_VERSION_FROM_GIT "${QPP_VERSION_FROM_GIT}")
    if(QPP_VERSION_FROM_GIT MATCHES "^[0-9]+\\.[0-9]+(\\.[0-9]+)?")
      set(QPP_VERSION ${QPP_VERSION_FROM_GIT})
    else()
      set(QPP_VERSION 0.1.6)
    endif()
  else()
    set(QPP_VERSION 0.1.6)
  endif()
endif()

project(qpp VERSION ${QPP_VERSION} LANGUAGES CXX)

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
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Architecture of Homosapiens")
  set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
  list(APPEND CPACK_GENERATOR "DEB")
endif()

# Windows: leave ZIP for now; installer can come later
# macOS: use ZIP/TGZ; Homebrew will build from tarball

# Override the default system name in package file/folder names.
# By default CPack uses values like "Darwin" which end up creating
# archives such as qpp-0.1.6-Darwin.tar.gz.  Downstream tooling expects
# the macOS arm64 package to contain a top-level directory named
# "qpp-<version>-macos-arm64" instead.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(CPACK_SYSTEM_NAME "macos-arm64")
  else()
    set(CPACK_SYSTEM_NAME "macos-x64")
  endif()
endif()

include(CPack)
