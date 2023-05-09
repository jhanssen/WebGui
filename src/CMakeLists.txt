# set(CMAKE_C_COMPILER emcc)
# set(CMAKE_CXX_COMPILER em++)

set(OUTPUT imgui)

set(SOURCES
    main.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
    ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_widgets.cpp ${IMGUI_DIR}/imgui_tables.cpp
    )

set(WASM_OPTS -sUSE_WEBGPU=1 -sUSE_GLFW=3 -sASSERTIONS=1 -sWASM=1 -sSINGLE_FILE=1 --preload-file ${WEBGUI_DATA_DIR})

add_executable(imgui ${SOURCES})
set_target_properties(imgui PROPERTIES
    CXX_STANDARD 20
    OUTPUT_NAME ${OUTPUT})
target_link_options(imgui PRIVATE ${WASM_OPTS})
target_include_directories(imgui PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

set_target_properties(imgui
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${WEBGUI_BINARY_DIR}/bin"
)
