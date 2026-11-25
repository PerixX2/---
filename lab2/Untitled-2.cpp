#include <windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

class Road {
public:
    void Draw(HDC hdc, int x, int y) {
        HBRUSH gray = CreateSolidBrush(RGB(90, 90, 90));
        HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, gray);
        Rectangle(hdc, x, y, x + 800, y + 80);
        SelectObject(hdc, white);
        for (int i = x + 30; i < x + 780; i += 100)
            Rectangle(hdc, i, y + 35, i + 50, y + 45);
        DeleteObject(gray);
        DeleteObject(white);
    }
};

class House {
public:
    void Draw(HDC hdc, int x, int y, bool light) {
        HBRUSH wall = CreateSolidBrush(RGB(245, 222, 179));
        HBRUSH roof = CreateSolidBrush(RGB(165, 42, 42));
        HBRUSH win = CreateSolidBrush(light ? RGB(255, 230, 50) : RGB(135, 206, 250));
        SelectObject(hdc, wall);
        Rectangle(hdc, x, y + 50, x + 140, y + 160);
        SelectObject(hdc, roof);
        POINT p[3] = { {x - 10, y + 50}, {x + 150, y + 50}, {x + 70, y} };
        Polygon(hdc, p, 3);
        SelectObject(hdc, win);
        Rectangle(hdc, x + 45, y + 80, x + 95, y + 125);
        DeleteObject(wall);
        DeleteObject(roof);
        DeleteObject(win);
    }
};

class Fence {
public:
    void Draw(HDC hdc, int x, int y) {
        HBRUSH wood = CreateSolidBrush(RGB(205, 133, 63));
        SelectObject(hdc, wood);
        Rectangle(hdc, x, y + 20, x + 220, y + 32);
        Rectangle(hdc, x, y + 60, x + 220, y + 72);
        for (int i = x; i < x + 220; i += 35)
            Rectangle(hdc, i, y, i + 20, y + 90);
        DeleteObject(wood);
    }
};

class Snowman {
public:
    void Draw(HDC hdc, int x, int y) {
        HBRUSH snow = CreateSolidBrush(RGB(245, 250, 255));
        HBRUSH hat = CreateSolidBrush(RGB(60, 60, 80));
        SelectObject(hdc, snow);
        Ellipse(hdc, x, y + 65, x + 60, y + 125);
        Ellipse(hdc, x + 8, y + 28, x + 52, y + 72);
        Ellipse(hdc, x + 15, y, x + 45, y + 30);
        SelectObject(hdc, hat);
        Rectangle(hdc, x + 18, y - 15, x + 42, y + 3);
        DeleteObject(snow);
        DeleteObject(hat);
    }
};

int snowmanOffset = 0;
bool lightsOn = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        Road road;
        House h1, h2;
        Fence f1, f2;
        Snowman s1, s2;

        h1.Draw(hdc, 80, 120, lightsOn);
        h2.Draw(hdc, 460, 120, lightsOn);
        f1.Draw(hdc, 60, 270);
        f2.Draw(hdc, 440, 270);
        s1.Draw(hdc, 310 + snowmanOffset, 245);
        s2.Draw(hdc, 680 + snowmanOffset, 245);
        road.Draw(hdc, 0, 410);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_RIGHT) snowmanOffset += 15;
        else if (wParam == VK_LEFT) snowmanOffset -= 15;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_LBUTTONDOWN:
        lightsOn = !lightsOn;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_SIZE:
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Task2Variant19";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Task 2 (Variant 19) - Tumak Maksym",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 550, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
