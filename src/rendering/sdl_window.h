#ifndef SDL_WINDOW_H
#define SDL_WINDOW_H
#include <SDL.h>
#include <SDL_vulkan.h>
#include "../util/typedef.h"

namespace SDL
{
    void create_window();
    void destroy_window();

    //vulkan stuff
    u32 get_vulkan_instance_extension_count();
    void get_vulkan_instance_extension_names(u32 count, const char** names);

    void create_vulkan_surface(VkSurfaceKHR *surface);

    void set_fullscreen(bool s);
}

#endif // SDL_WINDOW_H
