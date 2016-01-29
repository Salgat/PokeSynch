cmake_minimum_required(VERSION 3.2)
project(PokeSynch)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

#set(SOURCE_FILES main.cpp)
add_executable(PokeSynch ${SOURCE_FILES}  src/main.cpp
                                                src/Processor.cpp
                                                src/GameBoy.hpp
                                                src/GameBoy.cpp 
                                                src/MemoryManagementUnit.hpp 
                                                src/MemoryManagementUnit.cpp 
                                                src/Display.hpp 
                                                src/Display.cpp 
                                                src/Timer.hpp 
                                                src/Timer.cpp 
                                                src/Input.hpp 
                                                src/Input.cpp 
                                                src/Network.hpp 
                                                src/Network.cpp)

target_link_libraries(PokeSynch ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsfml-graphics.a
                                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsfml-window.a
                                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsfml-audio.a
                                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsfml-network.a
                                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsfml-system.a)