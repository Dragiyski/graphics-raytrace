cmake_minimum_required(VERSION 3.16)
project(raytrace)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#    message(STATUS "${_variableName}: ${${_variableName}}")
#endforeach()

message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES:   ${SDL2_LIBRARIES}")

message(STATUS "OPENGL_INCLUDE_DIRS: ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

add_executable(${PROJECT_NAME} src/main.cpp src/gl/shader.cpp src/gl/program.cpp src/Screen.cpp src/global.h src/global.cpp)

add_dependencies(${PROJECT_NAME} SDL2::SDL2)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${OPENGL_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PUBLIC GL_GLEXT_PROTOTYPES)
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} "SDL2" "pthread")