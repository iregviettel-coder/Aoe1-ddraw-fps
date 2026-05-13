// ============================================================
//  dllmain.cpp  –  entry point + exported DirectDraw functions
//  Build: 32-bit DLL, link against ddraw.lib / dxguid.lib
// ============================================================
#include "ddraw_wrapper.h"
#include "ddraw7_wrapper.h"

// ── Global FPS counter ────────────────────────────────────────
FpsCounter g_fps;

// ── Real ddraw.dll loaded from System32 ─────────────────────
static HMODULE g_realDdraw = nullptr;

using PFN_DirectDrawCreate        = HRESULT (WINAPI*)(GUID*, LPDIRECTDRAW*, IUnknown*);
using PFN_DirectDrawCreateEx      = HRESULT (WINAPI*)(GUID*, LPVOID*, REFIID, IUnknown*);
using PFN_DirectDrawCreateClipper = HRESULT (WINAPI*)(DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*);
using PFN_DirectDrawEnumerateA    = HRESULT (WINAPI*)(LPDDENUMCALLBACKA, LPVOID);
using PFN_DirectDrawEnumerateExA  = HRESULT (WINAPI*)(LPDDENUMCALLBACKEXA, LPVOID, DWORD);

static PFN_DirectDrawCreate        real_DirectDrawCreate        = nullptr;
static PFN_DirectDrawCreateEx      real_DirectDrawCreateEx      = nullptr;
static PFN_DirectDrawCreateClipper real_DirectDrawCreateClipper = nullptr;
static PFN_DirectDrawEnumerateA    real_DirectDrawEnumerateA    = nullptr;
static PFN_DirectDrawEnumerateExA  real_DirectDrawEnumerateExA  = nullptr;

static bool LoadRealDdraw()
{
    if (g_realDdraw) return true;

    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\ddraw.dll");

    g_realDdraw = LoadLibraryA(path);
    if (!g_realDdraw) return false;

#define GETPROC(name) real_##name = (PFN_##name)GetProcAddress(g_realDdraw, #name)
    GETPROC(DirectDrawCreate);
    GETPROC(DirectDrawCreateEx);
    GETPROC(DirectDrawCreateClipper);
    GETPROC(DirectDrawEnumerateA);
    GETPROC(DirectDrawEnumerateExA);
#undef GETPROC
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Exported: DirectDrawCreateEx
//  Age of Empires 1 uses this to get IDirectDraw7
// ─────────────────────────────────────────────────────────────
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectDrawCreateEx(GUID* lpGUID, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
{
    if (!LoadRealDdraw() || !real_DirectDrawCreateEx)
        return DDERR_GENERIC;

    HRESULT hr = real_DirectDrawCreateEx(lpGUID, lplpDD, iid, pUnkOuter);
    if (FAILED(hr)) return hr;

    if (iid == IID_IDirectDraw7) {
        IDirectDraw7* real7 = static_cast<IDirectDraw7*>(*lplpDD);
        *lplpDD = static_cast<IDirectDraw7*>(new DDraw7Wrapper(real7));
    }
    return DD_OK;
}

// ─────────────────────────────────────────────────────────────
//  Exported: DirectDrawCreate  (older path, also wrapped)
// ─────────────────────────────────────────────────────────────
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
{
    if (!LoadRealDdraw() || !real_DirectDrawCreate)
        return DDERR_GENERIC;

    // Forward as-is; AoE1 usually falls back to DirectDrawCreateEx
    return real_DirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
}

// ── Remaining pass-throughs ───────────────────────────────────
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectDrawCreateClipper(DWORD a, LPDIRECTDRAWCLIPPER* b, IUnknown* c)
{
    if (!LoadRealDdraw() || !real_DirectDrawCreateClipper) return DDERR_GENERIC;
    return real_DirectDrawCreateClipper(a, b, c);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI DirectDrawEnumerateA(LPDDENUMCALLBACKA cb, LPVOID ctx)
{
    if (!LoadRealDdraw() || !real_DirectDrawEnumerateA) return DDERR_GENERIC;
    return real_DirectDrawEnumerateA(cb, ctx);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA cb, LPVOID ctx, DWORD flags)
{
    if (!LoadRealDdraw() || !real_DirectDrawEnumerateExA) return DDERR_GENERIC;
    return real_DirectDrawEnumerateExA(cb, ctx, flags);
}

// ─────────────────────────────────────────────────────────────
//  DLL main
// ─────────────────────────────────────────────────────────────
BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        LoadRealDdraw();
    else if (reason == DLL_PROCESS_DETACH && g_realDdraw)
        FreeLibrary(g_realDdraw);
    return TRUE;
}
