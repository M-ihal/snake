#ifndef P_PLATFORM_COMMON_H
#define P_PLATFORM_COMMON_H

#include <stdio.h>
#include <string.h>

#include <assert.h>
#ifdef _WINDOWS_
#define ASSERT(exp, str, ...) {\
char __buffer[128];\
sprintf_s(__buffer, size_array(__buffer), str##"\n%s line: %d", __VA_ARGS__, __FILE__, __LINE__);\
if(!(exp)) {\
MessageBoxA(0, __buffer, "assertion failed", 0x00000000L);\
*((byte *)0) = 0;\
}\
}
#else
#define ASSERT(exp, str, ...) { if(!(exp)) { printf(str, __VA_ARGS__); } assert((exp)); }
#endif

#define P_MATH_IMPLEMENTATION
#include "p_math.h"

#define DATA_DIR_PATH "../data/"
#define DATA_DIR(file_name) DATA_DIR_PATH##file_name

#define zero_memory(m,s) (memset(m, 0, s))
#define zero_array(a,t) memset(a, 0, size_array(a) * sizeof(t))
#define zero_struct(s) zero_memory(s, sizeof(*(s)))
#define size_array(array) (sizeof(array) / sizeof(array[0]))
#define compare_memory(m0, m1, size) (!memcmp(m0, m1, size))
#define compare_strings(s0, s1) (!strcmp(s0, s1))
#define copy_memory(s, d, size) (memcpy(d, s, size))
#define set_memory(m, v, s) (memset((void *)m, v, s))

#define bool_str(v)  ((v) ? "true" : "false")
#define print_i32(v)  printf("%s = %+d\n",#v,v)
#define print_f32(v)  printf("%s = %+.6f\n",#v,v)
#define print_fexp(v) printf("%s = %e\n",#v,v)
#define print_u64(v)  printf("%s = %llu\n",#v,v)
#define print_u32(v)  printf("%s = %lu\n",#v,v)
#define print_vec2(v) printf("%s = (%+.6f, %+.6f)\n",#v,(v).x,(v).y)
#define print_vec3(v) printf("%s = (%+.6f, %+.6f, %+.6f)\n",#v,(v).x,(v).y,(v).z)
#define print_vec4(v) printf("%s = (%+.6f, %+.6f, %+.6f, %+.6f)\n",#v,(v).x,(v).y,(v).z,(v).w)
#define print_bool(v) printf("%s = %s\n",#v,v?"true":"false")
#define print_ptr(v)  printf("%s = %p\n",#v,v)
#define print_cstr(v) printf("%s = %s\n",#v,v)
#define print_cstr_len(v,n) printf("%s = %.*s"##" | len = %d\n",#v,n,v,n)
#define print_char(v) printf("%s = %c\n",#v,v)
#define print_new_line() printf("\n")

// TODO: 
struct PTime {
    union {
        u16 e[7];
        struct {
            u16 year;
            u16 month;
            u16 day;
            u16 hour;
            u16 minute;
            u16 second;
            u16 milliseconds;
        };
    };
};

inline b32 p_time_cmp(PTime *t0, PTime *t1, bool cmp_ms = true) {
    i32 s = cmp_ms ? size_array(PTime::e) : (size_array(PTime::e) - 1);
    b32 result = compare_memory(t0, t1, s * sizeof(u16));
    return result;
}

struct FileContents {
    u32 size;
    void *contents;
};

struct WindowState {
    i32 width;
    i32 height;
    
    i32 desired_fps;
    bool cap_framerate;
    
    char title[256];
    bool resizable;
    bool is_focused;
    bool fullscreen;
    
    i32 x_fullscreen;
    i32 y_fullscreen;
    i32 window_width;
    i32 window_height;
};

typedef WindowState WindowCreationParams;

inline WindowCreationParams 
default_window_params(void) {
    WindowCreationParams params = {};
    params.width = 0;
    params.height = 0;
    params.window_width = 640;
    params.window_height = 640;
    char title[] = "window title";
    copy_memory(title, params.title, size_array(title));
    params.resizable = true;
    params.desired_fps = 0;
    params.cap_framerate = false;
    return params;
}

enum kb_key {
    KB_up,
    KB_down,
    KB_left,
    KB_right,
    
    KB_esc, KB_space, 
    KB_enter,
    
    KB_left_shift,
    KB_left_ctrl,
    
    KB_page_up, KB_page_down,
    KB_home,
    
    KB_minus, KB_equal,
    
    KB_f01, KB_f02, KB_f03,
    KB_f04, KB_f05, KB_f06,
    KB_f07, KB_f08, KB_f09,
    KB_f10, KB_f11, KB_f12,
    
    KB_q, KB_w, KB_e, KB_r, 
    KB_t, KB_y, KB_u, KB_i,
    KB_o, KB_p, KB_a, KB_s,
    KB_d, KB_f, KB_g, KB_h,
    KB_j, KB_k, KB_l, KB_z,
    KB_x, KB_c, KB_v, KB_b,
    KB_n, KB_m,
    
    KB_KEY_COUNT
};

enum mouse_button {
    MOUSE_left,
    MOUSE_right,
    
    MOUSE_BUTTON_COUNT
};

struct Input {
    f32 delta_time;
    f32 time_elapsed;
    u32 frames_elapsed;
    
    bool kb_input;
    bool kb_down[KB_KEY_COUNT];
    i32  kb_pressed[KB_KEY_COUNT];
    i32  kb_released[KB_KEY_COUNT];
    
    bool is_down(kb_key key);
    bool pressed(kb_key key);
    bool released(kb_key key);
    
    vec2 mouse;
    vec2 mouse_last;
    bool mouse_moved;
    bool mouse_input;
    i32  scroll_move;
    bool mouse_down[MOUSE_BUTTON_COUNT];
    i32  mouse_pressed[MOUSE_BUTTON_COUNT];
    i32  mouse_released[MOUSE_BUTTON_COUNT];
    
    bool is_down(mouse_button button);
    bool pressed(mouse_button button);
    bool released(mouse_button button);
};

bool Input::is_down(kb_key key) {
    bool result = this->kb_down[key];
    return result;
}

bool Input::pressed(kb_key key) {
    bool result = (this->kb_pressed[key] > 0);
    return result;
}

bool Input::released(kb_key key) {
    bool result = (this->kb_released[key] > 0);
    return result;
}

bool Input::is_down(mouse_button button) {
    bool result = this->mouse_down[button];
    return result;
}

bool Input::pressed(mouse_button button) {
    bool result = (this->mouse_pressed[button] > 0);
    return result;
}

bool Input::released(mouse_button button) {
    bool result = (this->mouse_released[button] > 0);
    return result;
}

struct ButtonState {
    bool *down;
    i32  *pressed;
    i32  *released;
};

inline ButtonState 
get_mouse_button_state(Input *input, mouse_button b) {
    ButtonState state;
    state.down = &input->mouse_down[b];
    state.pressed = &input->mouse_pressed[b];
    state.released = &input->mouse_released[b];
    return state;
}

inline ButtonState 
get_kb_key_state(Input *input, kb_key k) {
    ButtonState state;
    state.down = &input->kb_down[k];
    state.pressed = &input->kb_pressed[k];
    state.released = &input->kb_released[k];
    return state;
}

static bool
update_button_state(ButtonState key, bool is_down) {
    (*key.pressed)--;
    (*key.released)--;
    clamp_min_v(*key.pressed, 0);
    clamp_min_v(*key.released, 0);
    
    i32 _down = *key.down;
    i32 _pressed = *key.pressed;
    i32 _released = *key.released;
    
    *key.pressed  += (!_down && is_down);
    *key.released += (_down && !is_down);
    *key.down = is_down;
    
    if(*key.down || 
       _pressed != *key.pressed || 
       _released != *key.pressed) {
        return true;
    }
    return false;
}

#define ALLOC_MEMORY_PROC(name) void *name(size_t bytes)
typedef ALLOC_MEMORY_PROC(alloc_memory_proc);

#define REALLOC_MEMORY_PROC(name) bool name(void **mem_ptr, size_t bytes)
typedef REALLOC_MEMORY_PROC(realloc_memory_proc);

#define FREE_MEMORY_PROC(name) void name(void *mem)
typedef FREE_MEMORY_PROC(free_memory_proc);

#define READ_FILE_PROC(name) b32 name(FileContents *file, const char *path, bool null_terminated)
typedef READ_FILE_PROC(read_file_proc);

#define FREE_FILE_PROC(name) void name(FileContents *file)
typedef FREE_FILE_PROC(free_file_proc);

#define LAST_WRITE_TIME_PROC(name) PTime name(const char *path)
typedef LAST_WRITE_TIME_PROC(last_write_time_proc);

typedef u32 sound_id;

#define CREATE_SOUND_PROC(name) sound_id name(struct LoadedSound *sound)
typedef CREATE_SOUND_PROC(create_sound_proc);

#define DELETE_SOUND_PROC(name) void name(sound_id id)
typedef DELETE_SOUND_PROC(delete_sound_proc);

#define PLAY_SOUND_PROC(name) void name(sound_id id /*, params */)
typedef PLAY_SOUND_PROC(play_sound_proc);

struct PlatformProcs {
    alloc_memory_proc   *alloc;
    realloc_memory_proc *realloc;
    free_memory_proc    *free;
    
    read_file_proc *read_file;
    free_file_proc *free_file;
    last_write_time_proc *last_write_time;
    
    create_sound_proc *create_sound;
    delete_sound_proc *delete_sound;
    play_sound_proc   *play_sound;
};

struct MemoryBlock {
    u32 size;
    byte *ptr;
};

struct MemoryAllocationParams {
    // NOTE: in bytes
    u32 permanent_memory_size;
    u32 temporary_memory_size;
};

inline MemoryAllocationParams
default_memory_params(void) {
    MemoryAllocationParams params = {};
    params.permanent_memory_size = MB(32);
    params.temporary_memory_size = MB(8);
    return params;
}

struct GameMemory {
    bool allocated;
    MemoryBlock permanent_memory;
    MemoryBlock temporary_memory;
};

struct MemoryArena {
    byte *base;
    u32   size;
    u32   used;
};

inline MemoryArena
make_arena(void *base, u32 size) {
    MemoryArena arena;
    arena.base = (byte *)base;
    arena.size = size;
    arena.used = 0;
    return arena;
}

#define push_memory(arena_ptr, s)         push_memory_((arena_ptr), (s))
#define push_struct(arena_ptr, s)    (s *)push_memory_((arena_ptr), sizeof(s))
#define push_array(arena_ptr, s, n)  (s *)push_memory_((arena_ptr), (n) * sizeof(s))
inline void *push_memory_(MemoryArena *arena, u32 size) {
    ASSERT((arena->used + size) <= arena->size, "arena overflow");
    void *memory = nullptr;
    if((arena->used + size) <= arena->size) {
        memory = arena->base + arena->used;
        arena->used += size;
        //}
        return memory;
    }
}

#include "p_utils.h"

#include "p_vars.h"
#include "p_vars.cpp"

#include "p_renderer.h"
#include "p_renderer.cpp"

#include "p_particles.h"
#include "p_particles.cpp"

#include "p_ui.h"
#include "p_ui.cpp"

struct Core {
    Renderer *renderer;
    GameMemory *memory;
    WindowState *window;
    PlatformProcs *procs;
};

struct StartupParams {
    WindowCreationParams window;
    MemoryAllocationParams memory;
};

#include "p_game_decls.h"

#endif /* P_PLATFORM_COMMON_H */
