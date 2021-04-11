#include "sdl_window.h"
#include "rendering_util.h"
#include "vulkan.h"

namespace SDL
{
    SDL_Window* window;
}

void SDL::create_window()
{
    window = SDL_CreateWindow("Nekro Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
}
void SDL::destroy_window()
{
    SDL_DestroyWindow(window);
}

//vulkan stuff
u32 SDL::get_vulkan_instance_extension_count()
{
    u32 result;
    SDL_Vulkan_GetInstanceExtensions(window, &result, nullptr);
    return result;
}
void SDL::get_vulkan_instance_extension_names(u32 count, const char** names)
{
    SDL_Vulkan_GetInstanceExtensions(window, &count, names);
}

void SDL::create_vulkan_surface(VkSurfaceKHR *surface)
{
    VkInstance instance = Vulkan::get_instance();
    SDL_Vulkan_CreateSurface(window, instance, surface);
}

void SDL::set_fullscreen(bool s)
{
    if (s)
    {
        SDL_ShowCursor(false);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    else
    {
        SDL_ShowCursor(true);
        SDL_SetWindowFullscreen(window, 0);
    }
}
