cmake_minimum_required(VERSION 3.26)
project(computer_graphic_hcmus C CXX)

# GLFW
set(GLFW_DIR ./src/vendors/glfw)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
add_subdirectory(${GLFW_DIR})
include_directories(${GLFW_DIR}/include)

# GLM
set(GLM_DIR ./src/vendors/glm)
add_subdirectory(${GLM_DIR})
include_directories(${GLM_DIR})

# GLAD
set(GLAD_DIR ./src/vendors/glad)
include_directories(${GLAD_DIR}/include)
set(GLAD_SOURCES ${GLAD_DIR}/src/glad.c)

# IMGUI
set(IMGUI_DIR ./src/vendors/imgui)
set(LIB_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# binary target
file(GLOB SOURCES "src/*.cpp")
set(TARGET_NAME cg)
add_executable(${TARGET_NAME} ${SOURCES} ${LIB_SOURCES} ${GLAD_SOURCES})
if(WIN32 AND MSVC)
    target_link_options(${TARGET_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif()
target_link_libraries(${TARGET_NAME} PRIVATE glm glfw)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# copy target
add_custom_target(
    "copy-${TARGET_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${TARGET_NAME}>/assets
    COMMENT "Copying ${TARGET_NAME} assets after build"
)
add_dependencies(${TARGET_NAME} "copy-${TARGET_NAME}")


# run target
add_custom_target(
    "run-${TARGET_NAME}"
    COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:${TARGET_NAME}>
            $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS ${TARGET_NAME}
    COMMENT "Running ${TARGET_NAME} in $<TARGET_FILE_DIR:${TARGET_NAME}>"
)