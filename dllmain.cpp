#include "ddraw_wrapper.h"
#include "ddraw7_wrapper.h"

FpsCounter g_fps;

static HMODULE g_realDdraw = nullptr;

typedef HRESULT (WINAPI* PFN_DDCreate)        (GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT (WINAPI* PFN_DDCreateEx)      (GUID*, LPVOID*, REFIID, IUnknown*);
typedef HRESULT (WINAPI* PFN_DDCreateClipper) (DWORD, LPDIRECTDRAWCLIPPER*, IUnknown*);
typedef HRESULT (WINAPI* PFN_DDEnumA)         (LPDDENUMCALLBACKA, LPVOID);
typedef HRESULT (WINAPI* PFN_DDEnumExA)       (LPDDENUMCALLBACKEXA, LPVOID, DWORD);

static PFN_DDCreate        real_DDCreate        = nullptr;
static PFN_DDCreateEx      real_DDCreateEx      = nullptr;
static PFN_DDCreateClipper real_DDCreateClipper = nullptr;
static PFN_DDEnumA         real_DDEnumA         = nullptr;
static PFN_DDEnumExA       real_DDEnumExA       = nullptr;

static bool LoadRealDdraw()
{
    if (g_realDdraw) return true;
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\ddraw.dll");
    g_realDdraw = LoadLibraryA(path);
    if (!g_realDdraw) return false;
    real_DDCreate        = (PFN_DDCreate)       GetProcAddress(g_realDdraw, "DirectDrawCreate");
    real_DDCreateEx      = (PFN_DDCreateEx)     GetProcAddress(g_realDdraw, "DirectDrawCreateEx");
    real_DDCreateClipper = (PFN_DDCreateClipper)GetProcAddress(g_realDdraw, "DirectDrawCreateClipper");
    real_DDEnumA         = (PFN_DDEnumA)        GetProcAddress(g_realDdraw, "DirectDrawEnumerateA");
    real_DDEnumExA       = (PFN_DDEnumExA)      GetProcAddress(g_realDdraw, "DirectDrawEnumerateExA");
    return true;
}

extern "C"
HRESULT WINAPI Wrap_DirectDrawCreateEx(GUID* lpGUID, LPVOID* lplpDD, REFIID iid, IUnknown* pUnkOuter)
{
    if (!LoadRealDdraw() || !real_DDCreateEx) return DDERR_GENERIC;
    HRESULT hr = real_DDCreateEx(lpGUID, lplpDD, iid, pUnkOuter);
    if (FAILED(hr)) return hr;
    if (iid == IID_IDirectDraw7) {
        IDirectDraw7* real7 = static_cast<IDirectDraw7*>(*lplpDD);
        *lplpDD = static_cast<IDirectDraw7*>(new DDraw7Wrapper(real7));
    }
    return DD_OK;
}

extern "C"
HRESULT WINAPI Wrap_DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
{
    if (!LoadRealDdraw() || !real_DDCreate) return DDERR_GENERIC;
    return real_DDCreate(lpGUID, lplpDD, pUnkOuter);
}

extern "C"
HRESULT WINAPI Wrap_DirectDrawCreateClipper(DWORD a, LPDIRECTDRAWCLIPPER* b, IUnknown* c)
{
    if (!LoadRealDdraw() || !real_DDCreateClipper) return DDERR_GENERIC;
    return real_DDCreateClipper(a, b, c);
}

extern "C"
HRESULT WINAPI Wrap_DirectDrawEnumerateA(LPDDENUMCALLBACKA cb, LPVOID ctx)
{
    if (!LoadRealDdraw() || !real_DDEnumA) return DDERR_GENERIC;
    return real_DDEnumA(cb, ctx);
}

extern "C"
HRESULT WINAPI Wrap_DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA cb, LPVOID ctx, DWORD flags)
{
    if (!LoadRealDdraw() || !real_DDEnumExA) return DDERR_GENERIC;
    return real_DDEnumExA(cb, ctx, flags);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)   LoadRealDdraw();
    else if (reason == DLL_PROCESS_DETACH && g_realDdraw) FreeLibrary(g_realDdraw);
    return TRUE;
}
