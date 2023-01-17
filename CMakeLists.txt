cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)

#project(cal_lib)

#add_library(cal_lib SHARED cal_main.h cal_tran.cpp)
#set_target_properties(cal_lib PROPERTIES VERSION ${PROJECT_VERSION})
#
#install(TARGETS cal_lib DESTINATION lib/cal_lib)
#
#file(GLOB HEADERS cal_main.h)
#install(FILES cal_main.h DESTINATION include/cal_lib)

project(cpp_cv)

add_executable(cpp_cv main.cpp cal_main.h cal_tran.cpp)


