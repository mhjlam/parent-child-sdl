#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <string>
#include <sstream>

#include <SDL.h>
#include <SDL_opengl.h>

static bool running = true;
static SDL_Window* window = nullptr;
static SDL_GLContext context;

static const std::wstring WindowTitle = L"Child";
static const std::wstring WindowClass = L"Child";

std::wstring GetLastErrorString()
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
    std::wstring err = GetLastErrorString();
    MessageBox(nullptr, err.c_str(), L"ERROR", MB_ICONEXCLAMATION | MB_OK);
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

    DWORD ppid = 0;

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

    if (argv == NULL || argc == 0)
    {
        MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
        return 1;
    }

    ppid = wcstol(argv[0], '\0', 10);
    wprintf(L"%d\n", ppid);
    LocalFree(argv);

    if (ppid == 0)
    {
        const char* err = "Unable to find parent PID";
        MessageBoxA(nullptr, err, "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    // Find parent window handle
    HWND hwndParent = FindWindowEx(NULL, NULL, L"Parent", NULL);
    DWORD pid; GetWindowThreadProcessId(hwndParent, &pid);

    if (pid != ppid)
    {
        std::stringstream ss;
        ss << "Unable to find parent window with PID " << ppid;
        MessageBoxA(nullptr, ss.str().c_str(), "ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        const char* err = SDL_GetError();
        MessageBoxA(nullptr, err, "ERROR", MB_ICONEXCLAMATION | MB_OK);
        SDL_Log("Failed to initialize SDL: %s", err);
        return 1;
    }

    // Create window class for child
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDC_ICON);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WindowClass.c_str();

    // Register window class
    if (!RegisterClass(&wc))
    {
        LastErrorMessageBox();
        return 1;
    }

    // Initialize application
    HWND hwndChild = CreateWindow(WindowClass.c_str(), WindowTitle.c_str(), WS_CHILD, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hwndParent, nullptr, hInstance, nullptr);

    if (!hwndChild)
    {
        LastErrorMessageBox();
        return 1;
    }

    // Get window handle and pass to SDL to create window
    window = SDL_CreateWindowFrom(hwndChild);
    
    //window = SDL_CreateWindow("Child", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 576, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        SDL_GL_MakeCurrent(window, context);

        glClearColor(0.2f, 0.4f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);

        SDL_Delay(1);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
