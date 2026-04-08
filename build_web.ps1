# Web build script for Computer Graphics HCMUS project (Windows/PowerShell)
# Requires: Emscripten SDK installed and activated
# Usage: .\build_web.ps1

param(
    [string]$Emsdk = "C:\dev\emsdk",
    [switch]$Clean = $false,
    [switch]$Watch = $false
)

# Check if build directory exists
if (-not (Test-Path "build_web")) {
    New-Item -ItemType Directory -Path "build_web" -Force | Out-Null
    Write-Host "Created build_web directory"
}

if ($Clean) {
    Write-Host "Cleaning build_web directory..."
    Remove-Item "build_web\*" -Force -Recurse
}

# Activate Emscripten SDK
Write-Host "Activating Emscripten SDK from: $Emsdk"
$emsdk_env = Join-Path $Emsdk "emsdk_env.bat"

if (-not (Test-Path $emsdk_env)) {
    Write-Error "emsdk_env.bat not found at $emsdk_env"
    Write-Host "Please provide correct path to emsdk with -Emsdk parameter"
    exit 1
}

# Run emsdk_env and capture the environment
$emcc_cmd = @"
cd "$pwd"
call `"$emsdk_env`"
emcc `
    -xc++ `
    -std=c++17 `
    -O3 `
    -s WASM=1 `
    -s USE_WEBGL2=1 `
    -s MIN_WEBGL_VERSION=2 `
    -s MAX_WEBGL_VERSION=2 `
    -s FULL_ES3=1 `
    -s ALLOW_MEMORY_GROWTH=1 `
    -s TOTAL_MEMORY=536870912 `
    -s EXPORTED_RUNTIME_METHODS=ccall,cwrap `
    --emrun `
    -I src `
    -I src/vendors/glad/include `
    -I src/vendors/glm `
    -I src/vendors/imgui `
    -I src/vendors/imgui/backends `
    --embed-file assets@/assets `
    -Wl,--export-all `
    src/main_web.cpp `
    src/App_Web.cpp `
    src/Camera.cpp `
    src/Render.cpp `
    src/Shader.cpp `
    src/Draw.cpp `
    src/vendors/imgui/imgui.cpp `
    src/vendors/imgui/imgui_draw.cpp `
    src/vendors/imgui/imgui_tables.cpp `
    src/vendors/imgui/imgui_widgets.cpp `
    src/vendors/imgui/backends/imgui_impl_opengl3.cpp `
    src/vendors/imgui/backends/imgui_impl_glfw.cpp `
    -xc `
    -std=c99 `
    src/vendors/glad/src/glad.c `
    -o build_web/app.js
"@

Write-Host "=== Building HCMUS for Web (Emscripten) ==="
Write-Host ""

$result = cmd /c $emcc_cmd 2>&1
Write-Host $result

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "=== Build successful! ===" -ForegroundColor Green
    Write-Host "Outputs:"
    Get-ChildItem "build_web\app.*" | ForEach-Object { Write-Host "  $_" }
    Write-Host ""
    Write-Host "To run locally:"
    Write-Host "  Option 1: Use emrun"
    Write-Host "    emrun --open browser build_web/app.html"
    Write-Host ""
    Write-Host "  Option 2: Python HTTP server"
    Write-Host "    python -m http.server 8000"
    Write-Host "    then open http://localhost:8000"
    Write-Host ""
    Write-Host "  Option 3: Node.js http-server"
    Write-Host "    npx http-server"
}
else {
    Write-Host ""
    Write-Host "=== Build failed! ===" -ForegroundColor Red
    exit 1
}
