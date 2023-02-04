#ifndef P_GAME_MAIN_H
#define P_GAME_MAIN_H

#define game_log(str, ...) printf("GAME: "##str, __VA_ARGS__)

struct SnakePart {
    vec2i tile_pos;
    vec2i last_tile_pos;
    
    vec2i move_dir;
    vec2i last_move_dir;
    
    bool just_spawned;
};

enum tile_type {
    TILE_none,
    TILE_wall,
    TILE_apple,
    
    TILE_COUNT
};

struct Tile {
    tile_type type;
};

struct Level {
    bool initialized;
    bool looping;
    bool do_not_draw;
    
    Camera camera;
    f32 default_camera_z_pos;
    f32 desired_camera_z_pos;
    
    Tile *tiles;
    u32   width;
    u32   height;
    
    bool head_warped;
    bool tail_warped;
    SnakePart *head;
    SnakePart *snake_parts;
    u32 snake_max;
    u32 snake_count;
    
    u32   snake_next_dir_count;
    vec2i snake_next_dir[3];
    
    u32 score;
    u32 add_snake_parts;
    
    f32 move_time;
    f32 move_counter;
};

struct InitSnakeInfo {
    vec2i head_pos;
    vec2i move_dir;
    u32   length;
};

struct LevelParams {
    bool looping;
    bool init_apple;
    
    i32 width;
    i32 height;
    f32 camera_zpos;
    f32 time_per_move;
    InitSnakeInfo snake_info;
};

enum game_state {
    STATE_menu,
    STATE_game,
    STATE_gameover,
    STATE_win
};

struct GameMenu {
    MemoryArena arena;
    Level level; // NOTE: just for menu
    
    // NOTE: game_level
    i32 last_played_level = -1;
    bool colors_rev;
    
    bool choosing_level;
    bool choosing_resolution;
    
    bool restart_at_startup;
    bool making_custom_level;
    LevelParams custom_level_params;
};

enum game_level {
    LEVEL_standard,
    LEVEL_big,
    LEVEL_third,
    LEVEL_with_walls,
    LEVEL_invalid,
    LEVEL_COUNT,
    LEVEL_custom,
};

static Tile   *get_tile(Level *level, u32 x, u32 y);
static void    set_tile(Level *level, u32 x, u32 y, tile_type type);
static vec2i   get_free_tile_pos(Level *level);
inline Camera *get_level_camera(Level *level);
inline Level  *get_level(game_level _level);
static vec2    get_desired_camera_xy(Level *level);

static Level make_level(LevelParams params, MemoryArena *arena);
static void  init_snake(Level *level, vec2i start_pos, vec2i dir, u32 count);
static void  init_game(void);
static void  init_menu(void);
static void  place_apple(Level *level);

inline void transition(f32 speed = 10.0f);
static void add_snake_part(Level *level);

static void draw_level(Level *level);
static void draw_snake_body(Level *level, SnakePart *part, vec4 color, bool no_corner = false, bool no_regular = false);
static void draw_snake(Level *level);

static void game_won(void);
static void game_over(void);
static void goto_menu(bool from_game = false, bool restart_game = false);
static void switch_to_level(game_level level, f32 transition_speed = 10.0f);
static void start_game(game_level level = (game_level)0);

static bool maybe_warp(Level *level, SnakePart *part);
static void update_free_camera(void);
static void update_camera(Level *level);
static void update_snake(Level *level, vec2i move_dir);
static void update_level(Level *level, vec2i move_dir);
static void update_and_render_menu(void);
static void update_and_render_game(void);

struct GameData {
    bool initialized;
    bool quit_game;
    
    bool debug_state;
    
    sound_id sound;
    sound_id sound_counter;
    
    MemoryArena permanent_memory;
    MemoryArena temporary_memory;
    
    VarsFile tweak_file;
    
    f32 game_speed;
    f32 update_dt;
    recti32 viewport;
    u32 last_frame_draw_calls;
    u32 last_frame_quads_drawn;
    
    random_seed random;
    game_state  state;
    GameMenu    menu;
    
    UI ui;
    Font font;
    PSystem bg_particles;
    Framebuffer level_fb;
    Framebuffer framebuffer;
    ShaderRef *framebuffer_shader;
    
    f32 transition_t_desired;
    f32 transition_t;
    f32 transition_speed;
    
    bool reverse_colors;
    f32  reverse_factor;
    
    SpriteSheet sprites;
    Texture2D   sprite_particle;
    
    bool ui_kb_up;
    bool ui_kb_down;
    bool ui_kb_left;
    bool ui_kb_right;
    
#define START_COUNTER_TIME 3.0f
    f32 start_counter;
    bool paused;
    
    bool use_free_camera;
    Camera free_camera;
    
    bool win;
    bool gameover;
#define WAIT_AFTER_GAMEOVER_TIME 2.0f
    f32 gameover_wait_counter;
    i32 game_over_score;
    MemoryArena level_arena;
    game_level current_level;
    Level levels[LEVEL_COUNT];
    Level level_custom;
};

#endif /* P_GAME_MAIN_H */
