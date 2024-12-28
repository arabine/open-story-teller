if (NOT TARGET imgui)
    include(FetchContent)

    FetchContent_Declare(
        imgui
        #GIT_REPOSITORY     https://github.com/ocornut/imgui.git
        #GIT_TAG            12a3c77c2f4671813c2a09197234d5672f8ab5f3

        GIT_REPOSITORY      https://github.com/thedmd/imgui.git
        GIT_TAG             layouts
    )

    FetchContent_MakeAvailable(imgui)

    file(GLOB SOURCES CONFIGURE_DEPENDS "${imgui_SOURCE_DIR}/*.cpp" "${imgui_SOURCE_DIR}/*.h")

    source_group(TREE ${imgui_SOURCE_DIR} FILES ${SOURCES})

    set(imgui_natvis_path "${imgui_SOURCE_DIR}/misc/debuggers/imgui.natvis")

    add_library(imgui STATIC ${SOURCES} ${imgui_natvis_path})

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
    )

    target_compile_definitions(imgui PUBLIC
        IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        IMGUI_DISABLE_OBSOLETE_KEYIO
    )

    set_target_properties(
        imgui
        PROPERTIES
            FOLDER "external"
    )
endif()

foreach(component ${imgui_FIND_COMPONENTS})

    if (TARGET imgui_${component})
        continue()
    endif()

    file(GLOB COMPONENT_SOURCES CONFIGURE_DEPENDS "${imgui_SOURCE_DIR}/backends/imgui_impl_${component}*")

    source_group(TREE ${imgui_SOURCE_DIR}/backends FILES ${COMPONENT_SOURCES})

    add_library(imgui_${component} STATIC ${COMPONENT_SOURCES})

    target_include_directories(imgui_${component} PUBLIC
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui_${component} PUBLIC imgui)

    if (component MATCHES "dx11")
        target_link_libraries(imgui_${component} PUBLIC d3d11)
    elseif (component MATCHES "opengl*")
        find_package(OpenGL REQUIRED COMPONENTS OpenGL)
        target_link_libraries(imgui_${component} PUBLIC OpenGL::GL)
    elseif(component STREQUAL "glfw")
        find_package(glfw REQUIRED)
        target_link_libraries(imgui_${component} PUBLIC glfw)
    endif()

    add_library(imgui::${component} ALIAS imgui_${component})

    set_target_properties(
        imgui_${component}
        PROPERTIES
            FOLDER "external"
    )

endforeach()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    imgui
    DEFAULT_MSG
)

