# This is directory for library configurations

# Extra search path for different environments
INCLUDE(LocalLibPath.cmake)

# Giant third-party libraries
## Boost
IF (WIN32)
  SET(Boost_USE_STATIC_LIBS        ON)
  SET(Boost_USE_MULTITHREADED      ON)
  SET(Boost_USE_STATIC_RUNTIME    OFF)
ENDIF (WIN32)
FIND_PACKAGE(Boost)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})
SET(LIB_BOOST ${Boost_LIBRARIES})
MESSAGE(STATUS "Available Boost Libraries: ${LIB_BOOST}")
## OpenCV
FIND_PACKAGE(OpenCV REQUIRED)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${OpenCV_INCLUDE_DIRS})
SET(LIB_OPENCV ${OpenCV_LIBS})
MESSAGE(STATUS "Available OpenCV Libraries: ${LIB_OPENCV}")

# Export Library Include Paths
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Export Library Libs
SET(LIB_BOOST ${LIB_BOOST} PARENT_SCOPE)
SET(LIB_OPENCV ${LIB_OPENCV} PARENT_SCOPE)
