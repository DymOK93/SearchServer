cmake_minimum_required (VERSION 3.8)
project ("SearchServer")

set(CMAKE_CXX_STANDARD 17)

set(
	SEARCH_SERVER_HEADER_FILES
		iterator_range.h
		parse.h
		search_server.h
)
set(
	SEARCH_SERVER_SOURCE_FILES
		parse.cpp
		search_server.cpp
)

add_library(
	SearchServer STATIC
		${SEARCH_SERVER_HEADER_FILES}
		${SEARCH_SERVER_SOURCE_FILES}
)
target_include_directories(
	SearchServer PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)