#include <windows.h>
#include <string>

#define ID_EDIT_X 101
#define ID_BUTTON 102
#define ID_RESULT 103

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditX, hButton, hResult;

    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = ((LPCREATESTRUCTA)lParam)->hInstance;
        hEditX = CreateWindowExA(0, "EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
            50, 60, 120, 25, hwnd, (HMENU)ID_EDIT_X, hInst, nullptr);
        hButton = CreateWindowExA(0, "BUTTON", "Розрахувати", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            50, 105, 120, 30, hwnd, (HMENU)ID_BUTTON, hInst, nullptr);
        hResult = CreateWindowExA(0, "STATIC", "0", WS_CHILD | WS_VISIBLE,
            150, 155, 160, 25, hwnd, (HMENU)ID_RESULT, hInst, nullptr);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON) {
            char buf[32]{};
            GetWindowTextA(hEditX, buf, sizeof(buf));
            try {
                int x = std::stoi(buf);
                int f = (x % 2 == 0) ? 1 : 0;
                SetWindowTextA(hResult, std::to_string(f).c_str());
            }
            catch (...) {
                SetWindowTextA(hResult, "Помилка введення");
            }
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        TextOutA(hdc, 50, 30, "Введіть ціле число x:", 21);
        TextOutA(hdc, 50, 155, "Результат f(x):", 15);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "Lab3Task1Class";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0, "Lab3Task1Class", "Лабораторна 3 - Завдання 1 (Варіант 19)",
        WS_OVERLAPPEDWINDOW, 400, 250, 420, 260, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
