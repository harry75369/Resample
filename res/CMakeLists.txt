# This is the directory for resources

FILE(GLOB ress RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
FOREACH(res ${ress})
  CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/${res}"
    "${PROJECT_BINARY_DIR}/${res}"
    COPYONLY
  )
ENDFOREACH(res)
