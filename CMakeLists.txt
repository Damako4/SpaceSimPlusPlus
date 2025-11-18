cmake_minimum_required(VERSION 3.30)
set(CMAKE_GENERATOR "Unix Makefiles")

project(SpaceSimPlusPlus)

set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

include_directories(${INCLUDE_DIR})

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${INCLUDE_DIR}
        ${INCLUDE_DIR}/common
)

find_package(glfw3 REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw soil2)