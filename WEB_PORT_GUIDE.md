# HCMUS Computer Graphics - Web & Desktop

This project supports both **desktop** and **web** builds:

## 🖥️ Desktop Build (Native)
```bash
# Build and run native OpenGL version
clang-cl.exe -c src/main.cpp ...  # or use VS Code build task
```

## 🌐 Web Build (Emscripten/WebGL2) ✨ NEW

### Quick Start
```bash
# Windows
.\build_web.bat

# Linux/Mac (with Emscripten activated)
./build_web.sh
```

### Run Locally
```bash
# Option 1: Python server
python -m http.server 8000
# Open http://localhost:8000

# Option 2: Node http-server  
npx http-server

# Option 3: Emscripten runner
emrun --open browser index.html
```

## 📁 File Structure

```
├── src/
│   ├── main.cpp          ← Desktop entry point
│   ├── main_web.cpp      ← Web entry point (Emscripten)
│   ├── App.cpp           ← Desktop app framework
│   ├── App_Web.cpp       ← Web app framework
│   ├── *.hpp/*.cpp       ← Shared code (with #ifdef __EMSCRIPTEN__)
│   └── vendors/          ← ImGui, GLM, GLFW, GLAD
├── assets/               ← Shaders, fonts (embedded in web)
├── build_web/            ← Web build output (app.js, app.wasm)
├── index.html            ← Web entry HTML
├── build_web.bat         ← Windows web build script
├── build_web.sh          ← Linux/Mac web build script
└── WEB_BUILD_README.md   ← Detailed web port docs
```

## ✅ What Works on Web

- ✅ ImGui interface & controls
- ✅ 3D rendering (WebGL2)
- ✅ Mouse/keyboard input (via GLFW port)
- ✅ Shader compilation & execution
- ✅ Camera controls
- ✅ Scene geometry editing
- ✅ All embedded assets

## ⚠️ Web Limitations

- ❌ `glPolygonMode` (disabled - not in WebGL)
- ⚠️ Mobile browsers (limited support)
- ⚠️ Large file I/O (use embedded assets only)

## 🔧 Requirements

### Desktop Build
- Visual Studio or Clang
- GLAD headers/libraries
- GLFW3
- OpenGL 3.3+

### Web Build  
- Emscripten SDK (`C:\dev\emsdk` configured)
- Python or Node.js (for local server)

## 📚 Documentation

- **Desktop** - Use existing docs/README.md
- **Web** - See [WEB_BUILD_README.md](WEB_BUILD_README.md)

---

**Both versions are independent**  - Desktop native build is unchanged. This is a true port with minimal modifications.

For questions on web deployment, see WEB_BUILD_README.md
