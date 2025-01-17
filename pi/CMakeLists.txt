CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

IF (NOT CMAKE_BUILD_TYPE)
    SET (CMAKE_BUILD_TYPE Release)
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -std=c++17 -Wno-psabi -pthread")

FIND_PACKAGE(OpenCV REQUIRED)
SET(REQUIRED_OpenCV_VERSION 4.1)
IF (OpenCV_VERSION VERSION_LESS REQUIRED_OpenCV_VERSION)
	MESSAGE(FATAL_ERROR "At least OpenCV ${REQUIRED_OpenCV_VERSION} is required")
ENDIF()
MESSAGE("- Found OpenCV ${OpenCV_VERSION}")

SET(IE_DIR $ENV{INTEL_OPENVINO_DIR}/inference_engine/)

INCLUDE_DIRECTORIES("include")  # PUBLIC HEADERS
INCLUDE_DIRECTORIES("3rdparty_include")
ADD_SUBDIRECTORY(src)