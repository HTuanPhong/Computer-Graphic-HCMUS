#!/bin/bash
# Web build script for Computer Graphics HCMUS project
# Requires: Emscripten SDK installed and activated

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "Error: emcc (Emscripten compiler) not found"
    echo "Please activate Emscripten SDK first:"
    echo "  source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build_web

echo "=== Building HCMUS for Web (Emscripten) ==="
emcc \
    -xc++ \
    -std=c++17 \
    -O3 \
    -s WASM=1 \
    -s USE_WEBGL2=1 \
    -s MIN_WEBGL_VERSION=2 \
    -s MAX_WEBGL_VERSION=2 \
    -s FULL_ES3=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s TOTAL_MEMORY=536870912 \
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
    --emrun \
    -I src \
    -I src/vendors/glad/include \
    -I src/vendors/glm \
    -I src/vendors/imgui \
    -I src/vendors/imgui/backends \
    --embed-file assets@/assets \
    -Wl,--export-all \
    src/main_web.cpp \
    src/App_Web.cpp \
    src/Camera.cpp \
    src/Render.cpp \
    src/Shader.cpp \
    src/Draw.cpp \
    src/vendors/imgui/imgui.cpp \
    src/vendors/imgui/imgui_draw.cpp \
    src/vendors/imgui/imgui_tables.cpp \
    src/vendors/imgui/imgui_widgets.cpp \
    src/vendors/imgui/backends/imgui_impl_opengl3.cpp \
    src/vendors/imgui/backends/imgui_impl_glfw.cpp \
    -xc \
    -std=c99 \
    src/vendors/glad/src/glad.c \
    -o build_web/app.js

if [ $? -eq 0 ]; then
    echo "=== Build successful! ==="
    echo "Output: build_web/app.js and build_web/app.wasm"
    echo "HTML: index.html"
    echo ""
    echo "To run locally:"
    echo "  emrun --browser firefox build_web/app.html"
    echo "Or start a simple HTTP server:"
    echo "  python3 -m http.server 8000"
else
    echo "=== Build failed! ==="
    exit 1
fi
