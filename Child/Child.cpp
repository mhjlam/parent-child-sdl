#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <string>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>

static bool running = true;
static SDL_Window* window = nullptr;
static SDL_GLContext context;
static SDL_SysWMinfo sysinfo;

std::string LastErrorMessage()
{
    DWORD id = GetLastError();
    if (id == 0) return std::string();

    LPSTR buffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                                 nullptr, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, nullptr);

    std::string message(buffer, size);
    LocalFree(buffer);
    return message;
}

int main(int argc, char* argv[])
{
    long parentPID = 0;

    if (argc > 1)
    {
        parentPID = atol(argv[1]);
    }

    if (parentPID == 0)
    {
        SDL_Log("Unable to find parent PID");
        return 1;
    }

    // Find parent window handle
    HWND hwndParent = FindWindow(L"Parent", NULL);

    if (hwndParent == NULL)
    {
        SDL_Log("%s", LastErrorMessage());
        return 1;
    }

    // Find parent process id
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwndParent, &lpdwProcessId);

    if (lpdwProcessId != parentPID)
    {
        SDL_Log("Unable to find parent window with PID %l", parentPID);
        return 1;
    }

    // Get parent client rect
    RECT pcr;
    if (GetWindowRect(hwndParent, &pcr) == 0)
    {
        SDL_Log("%s", LastErrorMessage());
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        const char* err = SDL_GetError();
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create SDL window and attach OpenGL context
    window = SDL_CreateWindow("Child", 0, 0, pcr.right - pcr.left, pcr.bottom - pcr.top, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
    context = SDL_GL_CreateContext(window);

    // Get SDL window handle
    SDL_VERSION(&sysinfo.version);

    HWND hwndChild = nullptr;
    if (SDL_GetWindowWMInfo(window, &sysinfo))
    {
        if (sysinfo.subsystem == SDL_SYSWM_WINDOWS)
        {
            hwndChild = sysinfo.info.win.window;
        }
    }

    if (!hwndChild)
    {
        SDL_Log("Unable to retrieve handle for created SDL window");
        return 1;
    }

    // Set SDL window as child to native parent window
    if (!SetParent(hwndChild, hwndParent))
    {
        SDL_Log("%s", LastErrorMessage());
        return 1;
    }

    // Set up OpenGL context for rendering
    SDL_GL_MakeCurrent(window, context);

    GLfloat color[4] = { 0.2f, 0.4f, 0.1f, 1.0f };

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                GLfloat red = color[0];
                color[0] = color[1];
                color[1] = red;
            }
        }

        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
