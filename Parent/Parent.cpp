#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <pathcch.h>
#include <tchar.h>

#include <string>


// dialogs
#define IDD_ABOUTBOX        1001

// menu items
#define IDM_FILE_NEW        2002
#define IDM_FILE_LOAD       2003
#define IDM_FILE_RELOAD     2004
#define IDM_FILE_CLOSE      2005
#define IDM_FILE_EXIT       2006
#define IDM_LAYOUT_LOAD     2011
#define IDM_LAYOUT_SAVE     2012
#define IDM_LAYOUT_RESET    2013
#define IDM_HELP_ABOUT      2021

typedef std::basic_string<TCHAR> TSTR;

static HWND ChildHwnd = NULL;
static const TSTR ChildWindowClass = _T("Child");
static const TSTR ParentWindowClass = _T("Parent");

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

TSTR LastErrorMessage()
{
    DWORD id = GetLastError();
    if (id == 0) return TSTR();

    LPTSTR buffer = NULL;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0, NULL);

    TSTR message(buffer, size);
    LocalFree(buffer);
    return message;
}

void LastErrorMessageBox()
{
    MessageBox(NULL, LastErrorMessage().c_str(), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
}

BOOL CALLBACK GetChildHWND(HWND hWnd, LPARAM lParam)
{
    ChildHwnd = NULL;
    TCHAR wndClassName[1024];
    GetClassName(hWnd, wndClassName, 1024);

    if (_tcscmp(wndClassName, _T("SDL_app")) == 0)
    {
        ChildHwnd = hWnd;
        return FALSE; // stop enumerating
    }

    return TRUE;
}

void CreateMenuBar(HWND hWnd)
{
    HMENU hMenu = CreateMenu();

    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_FILE_NEW, _T("&New"));
    AppendMenu(hSubMenu, MF_STRING, IDM_FILE_LOAD, _T("&Open"));
    AppendMenu(hSubMenu, MF_STRING, IDM_FILE_RELOAD, _T("&Reload"));
    AppendMenu(hSubMenu, MF_STRING, IDM_FILE_CLOSE, _T("&Close"));
    AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hSubMenu, MF_STRING, IDM_FILE_EXIT, _T("E&xit"));
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, _T("&File"));

    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_LAYOUT_LOAD, _T("&Load"));
    AppendMenu(hSubMenu, MF_STRING, IDM_LAYOUT_SAVE, _T("&Save"));
    AppendMenu(hSubMenu, MF_STRING, IDM_LAYOUT_RESET, _T("&Reset"));
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, _T("&Layout"));

    hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_HELP_ABOUT, _T("&Help"));
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, _T("&About"));

    SetMenu(hWnd, hMenu);
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
                case IDM_FILE_NEW:
                {
                    printf("New file\n");
                    break;
                }

                case IDM_FILE_LOAD:
                {
                    printf("Load file\n");
                    break;
                }

                case IDM_FILE_RELOAD:
                {
                    printf("Reload file\n");
                    break;
                }

                case IDM_FILE_CLOSE:
                {
                    printf("Close file\n");
                    break;
                }

                case IDM_FILE_EXIT:
                {
                    printf("Exit\n");
                    DestroyWindow(hWnd);
                    break;
                }

                case IDM_LAYOUT_LOAD:
                {
                    printf("Load layout\n");
                    break;
                }

                case IDM_LAYOUT_SAVE:
                {
                    printf("Save layout\n");
                    break;
                }

                case IDM_LAYOUT_RESET:
                {
                    printf("Reset layout\n");
                    break;
                }

                case IDM_HELP_ABOUT:
                {
                    HINSTANCE hinstance = (HINSTANCE)(LONG_PTR)GetWindowLong(hWnd, GWLP_HINSTANCE);
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
            EnumChildWindows(hWnd, (WNDENUMPROC)&GetChildHWND, 0);

            // Resize child to content inner content of parent
            if (ChildHwnd != NULL)
            {
                // Get size of parent
                RECT cr; // client rectangle
                GetClientRect(hWnd, &cr);

                if (cr.left >= 0 && cr.right >= 0 && cr.top >= 0 && cr.bottom >= 0)
                {
                    MoveWindow(ChildHwnd, cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top, FALSE);
                }
            }

            break;
        }

        case WM_CLOSE:
        {
            EnumChildWindows(hWnd, (WNDENUMPROC)&GetChildHWND, 0);

            if (ChildHwnd != NULL)
            {
                // TODO: send quit message to child
                DestroyWindow(hWnd);
            }

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
    FILE *out = NULL, *err = NULL;
    freopen_s(&out, "CONOUT$", "w", stdout);
    freopen_s(&err, "CONOUT$", "w", stderr);

    // Create window class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDC_ICON);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = ParentWindowClass.c_str();
    wcex.hIconSm = LoadIcon(NULL, IDC_ICON);

    // Register window class
    if (!RegisterClassEx(&wcex))
    {
        LastErrorMessageBox();
        return 1;
    }

    // Initialize application
    HWND hWnd = CreateWindow(ParentWindowClass.c_str(), ParentWindowClass.c_str(), 
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
                             NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        LastErrorMessageBox();
        return 1;
    }

    // Try to find Child.exe file in current working directory
    TCHAR lpParentExecutablePath[MAX_PATH];
    GetModuleFileName(NULL, lpParentExecutablePath, MAX_PATH);

    if (PathCchRemoveFileSpec(lpParentExecutablePath, MAX_PATH) != S_OK)
    {
        LastErrorMessageBox();
        return 1;
    }

    TCHAR lpChildPath[MAX_PATH];
    _stprintf_s(lpChildPath, _T("%ls\\%ls.exe"), lpParentExecutablePath, ChildWindowClass.c_str());

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(lpChildPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        LastErrorMessageBox();
        return 1;
    }

    FindClose(hFind);

    // Set child process startup and process info
    STARTUPINFO lpStartupInfo;
    ZeroMemory(&lpStartupInfo, sizeof(STARTUPINFO));
    lpStartupInfo.cb = sizeof(lpStartupInfo);

    PROCESS_INFORMATION lpProcessInformation;
    ZeroMemory(&lpProcessInformation, sizeof(PROCESS_INFORMATION));

    // Get parent process id
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hWnd, &lpdwProcessId);

    TCHAR lpProcessId[10];
    _stprintf_s(lpProcessId, _T("%d"), lpdwProcessId);
    _tprintf(_T("%s\n"), lpProcessId);

    TCHAR lpCommandLine[1024];
    _stprintf_s(lpCommandLine, _T("%ls %ls"), lpChildPath, lpProcessId);

    // Create process
    BOOL success = CreateProcess(lpChildPath, lpCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &lpStartupInfo, &lpProcessInformation);

    // Create menu
    CreateMenuBar(hWnd);

    // Show window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message pump
    MSG msg; ZeroMemory(&msg, sizeof(MSG));

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
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
