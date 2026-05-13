# ddraw FPS Overlay — Age of Empires 1

[![Build ddraw.dll](https://github.com/YOUR_USERNAME/aoe1-ddraw-fps/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/aoe1-ddraw-fps/actions/workflows/build.yml)

Hiển thị FPS real-time ở góc trên bên trái màn hình trong Age of Empires 1 (và các game DirectDraw khác). Không cần mod game, chỉ cần copy 1 file DLL.

```
┌──────────┐
│ FPS: 60  │
└──────────┘
```

**Màu FPS:**
- 🟢 ≥ 60 FPS — xanh lá
- 🟡 30–59 FPS — vàng  
- 🔴 < 30 FPS — đỏ

---

## Tải về (không cần build)

Vào **[Releases](../../releases/latest)** → tải `ddraw-msvc.dll` → đổi tên thành `ddraw.dll` → copy vào thư mục chứa `empires.exe`.

---

## Cài đặt

```
📁 Age of Empires\
    empires.exe       ← game gốc
    ddraw.dll         ← ← ← copy vào đây
```

Wrapper tự tìm và load `ddraw.dll` thật từ `C:\Windows\System32\` — không ảnh hưởng hệ thống.

**Gỡ cài đặt:** Xóa `ddraw.dll` trong thư mục game.

---

## Cách hoạt động

```
empires.exe
  └─ LoadLibrary("ddraw.dll")        ← wrapper (cùng thư mục)
        ├─ load System32\ddraw.dll   ← DirectDraw thật
        ├─ DDraw7Wrapper             ← bọc IDirectDraw7
        │     └─ CreateSurface()     ← phát hiện primary surface
        └─ SurfaceWrapper
              ├─ Flip()              ← full-screen: vẽ FPS + tick
              ├─ Blt()               ← windowed:    vẽ FPS + tick
              └─ BltFast()           ← phòng ngừa:  vẽ FPS + tick
```

FPS counter dùng **sliding 1-second window** — đếm số frame trong 1 giây vừa qua, cập nhật mỗi frame.

---

## Build từ source

### Yêu cầu
- Windows 10/11
- Visual Studio 2019+ **hoặc** MSYS2 với MinGW i686
- **Bắt buộc build 32-bit** (AoE1 là process Win32)

### MSVC (khuyên dùng)

Mở **x86 Native Tools Command Prompt for VS 2022**:

```bat
git clone https://github.com/YOUR_USERNAME/aoe1-ddraw-fps
cd aoe1-ddraw-fps

:: Dùng CMake
cmake -B build -A Win32
cmake --build build --config Release

:: Output: build\Release\ddraw.dll
```

### MinGW / MSYS2

```bash
# Cài trong MSYS2 MINGW32 shell
pacman -S mingw-w64-i686-gcc make

git clone https://github.com/YOUR_USERNAME/aoe1-ddraw-fps
cd aoe1-ddraw-fps

i686-w64-mingw32-g++ -std=c++17 -O2 -m32 -shared \
  -static-libgcc -static-libstdc++ -Wl,--kill-at \
  -o ddraw.dll \
  dllmain.cpp ddraw7_wrapper.cpp surface_wrapper.cpp \
  -lddraw -ldxguid -lgdi32 -luser32 -lkernel32
```

---

## CI/CD (GitHub Actions)

Mỗi commit lên `main` tự động:
1. Build với **MSVC x86** trên `windows-latest`
2. Build với **MinGW i686** qua MSYS2
3. Upload cả 2 artifact

Khi push tag `v*` (ví dụ `v1.0.0`):
- Tự động tạo **GitHub Release** với 2 file DLL đính kèm

```bash
# Tạo release mới
git tag v1.0.0
git push origin v1.0.0
```

---

## Tuỳ chỉnh

Mở `surface_wrapper.cpp`, hàm `RenderFpsOverlay()`:

```cpp
// Đổi vị trí overlay
RECT box = { 5, 5, 90, 24 };     // left, top, right, bottom
TextOutA(hdc, 8, 7, buf, ...);   // x, y pixel

// Đổi cỡ chữ (14 = chiều cao tính bằng pixel)
HFONT font = CreateFontA(14, 0, 0, 0, FW_BOLD, ...);

// Đổi ngưỡng màu
COLORREF col = fps >= 60.f ? RGB(0, 255, 80)    // xanh
             : fps >= 30.f ? RGB(255, 220, 0)   // vàng
                           : RGB(255, 60, 60);  // đỏ
```

---

## License

MIT
