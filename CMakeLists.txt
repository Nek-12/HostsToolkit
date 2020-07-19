cmake_minimum_required(VERSION 3.15)
project(HT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(TS_FILES HT_ru_RU.ts)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET main PROPERTY WIN32_EXECUTABLE true)
endif()

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")

find_package(Qt5 COMPONENTS Widgets LinguistTools Core Gui REQUIRED)

set(GCC_COVERAGE_COMPILE_FLAGS "-static-libgcc -static-libstdc++ -static -lstdc++ -lpthread -unicode -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings \
-Wlogical-op -Wredundant-decls -Wshadow -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-static")


qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
add_executable(HT mainwindow.h main.cpp mainwindow.cpp mainwindow.ui ${TS_FILES} )
target_link_libraries(HT PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui)