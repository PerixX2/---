#include <windows.h>
#include <math.h>
#include <stdio.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        int originX = 100, originY = 80;
        double scaleX = 250.0, scaleY = 120.0;

        HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HPEN graphPen = CreatePen(PS_SOLID, 2, RGB(220, 20, 60));

        // Малюємо осі X та Y
        SelectObject(hdc, axisPen);
        MoveToEx(hdc, originX - 20, originY, NULL);
        LineTo(hdc, originX + 550, originY);
        MoveToEx(hdc, originX, originY - 30, NULL);
        LineTo(hdc, originX, originY + 400);

        TextOut(hdc, originX + 555, originY - 8, L"X", 1);
        TextOut(hdc, originX - 15, originY - 35, L"Y", 1);
        TextOut(hdc, originX - 15, originY + 5, L"0", 1);

        // Розмітка осей
        wchar_t buf[16];
        for (double x = 0.5; x <= 2.01; x += 0.5) {
            int sx = originX + (int)(x * scaleX);
            MoveToEx(hdc, sx, originY - 4, NULL);
            LineTo(hdc, sx, originY + 4);
            swprintf_s(buf, L"%.1f", x);
            TextOut(hdc, sx - 10, originY + 8, buf, (int)wcslen(buf));
        }
        for (double y = -3.0; y <= -0.49; y += 0.5) {
            int sy = originY - (int)(y * scaleY);
            MoveToEx(hdc, originX - 4, sy, NULL);
            LineTo(hdc, originX + 4, sy);
            swprintf_s(buf, L"%.1f", y);
            TextOut(hdc, originX - 35, sy - 8, buf, (int)wcslen(buf));
        }

        // Малюємо графік y = (x - 3) / (1 + x^2) на [0; 2]
        SelectObject(hdc, graphPen);
        for (double x = 0.0; x <= 2.001; x += 0.02) {
            double y = (x - 3.0) / (1.0 + x * x);
            int sx = originX + (int)(x * scaleX);
            int sy = originY - (int)(y * scaleY);
            if (x == 0.0) MoveToEx(hdc, sx, sy, NULL);
            else LineTo(hdc, sx, sy);
        }

        DeleteObject(axisPen);
        DeleteObject(graphPen);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Task1Variant19";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Task 1 (Variant 19) - Tumak Maksym",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 750, 550, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
