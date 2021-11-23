cmake_minimum_required(VERSION 3.12)
project(devtale-core)

if(NOT "${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
  MESSAGE(FATAL_ERROR "Compiling devtale-core with a 64-bit target does not make any sense.")
endif()

if(!MSVC)
  MESSAGE(FATAL ERROR "Are you really sure what are you doing?")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

if(WIN32)
    macro(get_WIN32_WINNT version)
        if(CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif()
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif()
    endmacro()

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif()

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
    "src/websocket_handler.cpp"
 "src/utils.cpp")
    
target_link_libraries(devtale-core ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)

# loader

add_library(EWSF SHARED 
    "src/loader.cpp" "src/loader.rc"  "src/inputbox.h" "src/inputbox.cpp")

set_target_properties(
    EWSF
    PROPERTIES 
        SUFFIX ".EWS"
)