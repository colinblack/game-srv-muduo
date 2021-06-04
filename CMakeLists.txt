cmake_minimum_required(VERSION 2.8)

#project(sg17)

#允许gdb调试
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRNT -D_DEBUG_BUILD -fPIC -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -pipe -fno-ident -MMD -D_GNU_SOURCE -D_REENTRNT -D_RELEASE_BUILD -fPIC -std=c++11")
 
#添加C++11支持及其他选项
#set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs -Wno-deprecated")

add_subdirectory(proto)
add_subdirectory(save)
add_subdirectory(app)
#add_subdirectory(cgiapp)
