project(cfg_parser)
cmake_minimum_required(VERSION 2.8)

set(CFG_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/cfg_parser.h
	${CMAKE_CURRENT_LIST_DIR}/cfg_parser.cpp
	)
  
add_library(cfg_lib ${CFG_SOURCES})