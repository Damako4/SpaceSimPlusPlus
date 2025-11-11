cmake_minimum_required(VERSION 3.30)
project(FinalGame)

set(CMAKE_CXX_STANDARD 20)

include_directories(FinalGame include)

find_package(glfw3 REQUIRED)

add_executable(FinalGame src/main.cpp src/common/glad.c
        src/common/loadShader.cpp
        include/loadShader.hpp
        include/common/common.hpp
        src/common/common.cpp
        src/common/controls.cpp
        include/common/controls.h
        src/common/objloader.cpp
        include/common/objloader.h
        src/common/vboindexer.cpp
        include/common/vboindexer.h
        src/common/vboindexer.cpp
        include/common/vboindexer.h
        src/common/text2D.cpp
        include/common/text2D.h
        src/common/error.cpp
        include/common/error.h
        src/common/world.cpp
        include/common/world.h
        include/structs.h
        src/common/tangentspace.cpp
        include/common/tangentspace.h
        src/common/planets.cpp
        include/common/planets.h
        src/common/raycast.cpp
        include/common/raycast.h
        src/common/physics.cpp
        include/common/physics.h
)

target_link_libraries(FinalGame PRIVATE glfw soil2)