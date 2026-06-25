cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(ToolDAQFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
endif()

add_compile_options(-Wpedantic)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-unused-result -Wno-deprecated-declarations")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(TFC_GIT_REPOSITORY "https://github.com/gtortone/ToolFrameworkCore.git"
    CACHE STRING "ToolFrameworkCore repository")
set(TFC_GIT_TAG "main" CACHE STRING "ToolFrameworkCore branch/tag")
set(TFC_SOURCE_DIR "" CACHE PATH "ToolFrameworkCore local path (opt)")

include(FetchContent)
FetchContent_Declare(toolframeworkcore
    GIT_REPOSITORY "${TFC_GIT_REPOSITORY}"
    GIT_TAG        "${TFC_GIT_TAG}"
    GIT_SHALLOW    TRUE)
message(STATUS "Cloning ToolFrameworkCore ${TFC_GIT_REPOSITORY} (${TFC_GIT_TAG})...")
FetchContent_MakeAvailable(toolframeworkcore)
set(TFC_SRC "${toolframeworkcore_SOURCE_DIR}")
message(STATUS "ToolFrameworkCore cloned in: ${TFC_SRC}")

set(TDF_SRC "${CMAKE_CURRENT_SOURCE_DIR}")

list(APPEND CMAKE_MODULE_PATH "${TFC_SRC}/cmake")
include(ToolFrameworkDependencies)

find_zeromq()
find_boost_dependencies(date_time serialization iostreams)
find_package(Threads REQUIRED)

add_library(tooldaq_common INTERFACE)
target_link_libraries(tooldaq_common INTERFACE Threads::Threads)

if(TARGET ToolFramework::ZeroMQ)
    target_link_libraries(tooldaq_common INTERFACE ToolFramework::ZeroMQ)
else()
    target_include_directories(tooldaq_common INTERFACE ${ZMQ_INCLUDE_DIRS})
    target_link_libraries(tooldaq_common INTERFACE ${ZMQ_LIBRARIES})
endif()

if(TARGET Boost::date_time)
    target_link_libraries(tooldaq_common INTERFACE
        Boost::date_time Boost::serialization Boost::iostreams)
else()
    target_include_directories(tooldaq_common INTERFACE ${Boost_INCLUDE_DIRS})
    target_link_libraries(tooldaq_common INTERFACE ${Boost_LIBRARIES})
endif()

file(GLOB_RECURSE _all_headers CONFIGURE_DEPENDS
     "${TFC_SRC}/src/*.h"
     "${TDF_SRC}/src/*.h"
     "${TDF_SRC}/examples/UserTools/*.h"
     "${TDF_SRC}/examples/DataModel/*.h")

set(INCLUDE_OUT_DIR "${CMAKE_BINARY_DIR}/include")

add_custom_target(copy_headers ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${INCLUDE_OUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${_all_headers}
            "${INCLUDE_OUT_DIR}"
    DEPENDS ${_all_headers}
)

file(COPY ${TDF_SRC}/examples/configfiles DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

set(_hdr_dirs "")
foreach(_h ${_all_headers})
    get_filename_component(_d "${_h}" DIRECTORY)
    list(APPEND _hdr_dirs "${_d}")
endforeach()
if(_hdr_dirs)
    list(REMOVE_DUPLICATES _hdr_dirs)
    foreach(_d ${_hdr_dirs})
        target_include_directories(tooldaq_common INTERFACE "${_d}")
    endforeach()
endif()

file(GLOB_RECURSE _all_cpp CONFIGURE_DEPENDS
     "${TFC_SRC}/src/*.cpp"
     "${TDF_SRC}/src/*.cpp"
     "${TDF_SRC}/examples/UserTools/*.cpp"
     "${TDF_SRC}/examples/DataModel/*.cpp")

set(_lib_srcs "")
set(_prog_srcs "")
foreach(_f ${_all_cpp})
    file(READ "${_f}" _content)
    string(REGEX MATCH "int[ \t\r\n]+main[ \t\r\n]*\\(" _ismain "${_content}")
    if(_ismain)
        list(APPEND _prog_srcs "${_f}")
    else()
        list(APPEND _lib_srcs "${_f}")
    endif()
endforeach()

add_library(ToolDAQ_objs OBJECT ${_lib_srcs})
target_link_libraries(ToolDAQ_objs PUBLIC tooldaq_common)

add_library(ToolDAQ SHARED $<TARGET_OBJECTS:ToolDAQ_objs>)         # libToolDAQ.so
target_link_libraries(ToolDAQ PUBLIC tooldaq_common)
set_target_properties(ToolDAQ PROPERTIES OUTPUT_NAME "ToolDAQ")

add_library(ToolDAQ_static STATIC $<TARGET_OBJECTS:ToolDAQ_objs>)  # libToolDAQ.a
target_link_libraries(ToolDAQ_static PUBLIC tooldaq_common)
set_target_properties(ToolDAQ_static PROPERTIES OUTPUT_NAME "ToolDAQ")

message(STATUS "libToolDAQ.{so,a}: ${_lib_srcs}")

foreach(_p ${_prog_srcs})
    get_filename_component(_name "${_p}" NAME_WE)
    if(TARGET ${_name})
        message(WARNING "Executable '${_name}' duplicated, skip${_p}")
    else()
        add_executable(${_name} "${_p}")
        target_link_libraries(${_name} PRIVATE ToolDAQ)
        message(STATUS "Executable ${_name}: ${_p}")
    endif()
endforeach()

include(GNUInstallDirs)

install(TARGETS ToolDAQ ToolDAQ_static
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

get_property(_all_targets DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
             PROPERTY BUILDSYSTEM_TARGETS)
foreach(_t ${_all_targets})
    get_target_property(_type ${_t} TYPE)
    if(_type STREQUAL "EXECUTABLE")
        install(TARGETS ${_t} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endforeach()
