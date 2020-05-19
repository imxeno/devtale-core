cmake_minimum_required(VERSION 3.8)
project(devtale-core)

# boost
find_package(Boost 1.70 REQUIRED COMPONENTS
             date_time)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
include_directories(nlohmann_json)
add_subdirectory(nlohmann_json)

# target definition
add_library(devtale-core SHARED devtale-core.cpp memory.cpp)
target_link_libraries(devtale-core ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)
# target definition
add_executable(devtale-core-test devtale-core.cpp memory.cpp)
target_link_libraries(devtale-core-test ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)