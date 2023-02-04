#ifndef P_GAME_DECLS_H
#define P_GAME_DECLS_H

#define GAME_INIT_PROC(name) bool name(Core *_core)
typedef GAME_INIT_PROC(game_init_proc);

#define GAME_FRAME_PROC(name) bool name(Core *_core, Input *_input)
typedef GAME_FRAME_PROC(game_frame_proc);

#define GAME_SIZE_CALLBACK_PROC(name) void name(Core *_core, i32 window_width, i32 window_height)
typedef GAME_SIZE_CALLBACK_PROC(game_size_callback_proc);

#define GAME_HOTLOAD_CALLBACK_PROC(name) void name(Core *_core)
typedef GAME_HOTLOAD_CALLBACK_PROC(game_hotload_callback_proc);

#define GAME_GET_STARTUP_PARAMS_PROC(name) StartupParams name(PlatformProcs *procs)
typedef GAME_GET_STARTUP_PARAMS_PROC(game_get_startup_params_proc);

/*
extern "C" __declspec(dllexport) GAME_FRAME_PROC(game_frame);
extern "C" __declspec(dllexport) GAME_SIZE_CALLBACK_PROC(size_callback);
extern "C" __declspec(dllexport) GAME_HOTLOAD_CALLBACK_PROC(hotload_callback);
extern "C" __declspec(dllexport) GAME_GET_STARTUP_PARAMS_PROC(get_startup_params);
*/

#endif /* P_GAME_DECLS_H */
