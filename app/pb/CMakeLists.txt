# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
cmake_minimum_required (VERSION 2.8) 

include_directories(/usr/include/openssl)
include_directories(${PROJECT_SOURCE_DIR}/baselib/include)
include_directories(${PROJECT_SOURCE_DIR}/outerlib/dbc/include/)
include_directories(${PROJECT_SOURCE_DIR}/outerlib/pengyou/include)
include_directories(${PROJECT_SOURCE_DIR}/outerlib/xsg/include)
include_directories(${PROJECT_SOURCE_DIR}/save/)
#include_directories(/data/colin/fmh5_cmake/app/pb/)
include_directories(${PROJECT_SOURCE_DIR}/proto/)
#include_directories(/data/colin/fmh5_cmake/app/common/)
#include_directories(/data/colin/fmh5_cmake/app/kernel/)
#include_directories(/data/colin/fmh5_cmake/app/config/)

link_directories(/usr/lib64/openssl)
link_directories(${PROJECT_SOURCE_DIR}/baselib/lib)
link_directories(${PROJECT_SOURCE_DIR}/outerlib/dbc/lib/)
link_directories(${PROJECT_SOURCE_DIR}/outerlib/pengyou/lib)
link_directories(${PROJECT_SOURCE_DIR}/outerlib/xsg/lib)
link_directories(${PROJECT_SOURCE_DIR}/save)
#link_directories(/data/colin/fmh5_cmake/app/pb/)
link_directories(${PROJECT_SOURCE_DIR}/proto)
#link_directories(/data/colin/fmh5_cmake/app/common/)
#link_directories(/data/colin/fmh5_cmake/app/kernel)
#link_directories(/data/colin/fmh5_cmake/app/config/)

link_libraries(sg17base)
link_libraries(sg17proto)
link_libraries(sg17save)
link_libraries(dbs-gcc-4.4)
link_libraries(fcgi++)
link_libraries(fcgi)
link_libraries(nsl)
link_libraries(ssl)
link_libraries(sg17openapi)
link_libraries(curl)
link_libraries(z)
link_libraries(dl)
link_libraries(xsg)


aux_source_directory(. DIR_LIB_SRCS)
# 生成链接库
add_library(sg17pb SHARED ${DIR_LIB_SRCS})
#target_link_libraries(libsg17cgicommon libsg17common libsg17kernel liblibsg17proto libsg17cgicommon)


