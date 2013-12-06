# This is the directory for tests

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_INCLUDE_PATH})

ADD_EXECUTABLE(ProjectInfo ProjectInfo.cc)

ADD_EXECUTABLE(DisplayImage DisplayImage.cc)
TARGET_LINK_LIBRARIES(DisplayImage ${LIB_OPENCV})

ADD_EXECUTABLE(Resample Resample.cc)
TARGET_LINK_LIBRARIES(Resample ${LIB_OPENCV} resampler)

ADD_EXECUTABLE(FindMedian FindMedian.cc)
