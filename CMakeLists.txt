cmake_minimum_required(VERSION 3.10)

project (Hyperx)

set(C++_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)
find_package(hidapi REQUIRED)

add_executable(Hyperx src/hyperxApp.cpp src/hyperxFrame.cpp src/SwitchCtrl.cpp src/dialog.cpp src/pulse.cpp)
include(${wxWidgets_USE_FILE})
target_link_libraries(Hyperx PRIVATE hidapi::hidapi pulse-simple pulse ${wxWidgets_LIBRARIES})
include_directories(Hyperx PRIVATE ${CMAKE_HOME_DIRECTORY}/include ${wxWidgets_INCLUDE_DIRS})
