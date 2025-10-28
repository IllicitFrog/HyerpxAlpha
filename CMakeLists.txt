cmake_minimum_required(VERSION 3.10)
project(Hyperx)

# Use C++17 cleanly
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)

# First try the official CMake config for hidapi, then fall back to pkg-config
find_package(hidapi CONFIG QUIET)

if(NOT TARGET hidapi::hidapi)
  # Try common backend packages in order: hidraw, libusb, generic
  pkg_check_modules(HIDAPI_HIDRAW QUIET IMPORTED_TARGET hidapi-hidraw)
  pkg_check_modules(HIDAPI_LIBUSB QUIET IMPORTED_TARGET hidapi-libusb)
  pkg_check_modules(HIDAPI_GENERIC QUIET IMPORTED_TARGET hidapi)

  if(HIDAPI_HIDRAW_FOUND)
    add_library(hidapi::hidapi INTERFACE IMPORTED)
    target_link_libraries(hidapi::hidapi INTERFACE PkgConfig::HIDAPI_HIDRAW)
  elseif(HIDAPI_LIBUSB_FOUND)
    add_library(hidapi::hidapi INTERFACE IMPORTED)
    target_link_libraries(hidapi::hidapi INTERFACE PkgConfig::HIDAPI_LIBUSB)
  elseif(HIDAPI_GENERIC_FOUND)
    add_library(hidapi::hidapi INTERFACE IMPORTED)
    target_link_libraries(hidapi::hidapi INTERFACE PkgConfig::HIDAPI_GENERIC)
  else()
    message(FATAL_ERROR "Could not find hidapi (neither CMake config nor pkg-config backends). Install libhidapi-dev.")
  endif()
endif()

include(${wxWidgets_USE_FILE})

add_executable(Hyperx
  src/main.cpp
  src/hyperxApp.cpp
  src/hyperxFrame.cpp
  src/SwitchCtrl.cpp
  src/dialog.cpp
)

target_link_libraries(Hyperx PRIVATE hidapi::hidapi ${wxWidgets_LIBRARIES})
target_include_directories(Hyperx PRIVATE ${CMAKE_SOURCE_DIR}/include ${wxWidgets_INCLUDE_DIRS})

