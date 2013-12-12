# This is the directory for source code

# Configuration
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/Config.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Config.hpp"
)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_INCLUDE_PATH})
ADD_LIBRARY(resampler STATIC
  NearestResampler.cpp
  BilinearResampler.cpp
  BicubicResampler.cpp
  LanczosResampler.cpp
  AbstractionResampler.cpp
)
TARGET_LINK_LIBRARIES(resampler ${LIB_OPENCV})

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} PARENT_SCOPE)
