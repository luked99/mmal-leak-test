
add_definitions(-DENABLE_MMAL_STANDALONE)

find_path(root ../userland ../../userland)
set(mmal ${root}/interface/mmal)
add_definitions(-g -Wall -Werror)

cmake_minimum_required(VERSION 2.8)
include_directories(${root})
include_directories(${root}/interface/mmal)
include_directories(${root}/interface/vchiq_arm)
include_directories(${root}/interface/vcos/pthreads)
add_subdirectory(${root}/interface/mmal/core mmal_core)
add_subdirectory(${root}/interface/mmal/util mmal_util)
add_subdirectory(${root}/interface/vcos vcos)
add_executable(leaker mmal-leak.c ${mmal}/components/null_sink.c)
target_link_libraries(leaker mmal_core mmal_util)
