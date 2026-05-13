// ============================================================
//  surface_wrapper.cpp
//  Wraps IDirectDrawSurface / IDirectDrawSurface2 / IDirectDrawSurface4
//  Intercepts Flip, Blt, BltFast → tick FPS + draw overlay
// ============================================================
#include "ddraw_wrapper.h"
#include "surface_wrapper.h"
#include <cstdio>

// ── helpers ──────────────────────────────────────────────────
void RenderFpsOverlay(HDC hdc, int w, int h)
{
    float fps = g_fps.fps();

    char buf[64];
    sprintf_s(buf, "FPS: %.0f", fps);

    // Semi-transparent black box
    HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
    RECT box = { 5, 5, 90, 24 };
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, bg);
    // Draw with 50% alpha via PatBlt trick isn't easily available in plain GDI.
    // Instead draw a solid dark rect, then text on top.
    FillRect(hdc, &box, bg);
    SelectObject(hdc, oldBr);
    DeleteObject(bg);

    // Choose colour: green < 30 fps = red, 30-59 = yellow, ≥ 60 = green
    COLORREF col = fps >= 60.f ? RGB(0, 255, 80)
                 : fps >= 30.f ? RGB(255, 220, 0)
                               : RGB(255, 60,  60);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, col);

    HFONT font = CreateFontA(
        14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        NONANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    TextOutA(hdc, 8, 7, buf, (int)strlen(buf));
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// ── SurfaceWrapper ───────────────────────────────────────────
SurfaceWrapper::SurfaceWrapper(IDirectDrawSurface7* real, bool isPrimary)
    : real_(real), isPrimary_(isPrimary), refCount_(1) {}

// IUnknown
HRESULT STDMETHODCALLTYPE SurfaceWrapper::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IDirectDrawSurface7) {
        *ppv = this; AddRef(); return S_OK;
    }
    return real_->QueryInterface(riid, ppv);
}
ULONG STDMETHODCALLTYPE SurfaceWrapper::AddRef()  { return ++refCount_; }
ULONG STDMETHODCALLTYPE SurfaceWrapper::Release()
{
    ULONG r = --refCount_;
    if (r == 0) { real_->Release(); delete this; }
    return r;
}

// ── Flip ─────────────────────────────────────────────────────
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Flip(LPDIRECTDRAWSURFACE7 pTargetOverride, DWORD dwFlags)
{
    if (isPrimary_) {
        // Draw overlay onto back buffer via GDI GetDC before flip
        HDC hdc = nullptr;
        if (SUCCEEDED(real_->GetDC(&hdc))) {
            DDSURFACEDESC2 desc = {};
            desc.dwSize = sizeof(desc);
            real_->GetSurfaceDesc(&desc);
            RenderFpsOverlay(hdc, desc.dwWidth, desc.dwHeight);
            real_->ReleaseDC(hdc);
        }
        g_fps.tick();
    }
    return real_->Flip(pTargetOverride, dwFlags);
}

// ── Blt ──────────────────────────────────────────────────────
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Blt(
    LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpSrc,
    LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
    HRESULT hr = real_->Blt(lpDestRect, lpSrc, lpSrcRect, dwFlags, lpDDBltFx);
    if (SUCCEEDED(hr) && isPrimary_) {
        HDC hdc = nullptr;
        if (SUCCEEDED(real_->GetDC(&hdc))) {
            DDSURFACEDESC2 desc = {};
            desc.dwSize = sizeof(desc);
            real_->GetSurfaceDesc(&desc);
            RenderFpsOverlay(hdc, desc.dwWidth, desc.dwHeight);
            real_->ReleaseDC(hdc);
        }
        g_fps.tick();
    }
    return hr;
}

// ── BltFast ───────────────────────────────────────────────────
HRESULT STDMETHODCALLTYPE SurfaceWrapper::BltFast(
    DWORD x, DWORD y, LPDIRECTDRAWSURFACE7 lpSrc,
    LPRECT lpSrcRect, DWORD dwFlags)
{
    HRESULT hr = real_->BltFast(x, y, lpSrc, lpSrcRect, dwFlags);
    if (SUCCEEDED(hr) && isPrimary_) {
        HDC hdc = nullptr;
        if (SUCCEEDED(real_->GetDC(&hdc))) {
            DDSURFACEDESC2 desc = {};
            desc.dwSize = sizeof(desc);
            real_->GetSurfaceDesc(&desc);
            RenderFpsOverlay(hdc, desc.dwWidth, desc.dwHeight);
            real_->ReleaseDC(hdc);
        }
        g_fps.tick();
    }
    return hr;
}

// ── Pass-through stubs ────────────────────────────────────────
HRESULT STDMETHODCALLTYPE SurfaceWrapper::AddAttachedSurface(LPDIRECTDRAWSURFACE7 p) { return real_->AddAttachedSurface(p); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::AddOverlayDirtyRect(LPRECT p) { return real_->AddOverlayDirtyRect(p); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::BltBatch(LPDDBLTBATCH p, DWORD a, DWORD b) { return real_->BltBatch(p, a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::BltZ(LPDIRECTDRAWSURFACE7 a, LONG b) { return real_->BltZ(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::ChangeUniquenessValue() { return real_->ChangeUniquenessValue(); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::DeleteAttachedSurface(DWORD a, LPDIRECTDRAWSURFACE7 b) { return real_->DeleteAttachedSurface(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK7 b) { return real_->EnumAttachedSurfaces(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK7 c) { return real_->EnumOverlayZOrders(a, b, c); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::FreePrivateData(REFGUID a) { return real_->FreePrivateData(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetAttachedSurface(LPDDSCAPS2 a, LPDIRECTDRAWSURFACE7* b) { return real_->GetAttachedSurface(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetBltStatus(DWORD a) { return real_->GetBltStatus(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetCaps(LPDDSCAPS2 a) { return real_->GetCaps(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetClipper(LPDIRECTDRAWCLIPPER* a) { return real_->GetClipper(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetColorKey(DWORD a, LPDDCOLORKEY b) { return real_->GetColorKey(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetDC(HDC* a) { return real_->GetDC(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetFlipStatus(DWORD a) { return real_->GetFlipStatus(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetOverlayPosition(LPLONG a, LPLONG b) { return real_->GetOverlayPosition(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetPalette(LPDIRECTDRAWPALETTE* a) { return real_->GetPalette(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetPixelFormat(LPDDPIXELFORMAT a) { return real_->GetPixelFormat(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetPriority(LPDWORD a) { return real_->GetPriority(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetPrivateData(REFGUID a, LPVOID b, LPDWORD c) { return real_->GetPrivateData(a, b, c); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetSurfaceDesc(LPDDSURFACEDESC2 a) { return real_->GetSurfaceDesc(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::GetUniquenessValue(LPDWORD a) { return real_->GetUniquenessValue(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC2 b) { return real_->Initialize(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::IsLost() { return real_->IsLost(); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Lock(LPRECT a, LPDDSURFACEDESC2 b, DWORD c, HANDLE d) { return real_->Lock(a, b, c, d); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::PageLock(DWORD a) { return real_->PageLock(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::PageUnlock(DWORD a) { return real_->PageUnlock(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::ReleaseDC(HDC a) { return real_->ReleaseDC(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Restore() { return real_->Restore(); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetClipper(LPDIRECTDRAWCLIPPER a) { return real_->SetClipper(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetColorKey(DWORD a, LPDDCOLORKEY b) { return real_->SetColorKey(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetOverlayPosition(LONG a, LONG b) { return real_->SetOverlayPosition(a, b); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetPalette(LPDIRECTDRAWPALETTE a) { return real_->SetPalette(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetPriority(DWORD a) { return real_->SetPriority(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::SetPrivateData(REFGUID a, LPVOID b, DWORD c, DWORD d) { return real_->SetPrivateData(a, b, c, d); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::Unlock(LPRECT a) { return real_->Unlock(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::UpdateOverlay(LPRECT a, LPDIRECTDRAWSURFACE7 b, LPRECT c, DWORD d, LPDDOVERLAYFX e) { return real_->UpdateOverlay(a, b, c, d, e); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::UpdateOverlayDisplay(DWORD a) { return real_->UpdateOverlayDisplay(a); }
HRESULT STDMETHODCALLTYPE SurfaceWrapper::UpdateOverlayZOrder(DWORD a, LPDIRECTDRAWSURFACE7 b) { return real_->UpdateOverlayZOrder(a, b); }
