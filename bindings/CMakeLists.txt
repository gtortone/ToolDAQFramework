cmake_minimum_required(VERSION 3.15)
project(daqinterface_bindings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-O3 -Wno-unused-result -Wno-deprecated-declarations")

set(DAQ_INCLUDE_DIR "" CACHE PATH "ToolDAQ include directory path")
set(DAQ_LIB_DIR "" CACHE PATH "libToolDAQ.so directory path")

if(NOT DAQ_INCLUDE_DIR)
    message(FATAL_ERROR
        "set DAQ_INCLUDE_DIR, e.g.:\n"
        "  cmake -B build -DDAQ_INCLUDE_DIR=/path/to/libDAQInterface/include")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE pybind11_CMAKE_DIR_RESULT
)

if(pybind11_CMAKE_DIR_RESULT EQUAL 0 AND EXISTS "${pybind11_CMAKE_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${pybind11_CMAKE_DIR}")
endif()

find_package(pybind11 REQUIRED)

pybind11_add_module(daqinterface MODULE python/binding.cpp)

target_include_directories(daqinterface PRIVATE ${DAQ_INCLUDE_DIR})

target_link_directories(daqinterface PRIVATE ${DAQ_LIB_DIR})
target_link_libraries(daqinterface PRIVATE ToolDAQ)

set_target_properties(daqinterface PROPERTIES
    INSTALL_RPATH "${DAQ_LIB_DIR}"
    BUILD_WITH_INSTALL_RPATH TRUE
)

set(CROSS_TARGET "" CACHE STRING
    "target label to use for module")
 
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c
        "import sys; print(f'cpython-{sys.version_info.major}{sys.version_info.minor}')"
    OUTPUT_VARIABLE PYTHON_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
 
set_target_properties(daqinterface PROPERTIES
    PREFIX ""
    OUTPUT_NAME "daqinterface.${PYTHON_TAG}-${CROSS_TARGET}"
    SUFFIX ".so"
)

message(STATUS "")
message(STATUS "  Python:          ${Python3_EXECUTABLE}")
message(STATUS "  DAQ_INCLUDE_DIR: ${DAQ_INCLUDE_DIR}")
message(STATUS "  DAQ_LIB_DIR:     ${DAQ_LIB_DIR}")
message(STATUS "  Output:          $<TARGET_FILE:daqinterface>")
message(STATUS "")
