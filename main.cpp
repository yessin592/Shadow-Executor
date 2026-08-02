// ================================================================
// XENO STYLE UI - EXACT COPY (NO EXTRA FEATURES)
// ================================================================
#include <windows.h>
#include <commctrl.h>
#include <string>   // <-- تم إضافة هذا السطر

#pragma comment(lib, "comctl32.lib")

HWND hMain, hStatus, hTabControl, hEditScript;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(10, 10, 10)));

        // القائمة الجانبية
        int y = 40;
        const char* menuItems[] = { "Dashboard", "Emulator", "Scripts", "Client Manager", "Settings" };
        for (int i = 0; i < 5; i++) {
            CreateWindowA("BUTTON", menuItems[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
                5, y, 120, 25, hwnd, (HMENU)(100 + i), NULL, NULL);
            y += 30;
        }

        // علامة التبويب
        hTabControl = CreateWindow(WC_TABCONTROL, NULL,
            WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH,
            135, 35, 370, 25, hwnd, (HMENU)999, GetModuleHandle(NULL), NULL);
        TCITEM tie = {0};
        tie.mask = TCIF_TEXT;
        tie.pszText = "Script 1";
        TabCtrl_InsertItem(hTabControl, 0, &tie);

        // منطقة التحرير
        hEditScript = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            135, 65, 370, 120, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        SendMessageA(hEditScript, EM_SETBKGNDCOLOR, 0, RGB(20, 20, 20));

        // الصف الأول من الأزرار
        CreateWindowA("BUTTON", "Execute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, 195, 80, 25, hwnd, (HMENU)2, NULL, NULL);
        CreateWindowA("BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            225, 195, 80, 25, hwnd, (HMENU)3, NULL, NULL);
        CreateWindowA("BUTTON", "Kill Rules", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            315, 195, 100, 25, hwnd, (HMENU)4, NULL, NULL);

        // الصف الثاني من الأزرار
        CreateWindowA("BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            135, 225, 80, 25, hwnd, (HMENU)5, NULL, NULL);
        CreateWindowA("BUTTON", "Open", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            225, 225, 80, 25, hwnd, (HMENU)6, NULL, NULL);
        CreateWindowA("BUTTON", "Abort", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            315, 225, 80, 25, hwnd, (HMENU)7, NULL, NULL);

        // شريط الحالة
        hStatus = CreateWindowA("STATIC", "Status: Ready",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            5, 260, 500, 20, hwnd, (HMENU)8, GetModuleHandle(NULL), NULL);

        break;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(10, 10, 10));
        SetTextColor(hdc, RGB(0, 255, 0));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id >= 100 && id < 105) {
            // أزرار القائمة الجانبية
            char buf[256];
            GetWindowTextA((HWND)lParam, buf, 256);
            std::string text = "Status: ";
            text += buf;
            SetWindowTextA(hStatus, text.c_str());
        }
        if (id == 2) { // Execute
            SetWindowTextA(hStatus, "Status: Script Executed");
        }
        if (id == 3) { // Clear
            SetWindowTextA(hEditScript, "");
            SetWindowTextA(hStatus, "Status: Cleared");
        }
        if (id == 4) { // Kill Rules
            SetWindowTextA(hStatus, "Status: Kill Rules Activated");
        }
        if (id == 5) { // Save
            SetWindowTextA(hStatus, "Status: Saved");
        }
        if (id == 6) { // Open
            SetWindowTextA(hStatus, "Status: Open");
        }
        if (id == 7) { // Abort
            SetWindowTextA(hStatus, "Status: Aborted");
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "XenoClone";
    RegisterClassA(&wc);

    hMain = CreateWindowExA(0, "XenoClone", "Xeno",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 320,
        NULL, NULL, hInst, NULL);

    ShowWindow(hMain, nCmdShow);
    UpdateWindow(hMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
