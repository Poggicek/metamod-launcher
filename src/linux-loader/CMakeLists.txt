file(GLOB_RECURSE SOURCES_LIST
  "**.cpp"
)

file(GLOB_RECURSE HEADERS_LIST
  "**.h"
)

add_library(Loader SHARED ${SOURCES_LIST} ${HEADERS_LIST})

set_target_properties(Loader PROPERTIES OUTPUT_NAME "metamod-loader")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS_LIST})
