set(WEBGUI_3RDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui PARENT_SCOPE)

add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)
add_subdirectory(zep/src)
# uuuuuuuuuuuuuuugh
target_include_directories(Zep PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_include_directories(Zep INTERFACE ${CMAKE_CURRENT_LIST_DIR}/zep/include)
