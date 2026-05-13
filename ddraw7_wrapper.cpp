// ============================================================
//  ddraw7_wrapper.cpp  –  wraps IDirectDraw7
// ============================================================
#include "ddraw_wrapper.h"
#include "surface_wrapper.h"
#include "ddraw7_wrapper.h"

DDraw7Wrapper::DDraw7Wrapper(IDirectDraw7* real)
    : real_(real), refCount_(1) {}

// IUnknown
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDirectDraw7) {
        *ppv = this; AddRef(); return S_OK;
    }
    HRESULT hr = real_->QueryInterface(riid, ppv);
    return hr;
}
ULONG STDMETHODCALLTYPE DDraw7Wrapper::AddRef()  { return ++refCount_; }
ULONG STDMETHODCALLTYPE DDraw7Wrapper::Release()
{
    ULONG r = --refCount_;
    if (r == 0) { real_->Release(); delete this; }
    return r;
}

// ── CreateSurface – inject SurfaceWrapper for primary surfaces ──
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::CreateSurface(
    LPDDSURFACEDESC2 pDDSD, LPDIRECTDRAWSURFACE7* ppSurf, IUnknown* pUnkOuter)
{
    IDirectDrawSurface7* realSurf = nullptr;
    HRESULT hr = real_->CreateSurface(pDDSD, &realSurf, pUnkOuter);
    if (FAILED(hr)) return hr;

    // Detect primary surface (the one that gets displayed)
    bool isPrimary = (pDDSD->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0;
    *ppSurf = new SurfaceWrapper(realSurf, isPrimary);
    return DD_OK;
}

// ── Pass-through stubs ────────────────────────────────────────
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::Compact() { return real_->Compact(); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER* b, IUnknown* c) { return real_->CreateClipper(a, b, c); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE* c, IUnknown* d) { return real_->CreatePalette(a, b, c, d); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::DuplicateSurface(LPDIRECTDRAWSURFACE7 a, LPDIRECTDRAWSURFACE7* b) { return real_->DuplicateSurface(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d) { return real_->EnumDisplayModes(a, b, c, d); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d) { return real_->EnumSurfaces(a, b, c, d); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::FlipToGDISurface() { return real_->FlipToGDISurface(); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetCaps(LPDDCAPS a, LPDDCAPS b) { return real_->GetCaps(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetDisplayMode(LPDDSURFACEDESC2 a) { return real_->GetDisplayMode(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetFourCCCodes(LPDWORD a, LPDWORD b) { return real_->GetFourCCCodes(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetGDISurface(LPDIRECTDRAWSURFACE7* a) { return real_->GetGDISurface(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetMonitorFrequency(LPDWORD a) { return real_->GetMonitorFrequency(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetScanLine(LPDWORD a) { return real_->GetScanLine(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetSurfaceFromDC(HDC a, LPDIRECTDRAWSURFACE7* b) { return real_->GetSurfaceFromDC(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::GetVerticalBlankStatus(LPBOOL a) { return real_->GetVerticalBlankStatus(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::Initialize(GUID* a) { return real_->Initialize(a); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::RestoreAllSurfaces() { return real_->RestoreAllSurfaces(); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::RestoreDisplayMode() { return real_->RestoreDisplayMode(); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::SetCooperativeLevel(HWND a, DWORD b) { return real_->SetCooperativeLevel(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return real_->SetDisplayMode(a, b, c, d, e); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::StartModeTest(LPSIZE a, DWORD b, DWORD c) { return real_->StartModeTest(a, b, c); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::TestCooperativeLevel() { return real_->TestCooperativeLevel(); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::EvaluateMode(DWORD a, DWORD* b) { return real_->EvaluateMode(a, b); }
HRESULT STDMETHODCALLTYPE DDraw7Wrapper::WaitForVerticalBlank(DWORD a, HANDLE b) { return real_->WaitForVerticalBlank(a, b); }
