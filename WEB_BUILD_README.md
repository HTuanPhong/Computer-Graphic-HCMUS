# Web Port - HCMUS Computer Graphics

This directory contains the Emscripten/WebGL2 port of the Computer Graphics application.

## Build Status ✅

The application has been successfully ported to run in web browsers using:
- **Emscripten** compiler for C++ → WebAssembly
- **WebGL2** for GPU rendering
- **Ems GLFW3** port for windowing/input
- **ImGui** with OpenGL3 backend

## Build Output Files

Located in `build_web/`:
- `app.js` - Main JavaScript loader (150 KB)
- `app.wasm` - Compiled WebAssembly module (1.1 MB)
- `index.html` - Entry point HTML file

## Running Locally

### Option 1: Using Python HTTP Server (Recommended)
```bash
python -m http.server 8000
```
Then open: `http://localhost:8000`

### Option 2: Using Node.js http-server
```bash
npm install -g http-server
http-server
```

### Option 3: Using Emscripten Runner
```bash
emrun --open browser index.html
```

## Key Porting Changes

### Code Separation
- **Web files**: `src/main_web.cpp`, `src/App_Web.cpp`
- **Desktop files** (unchanged): `src/main.cpp`, `src/App.cpp`

### Header Compatibility
- Conditional GL headers: GLES3 for web, glad for desktop
- Conditional glad.h includes guarded with `#ifndef __EMSCRIPTEN__`

### Emscripten-Specific Adaptations
1. **Main Loop**: Regular while loop → `emscripten_set_main_loop()` callback
2. **Time Tracking**: `glfwGetTime()` → `emscripten_get_now()`
3. **GL Functions**: Desktop OpenGL → WebGL2/GLES3
4. **Wireframe Mode**: `glPolygonMode()` disabled (not in WebGL)
5. **Event Handling**: ImGui callbacks from Emscripten

### Compilation Flags
```
-s WASM=1                      # WebAssembly output
-s USE_WEBGL2=1                # WebGL2 support
-s FULL_ES3=1                  # Full ES3 support  
-s USE_GLFW=3                  # GLFW3 port
-s ALLOW_MEMORY_GROWTH=1       # Dynamic memory
-s TOTAL_MEMORY=536870912      # 512 MB initial
--embed-file assets@/assets    # Bundle assets
```

##  Rebuilding

To rebuild the web version:

### Windows
```batch
.\build_web.bat
```

### Linux/Mac
```bash
chmod +x build_web.sh
./build_web.sh
```

Or activate Emscripten and run emcc directly:
```bash
source /path/to/emsdk/emsdk_env.sh
emcc -std=c++17 -O3 ...  # See build_web.bat for full command
```

## Performance Notes

- Initial load: ~2-5 seconds (downloading 1.1 MB wasm + runtime)
- FPS: Depends on GPU + browser JS engine (typically 30-60 FPS)
- Memory: 512 MB allocated for wasm linear memory
- File size: Minimal (~1.3 MB total for app + loader)
- Best supported: Chrome, Firefox, Safari 15+

## Browser Compatibility

| Browser | WebGL2 | GLES3 | Status |
|---------|--------|-------|--------|
| Chrome 56+ | ✓ | ✓ | ✅ Full support |
| Firefox 51+ | ✓ | ✓ | ✅ Full support |
| Safari 15+ | ✓ | ✓ | ✅ Supported |
| Edge 79+ | ✓ | ✓ | ✅ Supported |
| Mobile browsers | ~ | ~ | ⚠️ Limited |

## Known Limitations

1. **Desktop Build Still Works**: Both desktop and web builds are independent - desktop version still compiles/runs normally
2. **No Wireframe**: `glPolygonMode` unavailable in WebGL
3. **Asset Loading**: All assets embedded at compile time (fonts, shaders, etc.)
4. **File I/O**: Limited to embedded/virtual filesystem
5. **Mouse Lock**: Requires user interaction first

## Next Steps for Optimization

- [ ] Reduce wasm size with `-Oz` optimization
- [ ] Remove unused ImGui features with `IMGUI_DISABLE_*`
- [ ] Consider WebWorker threading with `pthread` support
- [ ] Add progressive loading UI
- [ ] Implement WebGL2 extensions for better perf

## Getting Help

For Emscripten issues:
- https://emscripten.org/docs/
- https://github.com/emscripten-core/emscripten/wiki

For WebGL/GLES3:
- https://khronos.org/webgl/
- https://khronos.org/opengl/wiki/OpenGL_ES
