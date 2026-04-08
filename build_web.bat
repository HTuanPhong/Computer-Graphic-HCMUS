@echo off
REM Web build script for Computer Graphics HCMUS project (Windows Batch)
REM Usage: build_web.bat [clean]

setlocal enabledelayedexpansion

cd /d "%~dp0"

if "%1"=="clean" (
    echo Cleaning build_web directory...
    if exist build_web (
        for /r build_web %%F in (*) do del "%%F"
    )
)

if not exist build_web (
    echo Creating build_web directory...
    mkdir build_web
)

echo.
echo === Building HCMUS for Web (Emscripten) ===
echo.

REM Activate Emscripten SDK
call C:\dev\emsdk\emsdk_env.bat

REM Build command - compile everything together
REM Note: glad.c is skipped because Emscripten provides WebGL bindings directly
emcc -std=c++17 -O3 ^
    -s WASM=1 ^
    -s USE_WEBGL2=1 ^
    -s FULL_ES3=1 ^
    -s USE_GLFW=3 ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s TOTAL_MEMORY=536870912 ^
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap ^
    --emrun ^
    -I src ^
    -I src\vendors\glad\include ^
    -I src\vendors\glm ^
    -I src\vendors\imgui ^
    -I src\vendors\imgui\backends ^
    --embed-file assets@/assets ^
    src\main_web.cpp ^
    src\App_Web.cpp ^
    src\Camera.cpp ^
    src\Render.cpp ^
    src\Shader.cpp ^
    src\Draw.cpp ^
    src\vendors\imgui\imgui.cpp ^
    src\vendors\imgui\imgui_draw.cpp ^
    src\vendors\imgui\imgui_tables.cpp ^
    src\vendors\imgui\imgui_widgets.cpp ^
    src\vendors\imgui\backends\imgui_impl_opengl3.cpp ^
    src\vendors\imgui\backends\imgui_impl_glfw.cpp ^
    -o build_web\app.js

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === Build successful! ===
    echo Output files:
    dir build_web\app.*
    echo.
    echo To run locally:
    echo   Option 1: emrun --open browser build_web\app.html
    echo   Option 2: python -m http.server 8000
    echo            then open http://localhost:8000
) else (
    echo.
    echo === Build failed! ===
    exit /b 1
)

endlocal
