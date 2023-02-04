#include <shlwapi.h>
#include <windows.h>
#undef near // NOTE: lol
#undef far  // NOTE: lol

#include "p_platform_common.h"

#include "p_renderer_opengl.h"
#include "p_renderer_opengl.cpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "p_win32_main.h"

inline bool
is_fullscreen(Win32Window *window) {
    GLFWmonitor *monitor = glfwGetWindowMonitor(window->handle);
    bool result = monitor != nullptr;
    return result;
}

#define close_window(window) glfwSetWindowShouldClose((window)->handle, GLFW_TRUE)
void glfw_window_close_callback(GLFWwindow *window) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    UserPointer *data = (UserPointer *)glfwGetWindowUserPointer(window);
    if(data) {
        data->input->scroll_move = (i32)yoffset;
    }
}

void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    UserPointer *data = (UserPointer *)glfwGetWindowUserPointer(window);
    if(data) {
        data->window->state.width  = width;
        data->window->state.height = height;
        
        if(!is_fullscreen(data->window)) {
            data->window->x_windowed_size = width;
            data->window->y_windowed_size = height;
        }
        
        if(data->game && data->game->size_callback) {
            data->game->size_callback(data->core, width, height);
        }
    }
}

void glfw_window_pos_callback(GLFWwindow *window, int xpos, int ypos) {
    UserPointer *data = (UserPointer *)glfwGetWindowUserPointer(window);
    if(data) {
        if(!is_fullscreen(data->window)) {
            data->window->x_windowed_p = xpos;
            data->window->y_windowed_p = ypos;
        }
    }
}

void glfw_window_focus_callback(GLFWwindow *window, int focused) {
    UserPointer *data = (UserPointer *)glfwGetWindowUserPointer(window);
    if(data) {
        data->window->state.is_focused = (focused == GLFW_TRUE);
    }
}

// NOTE: set working directory to .exe location
static void set_working_directiory(void) { 
    char exe_dir[256];
    GetModuleFileNameA(0, exe_dir, size_array(exe_dir));
    PathRemoveFileSpecA(exe_dir);
    SetCurrentDirectory(exe_dir);
}

inline bool
file_exists(char *file_path) {
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if(GetFileAttributesExA(file_path, GetFileExInfoStandard, &ignored)) {
        return true;
    }
    return false;
}

inline bool
delete_file(char *file_path) {
#if 0
    char path[256];
    GetFullPathNameA(file_path, size_array(path), path, 0);
#else
    char *path = file_path;
#endif
    if(DeleteFileA(path)) {
        return true;
    }
    return false;
}

ALLOC_MEMORY_PROC(alloc_memory) {
    if(bytes == 0) { return nullptr; }
    void *mem = malloc(bytes);
    return mem;
}

REALLOC_MEMORY_PROC(realloc_memory) {
    void *mem = realloc(*mem_ptr, bytes);
    if(mem) {
        *mem_ptr = mem;
        return true;
    }
    return false;
}

FREE_MEMORY_PROC(free_memory) {
    free(mem);
}

READ_FILE_PROC(read_file) {
    FILE *_file;
    if(fopen_s(&_file, path, "rb") == 0) {
        fseek(_file, 0, SEEK_END);
        file->size = ftell(_file);
        fseek(_file, 0, SEEK_SET);
        if(null_terminated) {
            file->contents = malloc(file->size + 1);
            fread(file->contents, 1, file->size, _file);
            ((char *)file->contents)[file->size] = '\0';
        }
        else {
            file->contents = malloc(file->size);
            fread(file->contents, 1, file->size, _file);
        }
        fclose(_file);
        return true;
    }
    else {
        file->size = 0;
        file->contents = 0;
    }
    return false;
}

FREE_FILE_PROC(free_file) {
    free(file->contents);
    file->size = 0;
    file->contents = nullptr;
}

inline PTime
win32_filetime_to_p_time(FILETIME filetime) {
    PTime time = {};
    SYSTEMTIME systemtime;
    if(FileTimeToSystemTime(&filetime, &systemtime)) {
        time.year = systemtime.wYear;
        time.month = systemtime.wMonth;
        time.day = systemtime.wDay;
        time.hour = systemtime.wHour;
        time.minute = systemtime.wMinute;
        time.second = systemtime.wSecond;
        time.milliseconds = systemtime.wMilliseconds;
    }
    return time;
}

LAST_WRITE_TIME_PROC(last_write_time) {
    PTime last_write_time = {};
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle = FindFirstFileA(path, &find_data);
    if(find_handle != INVALID_HANDLE_VALUE) {
        last_write_time = win32_filetime_to_p_time(find_data.ftLastWriteTime);
        FindClose(find_handle);
    }
    return last_write_time;
}

CREATE_SOUND_PROC(create_sound) {
    if(!sound->data || !sound->data_size) {
        return 0;
    }
    
    ALuint buffer = 0;
    alGenBuffers(1, &buffer);
    
    ALenum format = 0;
    if(sound->channels == 1 && sound->bits_per_sample == 8) {
        format = AL_FORMAT_MONO8;
    }
    else if(sound->channels == 1 && sound->bits_per_sample == 16) {
        format = AL_FORMAT_MONO16;
    }
    /*
    else if(sound->channels == 1 && sound->bits_per_sample == 32) {
        format = AL_FORMAT_MONO_FLOAT32;
    }
*/
    else if(sound->channels == 2 && sound->bits_per_sample == 8) {
        format = AL_FORMAT_STEREO8;
    }
    else if(sound->channels == 2 && sound->bits_per_sample == 16) {
        format = AL_FORMAT_STEREO16;
    }
    /*
        else if(sound->channels == 2 && sound->bits_per_sample == 32) {
            // format = AL_FORMAT_STEREO_FLOAT32;
            format = AL_STEREO32F_SOFT;
        }
    */
    else {
        ASSERT(false, "unrecognised wave format");
    }
    
    alBufferData(buffer, format, (void *)sound->data, sound->data_size, sound->sample_rate);
    
    sound_id id = buffer;
    return id;
}

DELETE_SOUND_PROC(delete_sound) {
    if(id) {
        alDeleteBuffers(1, &id);
    }
}

PLAY_SOUND_PROC(play_sound) {
    if(!id) {
        return;
    }
    
    ALuint source = 0;
    alGenSources(1, &source);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, id);
    alSourcePlay(source);
    // alDeleteSources(1, &source);
}

static void
unload_game_dll(GameDLL *dll) {
    if(!dll->loaded) {
        return;
    }
    
    if(dll->module) {
        FreeLibrary(dll->module);
        dll->module = 0;
        dll->loaded = false;
    }
}

static GameDLL
load_game_dll(void) {
    GameDLL dll = {};
    if(!dll_lock_file || !file_exists((char *)dll_lock_file)) {
        if(CopyFileA(dll_path, dll_temp_path, FALSE)) {
            dll.last_write_time = last_write_time(dll_path);
            dll.module = LoadLibraryA(dll_temp_path);
            if(dll.module) {
                dll.game_init = (game_init_proc *)
                    GetProcAddress(dll.module, "game_init");
                dll.game_frame = (game_frame_proc *)
                    GetProcAddress(dll.module, "game_frame");
                dll.size_callback = (game_size_callback_proc *)
                    GetProcAddress(dll.module, "size_callback");
                dll.hotload_callback = (game_hotload_callback_proc *)
                    GetProcAddress(dll.module, "hotload_callback");
                dll.get_startup_params = (game_get_startup_params_proc *)
                    GetProcAddress(dll.module, "get_startup_params");
            }
            dll.loaded = dll.game_init && dll.game_frame && dll.size_callback
                && dll.hotload_callback && dll.get_startup_params;
        }
        else {
            dll.loaded = false;
        }
    }
    return dll;
}

static void
hotload_game_dll(GameDLL *dll, Core *core) {
    PTime game_dll_last_write_time = last_write_time(dll_path);
    if(!p_time_cmp(&game_dll_last_write_time, &dll->last_write_time)) {
        unload_game_dll(dll);
        *dll = load_game_dll();
        if(dll->hotload_callback) {
            dll->hotload_callback(core);
        }
    }
}

static void
init_input(UserPointer *user_pointer) {
    Win32Window *window = user_pointer->window;
    glfwSetWindowUserPointer(window->handle, (void *)user_pointer);
}

static void
get_input(UserPointer *user_pointer) {
    Win32Window *window = user_pointer->window;
    Input *input        = user_pointer->input;
    
    // NOTE: Keyboard
    bool kb_input = false;
    for(u32 i = 0; i < KB_KEY_COUNT; ++i) {
        ButtonState key = get_kb_key_state(input, (kb_key)i);
        bool key_down = glfwGetKey(window->handle, glfw_key_codes[i]) == GLFW_PRESS;
        if(update_button_state(key, key_down)) {
            kb_input = true;
        }
        
    }
    input->kb_input = kb_input;
    
    // NOTE: Mouse
    bool mouse_input = false;
    for(u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
        ButtonState button = get_mouse_button_state(input, (mouse_button)i);
        bool button_down = glfwGetMouseButton(window->handle, glfw_mouse[i]) == GLFW_PRESS;
        if(update_button_state(button, button_down)) {
            mouse_input = true;
        }
    }
    input->mouse_input = mouse_input;
    
    f64 x_mouse_new;
    f64 y_mouse_new;
    glfwGetCursorPos(window->handle, &x_mouse_new, &y_mouse_new);
    
    input->mouse_last.x = input->mouse.x;
    input->mouse_last.y = input->mouse.y;
    input->mouse.x = (f32)x_mouse_new;
    input->mouse.y = (f32)((f32)window->state.height - y_mouse_new);
    input->mouse_moved = (input->mouse_last.x != input->mouse.x ||
                          input->mouse_last.y != input->mouse.y);
    
    // NOTE: time
    ++input->frames_elapsed;
    f32 time = (f32)glfwGetTime();
    input->delta_time = time - input->time_elapsed;
    if(window->state.cap_framerate) {
        f32 framerate_cap_ms = 1.0f / window->state.desired_fps;
        while(input->delta_time < framerate_cap_ms) {
            time = (f32)glfwGetTime();
            input->delta_time = time - input->time_elapsed;
        }
    }
    input->time_elapsed = time;
    
    // NOTE: callbacks
    input->scroll_move = 0;
}

static void
set_fullscreen(Win32Window *window, i32 x_fullscreen, i32 y_fullscreen) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if(monitor) {
        GLFWvidmode *mode = (GLFWvidmode *)glfwGetVideoMode(monitor);
        
        i32 x_fullscreen_res = x_fullscreen;
        i32 y_fullscreen_res = y_fullscreen;
        if(x_fullscreen_res == 0) {
            x_fullscreen_res = mode->width;
        }
        if(y_fullscreen_res == 0) {
            y_fullscreen_res = mode->height;
        }
        
        glfwSetWindowMonitor(window->handle, monitor, 0, 0, x_fullscreen_res, y_fullscreen_res, mode->refreshRate);
        window->state.fullscreen = true;
    }
}

static void
set_windowed(Win32Window *window) {
    glfwSetWindowMonitor(window->handle, 0, window->x_windowed_p, window->y_windowed_p,
                         window->x_windowed_size, window->y_windowed_size, 0);
    window->state.fullscreen = false;
}

static void
set_window_size(Win32Window *window, i32 window_width, i32 window_height) {
    glfwSetWindowSize(window->handle, window_width, window_height);
    window->x_windowed_size = window_width;
    window->y_windowed_size = window_height;
}

static Win32Window
create_window(WindowCreationParams *params) {
    Win32Window window = {};
    
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,    4); 
    glfwWindowHint(GLFW_RED_BITS,   8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS,  8);
    glfwWindowHint(GLFW_RESIZABLE, params->resizable ? GLFW_TRUE : GLFW_FALSE);
    
    GLFWwindow *handle = glfwCreateWindow(params->window_width, params->window_height, params->title, 0, 0);
    if(handle) {
        window.handle = handle;
        window.win32_handle = glfwGetWin32Window(handle);
        
        i32 width, height;
        glfwGetWindowSize(handle, &width, &height);
        window.state.width = width;
        window.state.height = height;
        window.state.desired_fps = params->desired_fps;
        window.state.cap_framerate = params->cap_framerate;
        copy_memory(params->title, window.state.title, size_array(WindowCreationParams::title));
        window.state.resizable = params->resizable;
        window.state.is_focused = true;
        window.state.fullscreen = false;
        window.state.x_fullscreen = params->x_fullscreen;
        window.state.y_fullscreen = params->y_fullscreen;
        window.state.window_width = params->window_width;
        window.state.window_height = params->window_height;
        
        glfwSetWindowCloseCallback(handle, glfw_window_close_callback);
        glfwSetScrollCallback(handle, glfw_scroll_callback);
        glfwSetWindowSizeCallback(handle, glfw_window_size_callback);
        glfwSetWindowFocusCallback(handle, glfw_window_focus_callback);
        glfwSetWindowPosCallback(handle, glfw_window_pos_callback);
        // glfwSetKeyCallback(handle, glfw_key_callback);
        glfwMakeContextCurrent(handle);
        glfwSwapInterval(false);
        
        glfwGetWindowPos(window.handle, &window.x_windowed_p, &window.y_windowed_p);
        glfwGetWindowSize(window.handle, &window.x_windowed_size, &window.y_windowed_size);
        
#if 0 // NOTE: keep window on top
        RECT win32_rect;
        GetWindowRect(window.win32_handle, &win32_rect);
        SetWindowPos(window.win32_handle, HWND_TOPMOST, win32_rect.left, win32_rect.top,
                     win32_rect.right - win32_rect.left, win32_rect.bottom - win32_rect.top, 0);
#endif
        window.valid = true;
    }
    return window;
}

static GameMemory 
get_memory(MemoryAllocationParams *params) {
    GameMemory memory = {};
    memory.permanent_memory.size = params->permanent_memory_size;
    memory.permanent_memory.ptr = (byte *)malloc(params->permanent_memory_size);
    memory.temporary_memory.size = params->temporary_memory_size;
    memory.temporary_memory.ptr = (byte *)malloc(params->temporary_memory_size);
    if(memory.permanent_memory.ptr && memory.temporary_memory.ptr) {
        memory.allocated = true;
        zero_memory(memory.permanent_memory.ptr, memory.permanent_memory.size);
        zero_memory(memory.temporary_memory.ptr, memory.temporary_memory.size);
    }
    return memory;
}

int main(int, char**) {
    set_working_directiory();
    
    PlatformProcs procs = {
        (alloc_memory_proc *)alloc_memory,
        (realloc_memory_proc *)realloc_memory,
        (free_memory_proc *)free_memory,
        (read_file_proc *)read_file,
        (free_file_proc *)free_file,
        (last_write_time_proc *)last_write_time,
        (create_sound_proc *)create_sound,
        (delete_sound_proc *)delete_sound,
        (play_sound_proc *)play_sound,
    };
    
    GameDLL game_dll = load_game_dll();
    if(!game_dll.loaded) {
        message_box("win32", "couldn't load the game dll...\n");
        return -1;
    }
    StartupParams startup = game_dll.get_startup_params(&procs);
    
    GameMemory memory = get_memory(&startup.memory);
    if(!memory.allocated) {
        message_box("win32", "couldn't allocate memory...\n");
        return -1;
    }
    
    if(!glfwInit()) {
        message_box("win32", "couldn't initialize glfw...\n");
        return -1;
    }
    
    RendererAPI api = {};
    get_renderer_api(&api);
    ASSERT(is_renderer_api_valid(&api), "invalid renderer api\nprobably missing procs...\n");
    
    Win32Window window = create_window(&startup.window);
    if(!window.valid) {
        message_box("win32", "couldn't create window...\n");
        return -1;
    }
    
    if(glewInit() != GLEW_OK) {
        message_box("win32", "couldn't initialize glew...\n");
        return -1;
    }
    
    Renderer renderer  = {};
    init_renderer(&renderer, &api, &procs);
    
    Core core = {
        &renderer,
        &memory,
        &window.state,
        &procs
    };
    
    Input input = {};
    UserPointer user_pointer = {
        &window,
        &input,
        &core,
        &game_dll
    };
    init_input(&user_pointer);
    
    if(startup.window.fullscreen) {
        set_fullscreen(&window, startup.window.x_fullscreen, startup.window.y_fullscreen);
    }
    
    ALCdevice *al_device = alcOpenDevice(nullptr);
    if(!al_device) {
        // NOTE: error
        ASSERT(false, "");
    }
    
    ALCcontext *al_context = alcCreateContext(al_device, nullptr);
    if(!al_context) {
        // NOTE: error
        ASSERT(false, "");
    }
    
    bool current = alcMakeContextCurrent(al_context);
    if(!current) {
        // NOTE: error
        ASSERT(false, "");
    }
    
    ALfloat listener_ori[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, listener_ori);

    bool initialized = game_dll.game_init(&core);
    if(!initialized) {
        message_box("win32", "couldn't initialize the game...\n");
        return -1;
    }
    
    game_dll.size_callback(&core, window.state.width, window.state.height);
    while(!glfwWindowShouldClose(window.handle)) {
        WindowState last_state = window.state;
        
        hotload_game_dll(&game_dll, &core);
        get_input(&user_pointer);
        glfwPollEvents();
        
        begin_renderer_frame(&renderer);
        hotload_shaders(&renderer, input.delta_time);
        
        if(input.delta_time < (1.0f / 10.0f)) {
            if(game_dll.game_frame) {
                bool do_not_quit = game_dll.game_frame(&core, &input);
                if(!do_not_quit) {
                    close_window(&window);
                }
            }
            glfwSwapBuffers(window.handle);
        }
        
        if(window.state.fullscreen != last_state.fullscreen) {
            if(window.state.fullscreen) {
                set_fullscreen(&window, window.state.x_fullscreen, window.state.y_fullscreen);
            }
            else {
                set_windowed(&window);
            }
        }
        else if(((window.state.x_fullscreen != last_state.x_fullscreen) ||
                 (window.state.y_fullscreen != last_state.y_fullscreen))) {
            if(window.state.fullscreen) {
                set_windowed(&window);
                set_fullscreen(&window, window.state.x_fullscreen, window.state.y_fullscreen);
            }
        }
        else if(((window.state.window_width != last_state.window_width) ||
                 (window.state.window_height != last_state.window_height))) {
            if(!window.state.fullscreen) {
                set_window_size(&window, window.state.window_width, window.state.window_height);
            }
            else {
                window.x_windowed_size = window.state.window_width;
                window.y_windowed_size = window.state.window_height;
            }
        }
    }
    
    unload_game_dll(&game_dll);
    if(file_exists((char *)dll_temp_path)) {
        delete_file((char *)dll_temp_path);
    }
    
    return 0; 
}