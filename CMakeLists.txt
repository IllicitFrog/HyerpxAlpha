cmake_minimum_required(VERSION 3.10)

project (Hyperx)

set(C++_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_package(hidapi REQUIRED)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

add_executable(Hyperx src/hyperx.cpp)

target_link_libraries(Hyperx PRIVATE hidapi::hidapi PkgConfig::GTKMM)
include_directories(Hyperx PRIVATE ${CMAKE_HOME_DIRECTORY}/include)
