cmake_minimum_required(VERSION 2.8)


 
#添加C++11支持及其他选项
#set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -Wno-deprecated")

add_subdirectory(battle)
add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(data)
add_subdirectory(kernel)
add_subdirectory(main)
add_subdirectory(pb)
