#ifndef P_WIN32_MAIN_H
#define P_WIN32_MAIN_H

#define win32_log(str, ...) printf("win32: "##str, __VA_ARGS__)

#define message_box(caption, message, ...) {\
char buffer[256];\
sprintf_s(buffer, size_array(buffer), message, __VA_ARGS__);\
MessageBoxA(0, buffer, caption, 0x00000000L);\
}

#define set_window_title(window, buffer, ...) {\
char title[128];\
sprintf_s(title, size_array(title), buffer, __VA_ARGS__);\
glfwSetWindowTitle(window.handle, title);\
}

// NOTE: p_platform_common.h -> kb_key
const static i32 glfw_key_codes[KB_KEY_COUNT] = {
    GLFW_KEY_UP,
    GLFW_KEY_DOWN,
    GLFW_KEY_LEFT,
    GLFW_KEY_RIGHT,
    
    GLFW_KEY_ESCAPE, GLFW_KEY_SPACE, 
    GLFW_KEY_ENTER,
    
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    
    GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_HOME,
    
    GLFW_KEY_MINUS, GLFW_KEY_EQUAL,
    
    GLFW_KEY_F1,  GLFW_KEY_F2,  GLFW_KEY_F3,
    GLFW_KEY_F4,  GLFW_KEY_F5,  GLFW_KEY_F6,
    GLFW_KEY_F7,  GLFW_KEY_F8,  GLFW_KEY_F9,
    GLFW_KEY_F10, GLFW_KEY_F11, GLFW_KEY_F12,
    
    GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, GLFW_KEY_R,
    GLFW_KEY_T, GLFW_KEY_Y, GLFW_KEY_U, GLFW_KEY_I,
    GLFW_KEY_O, GLFW_KEY_P, GLFW_KEY_A, GLFW_KEY_S,
    GLFW_KEY_D, GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H,
    GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_Z,
    GLFW_KEY_X, GLFW_KEY_C, GLFW_KEY_V, GLFW_KEY_B,
    GLFW_KEY_N, GLFW_KEY_M,
};

inline kb_key get_kb_key_from_glfw_key_code(i32 glfw_key_code) {
    switch(glfw_key_code) {
        case GLFW_KEY_UP:           return KB_up;
        case GLFW_KEY_DOWN:         return KB_down;
        case GLFW_KEY_LEFT:         return KB_left;
        case GLFW_KEY_RIGHT:        return KB_right;
        case GLFW_KEY_ESCAPE:       return KB_esc;
        case GLFW_KEY_SPACE:        return KB_space;
        case GLFW_KEY_ENTER:        return KB_enter;
        case GLFW_KEY_LEFT_SHIFT:   return KB_left_shift;
        case GLFW_KEY_LEFT_CONTROL: return KB_left_ctrl;
        case GLFW_KEY_PAGE_UP:      return KB_page_up;
        case GLFW_KEY_PAGE_DOWN:    return KB_page_down;
        case GLFW_KEY_MINUS:        return KB_minus;
        case GLFW_KEY_EQUAL:        return KB_equal;
        case GLFW_KEY_F1:  return KB_f01;
        case GLFW_KEY_F2:  return KB_f02;
        case GLFW_KEY_F3:  return KB_f03;
        case GLFW_KEY_F4:  return KB_f04;
        case GLFW_KEY_F5:  return KB_f05;
        case GLFW_KEY_F6:  return KB_f06;
        case GLFW_KEY_F7:  return KB_f07;
        case GLFW_KEY_F8:  return KB_f08;
        case GLFW_KEY_F9:  return KB_f09;
        case GLFW_KEY_F10: return KB_f10;
        case GLFW_KEY_F11: return KB_f11;
        case GLFW_KEY_F12: return KB_f12;
        case GLFW_KEY_Q: return KB_q;
        case GLFW_KEY_W: return KB_w;
        case GLFW_KEY_E: return KB_e;
        case GLFW_KEY_R: return KB_r;
        case GLFW_KEY_T: return KB_t;
        case GLFW_KEY_Y: return KB_y;
        case GLFW_KEY_U: return KB_u;
        case GLFW_KEY_I: return KB_i;
        case GLFW_KEY_O: return KB_o;
        case GLFW_KEY_P: return KB_p;
        case GLFW_KEY_A: return KB_a;
        case GLFW_KEY_S: return KB_s;
        case GLFW_KEY_D: return KB_d;
        case GLFW_KEY_F: return KB_f;
        case GLFW_KEY_G: return KB_g;
        case GLFW_KEY_H: return KB_h;
        case GLFW_KEY_J: return KB_j;
        case GLFW_KEY_K: return KB_k;
        case GLFW_KEY_L: return KB_l;
        case GLFW_KEY_Z: return KB_z;
        case GLFW_KEY_X: return KB_x;
        case GLFW_KEY_C: return KB_c;
        case GLFW_KEY_V: return KB_v;
        case GLFW_KEY_B: return KB_b;
        case GLFW_KEY_N: return KB_n;
        case GLFW_KEY_M: return KB_m;
        default: return ((kb_key)-1);
    };
};

// NOTE: p_platform_common.h -> kb_key
const static u32 glfw_mouse[MOUSE_BUTTON_COUNT] = {
    GLFW_MOUSE_BUTTON_LEFT,
    GLFW_MOUSE_BUTTON_RIGHT,
};

struct Win32Window {
    bool valid;
    HWND win32_handle;
    GLFWwindow *handle;
    WindowState state;
    bool user_pointer_set;
    
    i32 x_windowed_p;
    i32 y_windowed_p;
    i32 x_windowed_size; 
    i32 y_windowed_size;
};

struct UserPointer {
    Win32Window    *window;
    Input          *input;
    Core           *core;
    struct GameDLL *game;
};

const char *dll_path      = "p_game.dll";
const char *dll_temp_path = "p_game_temp.dll";
const char *dll_lock_file = "p_game_dll.lock";

struct GameDLL {
    b32 loaded;
    HMODULE module;
    PTime last_write_time;
    
    game_init_proc               *game_init;
    game_frame_proc              *game_frame;
    game_size_callback_proc      *size_callback;
    game_hotload_callback_proc   *hotload_callback;
    game_get_startup_params_proc *get_startup_params;
};

#endif /* P_WIN32_MAIN_H */
