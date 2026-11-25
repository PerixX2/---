#include <windows.h>

#define ID_IMG_SHOW    201
#define ID_TR_COLOR    202
#define ID_TR_ADD      203
#define ID_INFO_ABOUT  204
#define ID_INFO_EXIT   205

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
    void Draw(HDC hdc, int x, int y, COLORREF winColor) {
        HBRUSH wall = CreateSolidBrush(RGB(245, 222, 179));
        HBRUSH roof = CreateSolidBrush(RGB(165, 42, 42));
        HBRUSH win = CreateSolidBrush(winColor);
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

bool showScene = false;
bool altColor = false;
bool extraSnowman = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        HMENU hMenu = CreateMenu();
        HMENU hImgMenu = CreatePopupMenu();
        HMENU hTrMenu = CreatePopupMenu();
        HMENU hInfoMenu = CreatePopupMenu();

        AppendMenuA(hImgMenu, MF_STRING, ID_IMG_SHOW, "Рисунок");
        AppendMenuA(hTrMenu, MF_STRING, ID_TR_COLOR, "Зміна кольору");
        AppendMenuA(hTrMenu, MF_STRING, ID_TR_ADD, "Додати сніговика");
        AppendMenuA(hInfoMenu, MF_STRING, ID_INFO_ABOUT, "Про програму");
        AppendMenuA(hInfoMenu, MF_STRING, ID_INFO_EXIT, "Вихід");

        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hImgMenu, "Зображення");
        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hTrMenu, "Трансформації");
        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hInfoMenu, "Інформація");
        SetMenu(hwnd, hMenu);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_IMG_SHOW:
            showScene = true;
            altColor = false;
            extraSnowman = false;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_TR_COLOR:
            altColor = !altColor;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_TR_ADD:
            extraSnowman = true;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_INFO_ABOUT:
            MessageBoxA(hwnd, "Лабораторна робота №3. Варіант 19. Виконав: Тумак Максим, ФІТ 3-17", "Про програму", MB_OK);
            break;
        case ID_INFO_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (showScene) {
            COLORREF winColor = altColor ? RGB(255, 220, 50) : RGB(135, 206, 250);
            Road road;
            House h1, h2;
            Fence f1, f2;
            Snowman s1, s2, s3;

            h1.Draw(hdc, 80, 100, winColor);
            h2.Draw(hdc, 460, 100, winColor);
            f1.Draw(hdc, 60, 250);
            f2.Draw(hdc, 440, 250);
            s1.Draw(hdc, 310, 225);
            s2.Draw(hdc, 680, 225);
            if (extraSnowman) s3.Draw(hdc, 380, 225);
            road.Draw(hdc, 0, 390);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "Lab3Task2Class";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0, "Lab3Task2Class", "Лабораторна 3 - Завдання 2 (Варіант 19)",
        WS_OVERLAPPEDWINDOW, 200, 150, 800, 540, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
