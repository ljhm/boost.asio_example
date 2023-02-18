cmake_minimum_required(VERSION 3.18)
project(MyProject VERSION 1.0)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_link_options(-fsanitize=address)
endif()
include_directories(/home/ljh/Downloads/boost_1_81_0)
link_directories(/home/ljh/Downloads/boost_1_81_0/stage/lib)
link_libraries(pthread)
#set(CMAKE_CXX_COMPILER g++) # .cpp
add_compile_options(-std=c11) # .cpp
file(GLOB SOURCES *.cpp) # .cpp

#add_executable(main ${SOURCES})
add_executable(server server.cpp)
add_executable(client client.cpp)
#add_library(main SHARED ${SOURCES})
#set_target_properties(main PROPERTIES VERSION 1.2.3 SOVERSION 1.2)
#add_subdirectory(./foo)

# make -C build DESTDIR=~/MyProject install
install(TARGETS server client # foo
       )
file(GLOB DYN_LIB /home/ljh/Downloads/boost_1_81_0/stage/lib/*so*)
install(FILES ${DYN_LIB} DESTINATION lib
       )
