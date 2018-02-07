#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlstr.h> // CString
#include <stdlib.h> // rand()

#include <SDL.h>
#include <SDL_opengl.h>

static bool running = true;
static SDL_Window* window = nullptr;
static SDL_GLContext context;

void render()
{
    SDL_GL_MakeCurrent(window, context);

    static float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    glClearColor(r, 0.4f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
}

int main(int argc, char* arv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        const char* err = SDL_GetError();

        MessageBox(nullptr, CString(err), CString("ERROR"), MB_ICONEXCLAMATION | MB_OK);
        SDL_Log("Failed to initialize SDL: %s", err);
        return 1;
    }

    // Get window handle and pass to SDL to create window
    //SDL_CreateWindowFrom();
    
    window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 576, SDL_WINDOW_OPENGL);
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

        render();
        SDL_Delay(1);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
