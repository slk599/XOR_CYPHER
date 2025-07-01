cmake_minimum_required(VERSION 3.16)
project(FileProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    fileprocessor.cpp
)

set(HEADERS
    mainwindow.h
    fileprocessor.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
