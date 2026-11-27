// MelonXbox stage 1: UWP window + D3D11 swapchain + dual-screen test layout
// + right-stick cursor on the bottom screen. No emulation yet.
#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.Display.h>
#include <d3d11_2.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>

#include "TouchCursor.h"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Graphics::Display;
using Microsoft::WRL::ComPtr;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    IFrameworkView CreateView() { return *this; }

    void Initialize(CoreApplicationView const&) {}
    void Load(hstring const&) {}
    void Uninitialize() {}
    void SetWindow(CoreWindow const& window) { window_ = window; }

    void Run()
    {
        window_.Activate();
        InitD3D();

        auto last = std::chrono::steady_clock::now();
        while (true)
        {
            window_.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            float sx = 0, sy = 0;
            bool tap = false, slow = false;
            auto pads = Gamepad::Gamepads();
            if (pads.Size() > 0)
            {
                auto r = pads.GetAt(0).GetCurrentReading();
                sx = float(r.RightThumbstickX);
                sy = float(r.RightThumbstickY);
                tap = r.RightTrigger > 0.5;
                slow = r.LeftTrigger > 0.5;
            }
            cursor_.Update(sx, sy, dt, slow);

            Render(tap);
            swap_->Present(1, 0);
        }
    }

private:
    CoreWindow window_{ nullptr };
    ComPtr<ID3D11Device> dev_;
    ComPtr<ID3D11DeviceContext1> ctx_;
    ComPtr<IDXGISwapChain1> swap_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    mxb::TouchCursor cursor_;
    int w_ = 1920, h_ = 1080;

    void InitD3D()
    {
        auto b = window_.Bounds();
        float scale = float(int(DisplayInformation::GetForCurrentView().ResolutionScale())) / 100.0f;
        w_ = int(b.Width * scale);
        h_ = int(b.Height * scale);

        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                       D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
        ComPtr<ID3D11Device> dev;
        ComPtr<ID3D11DeviceContext> ctx;
        check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, ARRAYSIZE(levels),
            D3D11_SDK_VERSION, &dev, nullptr, &ctx));
        dev_ = dev;
        check_hresult(ctx.As(&ctx_));

        ComPtr<IDXGIDevice1> dxgiDev;
        check_hresult(dev.As(&dxgiDev));
        ComPtr<IDXGIAdapter> adapter;
        check_hresult(dxgiDev->GetAdapter(&adapter));
        ComPtr<IDXGIFactory2> factory;
        check_hresult(adapter->GetParent(IID_PPV_ARGS(&factory)));

        DXGI_SWAP_CHAIN_DESC1 sd{};
        sd.Width = UINT(w_);
        sd.Height = UINT(h_);
        sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 2;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        check_hresult(factory->CreateSwapChainForCoreWindow(
            dev.Get(), get_unknown(window_), &sd, nullptr, &swap_));

        ComPtr<ID3D11Texture2D> bb;
        check_hresult(swap_->GetBuffer(0, IID_PPV_ARGS(&bb)));
        check_hresult(dev_->CreateRenderTargetView(bb.Get(), nullptr, &rtv_));
    }

    void Fill(int l, int t, int r, int b, float cr, float cg, float cb)
    {
        RECT rc{ l, t, r, b };
        float c[4] = { cr, cg, cb, 1.0f };
        ctx_->ClearView(rtv_.Get(), c, &rc, 1);
    }

    void Render(bool tap)
    {
        float bg[4] = { 0.06f, 0.06f, 0.08f, 1.0f };
        ctx_->ClearRenderTargetView(rtv_.Get(), bg);

        // Two 256x192 screens stacked, scaled to fit.
        int s = std::max(1, std::min(w_ / 256, h_ / 384));
        int sw = 256 * s, sh = 192 * s;
        int x0 = (w_ - sw) / 2;
        int yTop = (h_ - 2 * sh) / 2;
        int yBot = yTop + sh;

        // Top screen: 8 colour bars (the "test pattern").
        static const float bars[8][3] = {
            {1,1,1}, {1,1,0}, {0,1,1}, {0,1,0}, {1,0,1}, {1,0,0}, {0,0,1}, {0.1f,0.1f,0.1f} };
        for (int i = 0; i < 8; i++)
            Fill(x0 + i * sw / 8, yTop, x0 + (i + 1) * sw / 8, yTop + sh,
                 bars[i][0], bars[i][1], bars[i][2]);

        // Bottom screen: dark panel with a border.
        Fill(x0, yBot, x0 + sw, yBot + sh, 0.10f, 0.25f, 0.22f);
        Fill(x0, yBot, x0 + sw, yBot + 2, 1, 1, 1);
        Fill(x0, yBot + sh - 2, x0 + sw, yBot + sh, 1, 1, 1);
        Fill(x0, yBot, x0 + 2, yBot + sh, 1, 1, 1);
        Fill(x0 + sw - 2, yBot, x0 + sw, yBot + sh, 1, 1, 1);

        // Cursor crosshair, red while tapping.
        int cx = x0 + int(cursor_.X() * s);
        int cy = yBot + int(cursor_.Y() * s);
        int arm = 10 * s / 2 + 6, th = std::max(2, s);
        float cr = tap ? 1.0f : 1.0f, cg = tap ? 0.1f : 1.0f, cb = tap ? 0.1f : 0.2f;
        Fill(cx - arm, cy - th / 2, cx + arm, cy + th / 2 + 1, cr, cg, cb);
        Fill(cx - th / 2, cy - arm, cx + th / 2 + 1, cy + arm, cr, cg, cb);
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
    return 0;
}
