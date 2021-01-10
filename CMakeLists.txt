cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(APP_NAME volumemountersvc)
project(${APP_NAME} C)
add_executable(${APP_NAME} svc.c)
