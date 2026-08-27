cmake_minimum_required(VERSION 3.10)
project(FallOfTroy)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(troy main.cpp)

if(TARGET GLUT::GLUT)
    target_link_libraries(troy OpenGL::GL OpenGL::GLU GLUT::GLUT)
else()
    target_include_directories(troy PRIVATE ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
    target_link_libraries(troy ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
endif()
