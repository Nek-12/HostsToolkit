cmake_minimum_required(VERSION 3.15)
project(HT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(TS_FILES HT_ru_RU.ts)

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")

find_package(Qt5 COMPONENTS Widgets LinguistTools Core Gui REQUIRED)
set(GCC_COVERAGE_COMPILE_FLAGS "-unicode -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings \
-Wlogical-op -Wredundant-decls -Wshadow -Woverloaded-virtual ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
add_executable(HT WIN32 mainwindow.h main.cpp mainwindow.cpp mainwindow.ui res.qrc ${TS_FILES} )

target_link_libraries(HT PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)
