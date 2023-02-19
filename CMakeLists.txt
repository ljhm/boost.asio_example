cmake_minimum_required(VERSION 3.18)
project(MyProject VERSION 1.0)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_link_options(-fsanitize=address)
endif()
link_libraries(pthread)
link_directories(/home/ljh/Downloads/boost_1_81_0/stage/lib)
include_directories(/home/ljh/Downloads/boost_1_81_0)
#add_compile_definitions(VAR) # VAR=value
add_compile_options(-std=c++11) # .cpp
#set(CMAKE_C_COMPILER g++) # .cpp
#file(GLOB SOURCES *.cpp) # .cpp

#add_executable(main ${SOURCES})
add_executable(server server.cpp)
add_executable(client client.cpp)
#add_library(foo SHARED ${SOURCES})
#set_target_properties(foo PROPERTIES VERSION 1.2.3 SOVERSION 1.2)
#add_subdirectory(./foo)

# make -C build DESTDIR=~/foo install
#install(TARGETS main) # foo
#file(GLOB LIBRARIES ../lib/*so*)
#install(FILES ${LIBRARIES} DESTINATION lib)
