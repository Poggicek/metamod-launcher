file(GLOB_RECURSE SOURCES_LIST
  "**.cpp"
)

file(GLOB_RECURSE HEADERS_LIST
  "**.h"
)

add_executable(Launcher WIN32 ${SOURCES_LIST} ${HEADERS_LIST})

target_link_libraries(Launcher PRIVATE
  safetyhook
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCES_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADERS_LIST})