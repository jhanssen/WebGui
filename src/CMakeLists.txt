# set(CMAKE_C_COMPILER emcc)
# set(CMAKE_CXX_COMPILER em++)

set(OUTPUT imgui)

set(SOURCES
    main.cpp
    editor.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
    ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_widgets.cpp ${IMGUI_DIR}/imgui_tables.cpp
    )

set(WASM_OPTS
    -sNO_DISABLE_EXCEPTION_CATCHING=1
    -sUSE_WEBGPU=1
    -sUSE_GLFW=3
    -sASSERTIONS=1
    -sWASM=1
    -sSINGLE_FILE=1
    --preload-file data)

add_custom_target(copy_data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../data ${WEBGUI_BINARY_DIR}/data
)

add_executable(imgui ${SOURCES})
set_target_properties(imgui PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME ${OUTPUT})
target_link_options(imgui PRIVATE ${WASM_OPTS})
target_link_libraries(imgui PRIVATE Zep::Zep)
target_include_directories(imgui PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)
add_dependencies(imgui copy_data)

set_target_properties(imgui
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}"
)

add_custom_command(
    TARGET imgui
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WEBGUI_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy ${WEBGUI_BINARY_DIR}/imgui.js ${WEBGUI_BINARY_DIR}/imgui.data ${WEBGUI_BINARY_DIR}/bin/
    COMMAND ${CMAKE_COMMAND} -E rm ${WEBGUI_BINARY_DIR}/imgui.js ${WEBGUI_BINARY_DIR}/imgui.data
)
