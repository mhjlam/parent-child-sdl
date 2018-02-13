#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tchar.h>

#include "resource.h"

static const std::wstring WindowTitle = L"Parent";
static const std::wstring WindowClass = L"Parent";

static std::vector<HWND> ChildHandles = std::vector<HWND>();

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

std::wstring LastErrorMessage()
{
    DWORD id = GetLastError();
    if (id == 0) return std::wstring();

    LPTSTR buffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                FORMAT_MESSAGE_FROM_SYSTEM | 
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                nullptr, id, 
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                (LPTSTR)&buffer, 0, nullptr);

    std::wstring message(buffer, size);
    LocalFree(buffer);
    return message;
}

void LastErrorMessageBox()
{
    std::wstring err = LastErrorMessage();
    MessageBox(nullptr, err.c_str(), L"ERROR", MB_ICONEXCLAMATION | MB_OK);
}

BOOL CALLBACK EnumChildren(HWND hWnd, LPARAM lParam)
{
    ChildHandles.push_back(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND: // menu selections and accelerators
        {
            int wmId = LOWORD(wParam);

            switch (wmId)
            {
                case ID_FILE_NEW:
                {
                    printf("New file\n");
                    break;
                }

                case ID_FILE_LOAD:
                {
                    printf("Load file\n");
                    break;
                }

                case ID_FILE_RELOAD:
                {
                    printf("Reload file\n");
                    break;
                }

                case ID_FILE_CLOSE:
                {
                    printf("Close file\n");
                    break;
                }

                case IDM_EXIT:
                {
                    printf("Exit\n");
                    DestroyWindow(hWnd);
                    break;
                }

                case ID_LAYOUT_LOAD:
                {
                    printf("Load layout\n");
                    break;
                }

                case ID_LAYOUT_SAVE:
                {
                    printf("Save layout\n");
                    break;
                }

                case ID_LAYOUT_RESET:
                {
                    printf("Reset layout\n");
                    break;
                }

                case IDM_ABOUT:
                {
                    HINSTANCE hinstance = (HINSTANCE) GetWindowLong(hWnd, GWLP_HINSTANCE);
                    DialogBox(hinstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                }

                default:
                {
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 2));
            EndPaint(hWnd, &ps);
            break;
        }

        case WM_SIZE:
        {
            ChildHandles.clear();
            if (EnumChildWindows(hWnd, (WNDENUMPROC)&EnumChildren, 0) == 0)
            {
                break;
            }

            // Get handle to SDL_app child window
            HWND hWndChild = nullptr;

            for (HWND handle : ChildHandles)
            {
                TCHAR wndClassName[1024];
                GetClassName(handle, wndClassName, 1024);

                if (std::wstring(wndClassName) == L"SDL_app")
                {
                    hWndChild = handle;
                }
            }

            // Resize child to content inner content of parent
            if (hWndChild != nullptr)
            {
                // Get size of parent
                RECT cr; // client rectangle
                GetClientRect(hWnd, &cr);

                if (cr.left >= 0 && cr.right >= 0 && cr.top >= 0 && cr.bottom >= 0)
                {
                    MoveWindow(hWndChild, cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top, FALSE);
                }
            }

            break;
        }

        case WM_CLOSE:
        {
            ChildHandles.clear();
            if (EnumChildWindows(hWnd, (WNDENUMPROC)&EnumChildren, 0) != 0)
            {
                for (HWND handle : ChildHandles)
                {
                    TCHAR classname[1024];
                    GetClassName(handle, classname, 1024);

                    if (std::wstring(classname) == L"SDL_app")
                    {
                        DestroyWindow(handle);
                    }
                }
            }

            // TODO: send quit message to child
            DestroyWindow(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Allocate new console
    if (AllocConsole() == 0)
    {
        LastErrorMessageBox();
        return 1;
    }

    // Redirect standard output and error to console
    FILE *out = nullptr, *err = nullptr;
    freopen_s(&out, "CONOUT$", "w", stdout);
    freopen_s(&err, "CONOUT$", "w", stderr);

    // Create window class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDC_ICON);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDC_PARENT);
    wc.lpszClassName = WindowClass.c_str();
    wc.hIconSm = LoadIcon(nullptr, IDC_ICON);

    // Register window class
    if (!RegisterClassEx(&wc))
    {
        LastErrorMessageBox();
        return 1;
    }

    // Initialize application
    HWND hWnd = CreateWindow(WindowClass.c_str(), WindowTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        LastErrorMessageBox();
        return 1;
    }

    // Find Child.exe file
    TCHAR buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(L"Child.exe", &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        LastErrorMessageBox();
        return 1;
    }

    FindClose(hFind);

    // Create child process
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);

    TCHAR pidstr[10];
    _stprintf_s(pidstr, L"%d", pid);
    _tprintf(L"%s\n", pidstr);

    TCHAR cmdl[1024];
    _stprintf_s(cmdl, L"%ls %ls", FindFileData.cFileName, pidstr);

    BOOL success = CreateProcess(FindFileData.cFileName,
                                 cmdl,    // command line
                                 NULL,      // process security attributes
                                 NULL,      // primary thread security attributes
                                 TRUE,      // handles are inherited
                                 0,         // creation flags
                                 NULL,      // use parent's environment
                                 NULL,      // use parent's current directory
                                 &si,       // STARTUPINFO
                                 &pi);      // PROCESS_INFORMATION

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Get table of accelerators
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PARENT));

    // Message pump
    MSG msg; ZeroMemory(&msg, sizeof(MSG));

    while (GetMessage(&msg, nullptr, 0, 0) != WM_QUIT)
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            return (INT_PTR)TRUE;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    }

    return (INT_PTR)FALSE;
}
