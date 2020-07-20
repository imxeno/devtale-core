cmake_minimum_required(VERSION 3.8)
project(devtale-core)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

# boost
find_package(Boost 1.73 REQUIRED COMPONENTS
             date_time)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
include_directories(nlohmann_json)
add_subdirectory(nlohmann_json)

# target definition
add_library(devtale-core SHARED devtale-core.cpp memory.cpp ansistring.cpp packet_handler.cpp protocol.cpp)
target_link_libraries(devtale-core ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)
# target definition
# add_executable(devtale-core-test devtale-core.cpp memory.cpp protocol.cpp packet_handler.cpp)
#target_link_libraries(devtale-core-test ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)