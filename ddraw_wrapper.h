#pragma once
// ============================================================
//  ddraw_wrapper.h  –  FPS Overlay wrapper for Age of Empires 1
//  Drop ddraw.dll next to empires.exe (or in System32 backup folder)
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include <string>
#include <chrono>
#include <deque>
#include <mutex>

// ── FPS counter ──────────────────────────────────────────────
class FpsCounter
{
public:
    void tick()
    {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lk(mtx_);
        timestamps_.push_back(now);
        // keep only last 1 second
        auto cutoff = now - std::chrono::seconds(1);
        while (!timestamps_.empty() && timestamps_.front() < cutoff)
            timestamps_.pop_front();
    }

    float fps() const
    {
        std::lock_guard<std::mutex> lk(mtx_);
        return static_cast<float>(timestamps_.size());
    }

private:
    mutable std::mutex          mtx_;
    std::deque<std::chrono::steady_clock::time_point> timestamps_;
};

extern FpsCounter g_fps;

// ── GDI overlay renderer ─────────────────────────────────────
void RenderFpsOverlay(HDC hdc, int surfaceW, int surfaceH);
