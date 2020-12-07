cmake_minimum_required(VERSION 3.8)
project(devtale-core)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

macro(get_WIN32_WINNT version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

        set(${version} "0x${ver}")
    endif()
endmacro()

get_WIN32_WINNT(ver)
add_definitions(-D_WIN32_WINNT=${ver})

# boost
find_package(Boost 1.72 REQUIRED COMPONENTS date_time)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
include_directories(nlohmann_json)
add_subdirectory(nlohmann_json)

# project

# target definition
add_library(devtale-core SHARED 
	src/devtale-core.cpp 
	src/ansistring.cpp
	src/packet_handler.cpp
	src/protocol.cpp
	src/memory.cpp
 )

target_link_libraries(devtale-core ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)