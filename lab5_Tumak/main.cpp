#include <windows.h>
#include <cmath>
#include <cwchar>
#include <vector>

struct PendulumParams {
    double g = 9.81;
    double L = 1.5;
    double damping = 0.15;
    double forceAmp = 2.5;
    double forceFreq = 2.56;
    bool paused = false;
} params;

struct PendulumState {
    double theta = 0.2;
    double omega = 0.0;
    double t = 0.0;
    double maxAmp = 0.2;
} state;

std::vector<POINT> trail;

void stepPhysics(double dt) {
    if (params.paused) return;
    double w0_sq = params.g / params.L;
    auto acc = [&](double th, double om, double time) {
        return -w0_sq * std::sin(th) - params.damping * om + params.forceAmp * std::cos(params.forceFreq * time);
    };

    double k1_th = state.omega;
    double k1_om = acc(state.theta, state.omega, state.t);

    double k2_th = state.omega + 0.5 * dt * k1_om;
    double k2_om = acc(state.theta + 0.5 * dt * k1_th, k2_th, state.t + 0.5 * dt);

    double k3_th = state.omega + 0.5 * dt * k2_om;
    double k3_om = acc(state.theta + 0.5 * dt * k2_th, k3_th, state.t + 0.5 * dt);

    double k4_th = state.omega + dt * k3_om;
    double k4_om = acc(state.theta + dt * k3_th, k4_th, state.t + dt);

    state.theta += (dt / 6.0) * (k1_th + 2.0 * k2_th + 2.0 * k3_th + k4_th);
    state.omega += (dt / 6.0) * (k1_om + 2.0 * k2_om + 2.0 * k3_om + k4_om);
    state.t += dt;

    if (std::abs(state.theta) > state.maxAmp) {
        state.maxAmp = std::abs(state.theta);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        if (wp == VK_UP) params.forceAmp += 0.2;
        if (wp == VK_DOWN) params.forceAmp = (params.forceAmp - 0.2 > 0) ? params.forceAmp - 0.2 : 0.0;
        if (wp == VK_RIGHT) params.forceFreq += 0.1;
        if (wp == VK_LEFT) params.forceFreq = (params.forceFreq - 0.1 > 0.1) ? params.forceFreq - 0.1 : 0.1;
        if (wp == 'P') params.paused = !params.paused;
        if (wp == 'R') { state = PendulumState{}; trail.clear(); }
    } else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"PendulumLab5Var19";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(
        L"PendulumLab5Var19",
        L"Лабораторна робота №5 - Варіант 19: Маятник під дією періодичної сили",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        150, 100, 820, 620,
        nullptr, nullptr, hInst, nullptr
    );
    ShowWindow(hwnd, nShow);

    LARGE_INTEGER freq, prev, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    MSG msg{};
    while (msg.message != WM_QUIT) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        QueryPerformanceCounter(&now);
        double dt = double(now.QuadPart - prev.QuadPart) / freq.QuadPart;
        prev = now;
        if (dt > 0.05) dt = 0.05;

        stepPhysics(dt);

        HDC hdc = GetDC(hwnd);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, 800, 600);
        SelectObject(mem, bmp);

        HBRUSH bg = CreateSolidBrush(RGB(18, 22, 30));
        RECT rc{0, 0, 800, 600};
        FillRect(mem, &rc, bg);
        DeleteObject(bg);

        int px = 400, py = 160;
        double lenPx = params.L * 180.0;
        int bx = px + int(lenPx * std::sin(state.theta));
        int by = py + int(lenPx * std::cos(state.theta));

        if (!params.paused) {
            trail.push_back({bx, by});
            if (trail.size() > 140) trail.erase(trail.begin());
        }

        HPEN tPen = CreatePen(PS_SOLID, 2, RGB(70, 170, 230));
        SelectObject(mem, tPen);
        for (size_t i = 1; i < trail.size(); ++i) {
            MoveToEx(mem, trail[i - 1].x, trail[i - 1].y, nullptr);
            LineTo(mem, trail[i].x, trail[i].y);
        }
        DeleteObject(tPen);

        HPEN rPen = CreatePen(PS_SOLID, 3, RGB(210, 220, 230));
        SelectObject(mem, rPen);
        MoveToEx(mem, px, py, nullptr);
        LineTo(mem, bx, by);
        DeleteObject(rPen);

        double fForce = params.forceAmp * std::cos(params.forceFreq * state.t);
        HPEN fPen = CreatePen(PS_SOLID, 4, RGB(255, 85, 85));
        SelectObject(mem, fPen);
        MoveToEx(mem, bx, by, nullptr);
        LineTo(mem, bx + int(fForce * 28.0), by);
        DeleteObject(fPen);

        HBRUSH pivotBr = CreateSolidBrush(RGB(140, 150, 165));
        SelectObject(mem, pivotBr);
        Ellipse(mem, px - 6, py - 6, px + 6, py + 6);
        DeleteObject(pivotBr);

        HBRUSH bobBr = CreateSolidBrush(RGB(255, 165, 45));
        SelectObject(mem, bobBr);
        Ellipse(mem, bx - 18, by - 18, bx + 18, by + 18);
        DeleteObject(bobBr);

        SetBkMode(mem, TRANSPARENT);
        SetTextColor(mem, RGB(235, 240, 245));
        wchar_t buf[512];
        double w0 = std::sqrt(params.g / params.L);
        swprintf(buf, 512,
            L"Варіант 19: Маятник під дією періодичної сили\n"
            L"Амплітуда сили (Вгору/Вниз): %.2f рад/с^2\n"
            L"Частота сили (Вліво/Вправо): %.2f рад/с  |  Власна w0: %.2f рад/с\n"
            L"Поточний кут: %.3f рад  |  Макс. амплітуда: %.3f рад\n"
            L"Керування: [P] Пауза/Старт   [R] Скидання",
            params.forceAmp, params.forceFreq, w0, state.theta, state.maxAmp);
        RECT tr{20, 15, 780, 145};
        DrawTextW(mem, buf, -1, &tr, DT_LEFT | DT_TOP);

        BitBlt(hdc, 0, 0, 800, 600, mem, 0, 0, SRCCOPY);
        DeleteObject(bmp);
        DeleteDC(mem);
        ReleaseDC(hwnd, hdc);
        Sleep(16);
    }
    return 0;
}