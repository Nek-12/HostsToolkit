cmake_minimum_required(VERSION 3.15)
project(HT LANGUAGES CXX)
# set( CMAKE_VERBOSE_MAKEFILE ON )

# QT stuff
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(TS_FILES HT_ru_RU.ts)

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")

find_package(Qt5 COMPONENTS Widgets LinguistTools Core Gui Network REQUIRED)
set(GCC_COVERAGE_COMPILE_FLAGS "-unicode -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings \
-Wlogical-op -Wredundant-decls -Woverloaded-virtual") # -static
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

#include everything from this dir
include_directories(
        ${PROJECT_SOURCE_DIR}/src
)

# list all the code files
file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
)

# generate .ts file if unfinished
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
# include all code + qt stuff
add_executable(HT WIN32 ${all_SRCS} src/app.ui res/res.qrc ${TS_FILES} )

target_link_libraries(HT PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)

install(TARGETS HT
        CONFIGURATIONS Release
        DESTINATION bin)
