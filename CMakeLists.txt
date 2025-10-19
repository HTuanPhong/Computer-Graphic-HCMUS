cmake_minimum_required(VERSION 3.16)
project(computer_graphic_hcmus C CXX)

# GLFW
set(GLFW_DIR ./src/vendors/glfw)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
add_subdirectory(${GLFW_DIR})
include_directories(${GLFW_DIR}/include)

# GLAD
set(GLAD_DIR ./src/vendors/glad)
include_directories(${GLAD_DIR}/include)

# GLM
set(GLM_DIR ./src/vendors/glm)
add_subdirectory(${GLM_DIR})


file(GLOB SOURCES "src/*.c" "src/*.cpp")
set(TARGET_NAME cg)
add_executable(${TARGET_NAME} ${SOURCES} "./src/vendors/glad/src/glad.c")
set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(${TARGET_NAME} PRIVATE glm glfw)


add_custom_target(
    "run-${TARGET_NAME}"
    COMMAND $<TARGET_FILE:${TARGET_NAME}>
    DEPENDS ${TARGET_NAME}
    COMMENT "Running ${TARGET_NAME}."
)
