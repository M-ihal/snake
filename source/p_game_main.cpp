#include "p_platform_common.h"
#include "p_game_main.h"

extern "C" __declspec(dllexport) GAME_INIT_PROC(game_init);
extern "C" __declspec(dllexport) GAME_FRAME_PROC(game_frame);
extern "C" __declspec(dllexport) GAME_SIZE_CALLBACK_PROC(size_callback);
extern "C" __declspec(dllexport) GAME_HOTLOAD_CALLBACK_PROC(hotload_callback);
extern "C" __declspec(dllexport) GAME_GET_STARTUP_PARAMS_PROC(get_startup_params);

static Core     *core;
static Input    *input;
static GameData *game_data;

void setup_globals(Core *core_ptr, Input *input_ptr) {
    assert(core_ptr);
    
    core = core_ptr;
    input = input_ptr;
    game_data = (GameData *)core->memory->permanent_memory.ptr;
}

static Tile *
get_tile(Level *level, u32 x, u32 y) {
    assert(x >= 0 && x < level->width);
    assert(y >= 0 && y < level->height);
    Tile *tile = &level->tiles[y * level->width + x];
    return tile;
}

static void
set_tile(Level *level, u32 x, u32 y, tile_type type) {
    assert(x >= 0 && x < level->width);
    assert(y >= 0 && y < level->height);
    Tile *tile = get_tile(level, x, y);
    tile->type = type;
}

static vec2i
get_free_tile_pos(Level *level) {
    i32 x_apple = 0;
    i32 y_apple = 0;
    
    if((level->snake_count + level->add_snake_parts) >= (level->width * level->height + 1)) {
        return { -1, -1 };
    }
    
    while(true) {
        if((level->width - 1) <= 0) {
            x_apple = 0;
        }
        else {
            x_apple = rand_i32_in_range(&game_data->random, 0, level->width  - 1);
        }
        
        if((level->height - 1) <= 0) {
            y_apple = 0;
        }
        else {
            y_apple = rand_i32_in_range(&game_data->random, 0, level->height - 1);
        }
        
        bool on_snake = false;
        for(u32 i = 0; i < level->snake_count; ++i) {
            SnakePart *part = level->snake_parts + i;
            if(x_apple == part->tile_pos.x && y_apple == part->tile_pos.y) {
                on_snake = true;
                break;
            }
        }
        
        bool on_wall = false;
        Tile *tile = get_tile(level, x_apple, y_apple);
        if(tile->type == TILE_wall) {
            on_wall = true;
        }
        
        if(!on_wall && !on_snake) {
            return { x_apple, y_apple };
        }
    }
    return { -1, -1 };
}

inline Camera *
get_level_camera(Level *level) {
    Camera *camera = game_data->use_free_camera ? &game_data->free_camera : &level->camera;
    return camera;
}

inline Level *
get_level(game_level _level) {
    Level *level = nullptr;
    if(_level == LEVEL_custom) {
        level = &game_data->level_custom;
    }
    else {
        level = &game_data->levels[_level];
    }
    
    // ASSERT(level->initialized, "level not initialized...\n");
    return level;
} 

#define MAX_CAMERA_X_POS 10.0f
#define MAX_CAMERA_Y_POS 10.0f
static vec2
get_desired_camera_xy(Level *level) {
    SnakePart *head = level->head;
    f32 x_perc = 0.5f;
    f32 y_perc = 0.5f;
    if(head) {
        x_perc = (f32)head->tile_pos.x / ((f32)level->width);
        y_perc = (f32)head->tile_pos.y / ((f32)level->height);
    }
    
    x_perc = (x_perc * 2.0f) - 1.0f;
    y_perc = (y_perc * 2.0f) - 1.0f;
    
    const f32 max_x_pos = MAX_CAMERA_X_POS;
    f32 desired_x_pos = 0.0f;
    if((level->width - 1) > 0) {
        desired_x_pos = -x_perc * max_x_pos + (level->width - 1) * 0.5f;
    }
    
    const f32 max_y_pos = MAX_CAMERA_Y_POS;
    f32 desired_y_pos = 0.0f;
    if((level->height - 1) > 0) {
        desired_y_pos = -y_perc * max_y_pos + (level->height - 1) * 0.5f;
    }
    
    return { desired_x_pos, desired_y_pos };
}

static Level 
make_level(LevelParams params, MemoryArena *arena) {
    Level level = {};
    level.initialized = true;
    level.looping = params.looping;
    level.width  = params.width;
    level.height = params.height;
    level.tiles  = push_array(arena, Tile, params.width * params.height);
    level.snake_max   = params.width * params.height;
    level.snake_count = 0;
    level.snake_parts = push_array(arena, SnakePart, level.snake_max);
    level.score = 0;
    level.move_time = params.time_per_move;
    level.move_counter = 0.0f;
    
    for(u32 y = 0; y < level.height; ++y) {
        for(u32 x = 0; x < level.width; ++x) {
            Tile *tile = get_tile(&level, x, y);
            tile->type = TILE_none;
        }
    }
    
    if(params.snake_info.length > 0) {
        init_snake(&level, params.snake_info.head_pos, params.snake_info.move_dir, params.snake_info.length);
    }
    
    if(params.init_apple) {
        place_apple(&level);
    }
    
    level.default_camera_z_pos = params.camera_zpos;
    level.desired_camera_z_pos = params.camera_zpos;
    
    PerspCamera *camera = init_persp_camera(&level.camera);
    camera->aspect = (f32)game_data->framebuffer.width / (f32)game_data->framebuffer.height;
    camera->focus_on_point = true;
    camera->focus_point = { level.width * 0.5f, level.height * 0.5f, 0.0f };
    camera->position = { 0.0f, 0.0f, params.camera_zpos };
    camera->position.xy = {
        ((level.width - 1) > 0) ? (level.width - 1) * 0.5f : 0.0f,
        ((level.height - 1) > 0) ? (level.height - 1) * 0.5f : 0.0f
    };
    return level;
}

static void 
init_snake(Level *level, vec2i start_pos, vec2i dir, u32 count) {
    assert(!level->head);
    assert(!level->snake_count);
    
    SnakePart *head = &level->snake_parts[level->snake_count++];
    head->tile_pos = start_pos;
    head->move_dir = dir;
    head->last_tile_pos = head->tile_pos - head->move_dir;
    head->last_move_dir = head->move_dir;
    
    SnakePart *last = head;
    for(u32 i = 0; i < count; ++i) {
        SnakePart *next = &level->snake_parts[level->snake_count++];
        next->tile_pos = last->tile_pos - last->move_dir;
        next->move_dir = last->move_dir;
        next->last_tile_pos = next->tile_pos - next->move_dir;
        next->last_move_dir = next->move_dir;
        last = next;
    }
    
    level->head = head;
    level->snake_next_dir[0] = head->move_dir;
    level->snake_next_dir_count = 1;
}

static void
init_game(void) {
    game_data->gameover = false;
    
    game_data->level_arena.used = 0;
    zero_memory(game_data->level_arena.base, game_data->level_arena.size);
    
    /* NOTE: LEVEL_standard */ {
        LevelParams params = {
            true,
            true,
            16, 16,
            25.0f,
            0.15f,
            { { 6, 6 }, { 1, 0 }, 4 }
        };
        
        Level *level = &game_data->levels[LEVEL_standard];
        *level = make_level(params, &game_data->level_arena);
    }
    
    /* NOTE: LEVEL_big */ {
        LevelParams params = {
            true,
            true,
            64, 64,
            100.0f,
            0.05f,
            { { 24, 22 }, { 1, 0 }, 16 }
        };
        
        Level *level = &game_data->levels[LEVEL_big];
        *level = make_level(params, &game_data->level_arena);
    }
    
    /* NOTE: LEVEL_third */ {
        LevelParams params = {
            false,
            true,
            16, 16,
            25.0f,
            0.2f,
            { { 6, 6 }, { 1, 0 }, 4 }
        };
        
        Level *level = &game_data->levels[LEVEL_third];
        *level = make_level(params, &game_data->level_arena);
    }
    
    /* NOTE: LEVEL_with_walls */ {
        LevelParams params = {
            false,
            true,
            16, 16,
            25.0f,
            0.2f,
            { { 6, 6 }, { 1, 0 }, 4 }
        };
        
        Level *level = &game_data->levels[LEVEL_with_walls];
        *level = make_level(params, &game_data->level_arena);
        
        i32 wall_count = 8;
        for(i32 i = 0; i < wall_count; ++i) {
            vec2i free_pos = get_free_tile_pos(level);
            set_tile(level, free_pos.x, free_pos.y, TILE_wall);
        }
    }
}

static void
init_menu(void) {
    GameMenu *menu = &game_data->menu;
    menu->arena.used = 0;
    
    menu->last_played_level = -1;
    menu->making_custom_level = false;
    menu->custom_level_params = {};
    menu->custom_level_params.looping = true;
    menu->restart_at_startup = false;
    
    LevelParams params = {
        true, false,
        16, 1,
        25.0f,
        0.52f,
        { { 15, 0 }, { 1, 0 }, 10}
    };
    
    Level *level = &menu->level;
    *level = make_level(params, &menu->arena);
}

static void
place_apple(Level *level) {
    vec2i free = get_free_tile_pos(level);
    if(free.x != -1 && free.y != -1) {
        set_tile(level, free.x, free.y, TILE_apple);
    }
}

inline void
transition(f32 speed) {
    game_data->transition_t_desired = 1.0f;
    game_data->transition_speed = speed;
}

static void
add_snake_part(Level *level) {
    ++level->add_snake_parts;
}

static void
draw_level(Level *level) {
    flush(core->renderer);
    push(core->renderer);
    {
        Framebuffer *fb = &game_data->level_fb;
        i32 mult = 128;
        // NOTE: ...
        if((fb->width / mult) != level->width || (fb->height / mult) != level->height) {
            resize_framebuffer(core->renderer, fb, level->width * mult, level->height * mult);
        }
        bind_framebuffer(core->renderer, fb);
        set_viewport(core->renderer, { 0, 0, (i32)(level->width * mult), (i32)(level->height * mult) });
        clear(core->renderer, GRAY(0.103f, 1.0f));
        bind_shader(core->renderer, &core->renderer->shader_basic->shader);
        mat4x4 proj = mat4x4_orthographic(0.0f, 0.0f, (f32)level->width, (f32)level->height, -2.0f, 2.0f);
        mat4x4 view = mat4x4_identity();
        set_proj_and_view(core->renderer, proj, view);
        
        for(u32 y = 0; y < level->height; ++y) {
            for(u32 x = 0; x < level->width; ++x) {
                Tile *tile = get_tile(level, x, y);
                
                vec2 pos  = { (f32)x, (f32)y };
                vec2 size = { 1.0f, 1.0f };
                
                if(game_data->debug_state) {
                    draw_quad_outline(core->renderer, make_vec3(pos, -0.05f), size, 0.03f, WHITE(0.1f));
                }
                
                if(tile->type == TILE_none) {
                    continue;
                }
                
                if(game_data->debug_state) {
                    draw_quad_outline(core->renderer, make_vec3((f32)x, (f32)y, -0.1f), size, 0.075f, CYAN(0.4f));
                }
                
                vec4 color = WHITE(1.0f);
                SpriteSheet *ss = &game_data->sprites;
                i32 x_id = 0;
                i32 y_id = 0;
                
                switch(tile->type) {
                    case TILE_wall: {
                        color = WHITE(0.4f);
                        x_id = 1;
                        y_id = 0;
                    } break;
                    
                    case TILE_apple: {
                        color = WHITE(1.0f);
                        x_id = 0;
                        y_id = 0;
                    } break;
                    
                    default: {
                        color = WHITE(1.0f);
                        ASSERT(false, "");
                    };
                }
                
                draw_quad(core->renderer, pos, size, { ss, x_id, y_id }, color);
            }
        }
        draw_snake(level);
        
        flush(core->renderer);
    }
    pop(core->renderer);
    
    draw_quad(core->renderer, make_vec3(0.0f), make_vec2((f32)level->width, (f32)level->height), WHITE(1.0f), &game_data->level_fb.color);
    
    f32 line = 0.15f;
    f32 margin = 0.1f;
    vec4 color = GRAY(0.6f, 1.0f);
    if(!level->looping) {
        color = vec_lerp(vec_lerp(WHITE(0.6f), RED(0.6f), 0.5f), 
                         RED(0.8f), (sinf(input->time_elapsed * 10.0f) + 1.0f) * 0.5f);
    }
    draw_quad_outline(core->renderer, make_vec3(make_vec2(-line - margin), -0.05f), 
                      make_vec2((f32)level->width, (f32)level->height) + ((line + margin) * 2.0f), line, color);
}

static void
draw_snake_body(Level *level, SnakePart *part, vec4 color, bool no_corner, bool no_regular) {
    SpriteSheet *ss = &game_data->sprites;
    vec2i snake_head   = { 3, 3 };
    vec2i snake_tail   = { 0, 3 };
    vec2i snake_body   = { 1, 3 }; 
    vec2i snake_neck   = { 2, 3 };
    vec2i snake_corner = { 0, 1 };
    
    vec2 size = make_vec2(1.0f, 1.0f);
    vec2 tile_pos = part->tile_pos.to_vec2();
    
    // NOTE: corner
    if(part->last_move_dir != part->move_dir && !no_corner) {
        f32 rotation = 0.0f;
        vec2i last_dir = part->last_move_dir;
        vec2i curr_dir = part->move_dir;
        
        // TODO:
        if(last_dir == make_vec2i(1, 0) && curr_dir == make_vec2i(0, 1)) {
            rotation = PI32 * 0.0f;
        }
        else if(last_dir == make_vec2i(1, 0) && curr_dir == make_vec2i(0, -1)) {
            rotation = PI32 * 1.5f;
        }
        else if(last_dir == make_vec2i(-1, 0) && curr_dir == make_vec2i(0, 1)) {
            rotation = PI32 * 0.5f;
        }
        else if(last_dir == make_vec2i(-1, 0) && curr_dir == make_vec2i(0, -1)) {
            rotation = PI32 * 1.0f;
        }
        else if(last_dir == make_vec2i(0, 1) && curr_dir == make_vec2i(1, 0)) {
            rotation = PI32 * 1.0f;
        }
        else if(last_dir == make_vec2i(0, 1) && curr_dir == make_vec2i(-1, 0)) {
            rotation = PI32 * 1.5f;
        }
        else if(last_dir == make_vec2i(0, -1) && curr_dir == make_vec2i(1, 0)) {
            rotation = PI32 * 0.5f;
        }
        else if(last_dir == make_vec2i(0, -1) && curr_dir == make_vec2i(-1, 0)) {
            rotation = PI32 * 2.0f;
        }
        draw_quad_rotated(core->renderer, tile_pos, size, rotation, { ss, snake_corner.x, snake_corner.y }, color);
    }
    
    // NOTE: regular body part
    else {
        vec2i sprite = snake_body;
        
        f32 rotation = 0.0f;
        if(part->move_dir == make_vec2i(0, -1)) {
            rotation = PI32 * 0.5f;
        }
        else if(part->move_dir == make_vec2i(0, 1)) {
            rotation = PI32 * 1.5f;
        }
        else if(part->move_dir == make_vec2i(-1, 0)) {
            rotation = PI32 * 1.0f;
        }
        
        if((part - 1) == level->head) {
            sprite = snake_neck;
        }
        
        if(part == (level->snake_parts + (level->snake_count - 1))) {
            sprite = snake_neck;
            rotation += PI32;
        }
        
        if(no_regular) {
            sprite = snake_neck;
        }
        
        draw_quad_rotated(core->renderer, tile_pos, size, rotation, { ss, sprite.x, sprite.y }, color);
    }
}

static void
draw_snake(Level *level) {
    if(!level->head) {
        return;
    }
    
    SpriteSheet *ss = &game_data->sprites;
    vec2i snake_head   = { 3, 3 };
    vec2i snake_tail   = { 0, 3 };
    vec2i snake_body   = { 1, 3 }; 
    vec2i snake_neck   = { 2, 3 };
    vec2i snake_corner = { 0, 1 };
    
    f32 perc = level->move_counter / level->move_time;
    for(u32 i = 0; i < level->snake_count; ++i) {
        SnakePart *part = level->snake_parts + i;
        
        vec2 tile_pos = part->tile_pos.to_vec2();
        
        vec2 pos  = vec_lerp(part->last_tile_pos.to_vec2(), part->tile_pos.to_vec2(), perc);
        vec2 size = { 1.0f, 1.0f };
        
        f32 color_t = (f32)i / (f32)(level->snake_count);
        vec4 color0 = WHITE(1.0f);
        vec4 color1 = GRAY(0.6f, 1.0f);
        vec4 color = vec_lerp(color0, color1, color_t);
        
        if(game_data->debug_state) {
            draw_quad_outline(core->renderer, make_vec3(part->tile_pos.to_vec2(), -0.1f), size, 0.075f, CYAN(0.4f));
        }
        
        if(part == level->head) {
            QuadExParams params = quad_ex_default();
            params.spritesheet = true;
            params.ss_tile = { ss, snake_head.x, snake_head.y };
            params.color = color;
            
            vec2 head_size = { size.x, size.y };
            vec2 head_pos = pos;
            
            f32 rotation = 0.0f;
            if(part->move_dir == make_vec2i(0, -1)) {
                rotation = PI32 * 0.5f;
            }
            else if(part->move_dir == make_vec2i(0, 1)) {
                rotation = PI32 * 1.5f;
            }
            else if(part->move_dir == make_vec2i(-1, 0)) {
                params.flip_x = true;
            }
            
            params.rotation = rotation;
            draw_quad_ex(core->renderer, make_vec3(head_pos, -0.05f), head_size, &params);
            
            if(level->head_warped) {
                vec3 _pos = make_vec3(head_pos, -0.05f);
                if(part->move_dir == make_vec2i(1, 0)) {
                    _pos.x += level->width;
                }
                else if(part->move_dir == make_vec2i(-1, 0)) {
                    _pos.x -= level->width;
                }
                else if(part->move_dir == make_vec2i(0, 1)) {
                    _pos.y += level->height;
                }
                else if(part->move_dir == make_vec2i(0, -1)) {
                    _pos.y -= level->height;
                }
                draw_quad_ex(core->renderer, _pos, head_size, &params);
            }
        }
        else if(i == (level->snake_count - 1)) {
            f32 rotation = 0.0f;
            if(part->last_move_dir == make_vec2i(0, -1)) {
                rotation = PI32 * 0.5f;
            }
            else if(part->last_move_dir == make_vec2i(0, 1)) {
                rotation = PI32 * 1.5f;
            }
            else if(part->last_move_dir == make_vec2i(-1, 0)) {
                rotation = PI32 * 1.0f;
            }
            
            f32 z_pos = 0.1f;
            draw_quad_rotated(core->renderer, make_vec3(pos, z_pos), size, rotation, { ss, snake_tail.x, snake_tail.y }, color);
            if(level->tail_warped) {
                vec3 _pos = make_vec3(pos.x, pos.y, z_pos);
                if(part->move_dir == make_vec2i(1, 0)) {
                    _pos.x += level->width;
                }
                else if(part->move_dir == make_vec2i(-1, 0)) {
                    _pos.x -= level->width;
                }
                else if(part->move_dir == make_vec2i(0, 1)) {
                    _pos.y += level->height;
                }
                else if(part->move_dir == make_vec2i(0, -1)) {
                    _pos.y -= level->height;
                }
                draw_quad_rotated(core->renderer, _pos, size, rotation, { ss, snake_tail.x, snake_tail.y }, color);
            }
            
            if(!part->just_spawned) {
                draw_snake_body(level, part, color, true);
            }
        }
        else {
            bool no_regular = false;
            if((i + 1) == (level->snake_count - 1)) {
                if((part + 1)->just_spawned) {
                    no_regular = true;
                }
            }
            draw_snake_body(level, part, color, false, no_regular);
        }
    }
}

static void
game_won(void) {
    random_seed seed = 144;
    PSystemParams bg_particles_params = game_data->bg_particles.params;
    delete_particles(&game_data->bg_particles, core->procs);
    bg_particles_params.no_textures();
    bg_particles_params.add_texture(&game_data->font.glyphs['W'].tex);
    bg_particles_params.add_texture(&game_data->font.glyphs['I'].tex);
    bg_particles_params.add_texture(&game_data->font.glyphs['N'].tex);
    bg_particles_params.move_speed.set(2.0f);
    bg_particles_params.direction.set(-180.0f, 0.0f);
    bg_particles_params.size.set({ 0.2f, 0.2f }, { 0.8f, 0.8f });
    bg_particles_params.desired_size.set({ 0.05f, 0.05f }, { 0.0f, 0.0f});
    bg_particles_params.color.set(random_color(&seed, 1.0f), random_color(&seed, 1.0f));
    bg_particles_params.desired_color.set(random_color(&seed, 1.0f), random_color(&seed, 1.0f));
    init_particles(&game_data->bg_particles, 16394, 1024, bg_particles_params, core->procs);
    
    game_data->win = true;
    game_over();
}

static void
game_over(void) {
    game_data->gameover = true;
    game_data->gameover_wait_counter = WAIT_AFTER_GAMEOVER_TIME;
}

static void
goto_menu(bool from_game, bool restart_game) {
    GameMenu *menu = &game_data->menu;
    game_data->state = STATE_menu;
    menu->choosing_level = false;
    menu->choosing_resolution = false;
    menu->making_custom_level = false;
    menu->level.desired_camera_z_pos = menu->level.default_camera_z_pos;
    menu->level.camera.persp.position.z    = menu->level.default_camera_z_pos;
    transition();
    
    if(from_game) {
        menu->restart_at_startup = restart_game;
        menu->last_played_level = (restart_game) ? -1 : game_data->current_level;
        if(game_data->reverse_colors) {
            menu->colors_rev = true;
        }
        game_data->reverse_colors = false;
    }
}


static void 
switch_to_level(game_level level, f32 transition_speed) {
    ASSERT((level >= 0 && level < LEVEL_COUNT) || level == LEVEL_custom, "wrong level");
    transition(transition_speed);
    game_data->current_level = level;
    game_data->start_counter = START_COUNTER_TIME;
}

static void
start_game(game_level level) {
    game_data->state = STATE_game;
    switch_to_level(level);
}

static bool
maybe_warp(Level *level, SnakePart *part) {
    vec2i tile_pos = part->tile_pos;
    
    bool warped = true;
    if(part->tile_pos.x < 0) {
        part->tile_pos.x = level->width - 1;
    }
    else if(part->tile_pos.x > (i32)(level->width - 1)) {
        part->tile_pos.x = 0;
    }
    else if(part->tile_pos.y < 0) {
        part->tile_pos.y = level->height - 1;
    }
    else if(part->tile_pos.y > (i32)(level->height - 1)) {
        part->tile_pos.y = 0;
    }
    else {
        warped = false;
    }
    
    if(warped) {
        vec2i diff = tile_pos - part->last_tile_pos;
        part->last_tile_pos = part->tile_pos - diff;
    }
    return warped;
}

static void
update_free_camera(void) {
    PerspCamera *camera = &game_data->free_camera.persp;
    
    camera->focus_on_point = false;
    camera->aspect = (f32)core->window->width / (f32)core->window->height;
    
    if(core->window->is_focused && input->is_down(MOUSE_right)) {
        rotate_persp_camera(camera, input->mouse - input->mouse_last, 0.01f);
    }
    
    f32 move = 0.0f;
    f32 side_move = 0.0f;
    if(input->is_down(KB_w)) { move += 1.0f; }
    if(input->is_down(KB_s)) { move -= 1.0f; }
    if(input->is_down(KB_a)) { side_move = -1.0f; }
    if(input->is_down(KB_d)) { side_move = +1.0f; }
    vec3 dir = vec_from_spherical(1.0f, camera->horiz_rotation, camera->vert_rotation);
    vec3 side_dir = vec_from_spherical(1.0f, camera->horiz_rotation + PI32 * 0.5f, PI32 * 0.5f);
    
    f32 speed = 10.0f;
    if(input->is_down(KB_left_shift)) {
        speed *= 4.0f;
    }
    camera->position += dir      * move      * speed * input->delta_time;
    camera->position += side_dir * side_move * speed * input->delta_time;
}

static void
update_camera(Level *level) {
    if(game_data->use_free_camera) {
        if(!game_data->debug_state) {
            game_data->use_free_camera = false;
        }
        else {
            update_free_camera();
        }
    }
    else {
        PerspCamera *camera = &level->camera.persp;
        vec2 desired = get_desired_camera_xy(level);
        camera->aspect = (f32)game_data->framebuffer.width / (f32)game_data->framebuffer.height;
        
        f32 speed = 1.25f;
        camera->position.x = lerp(camera->position.x, desired.x, game_data->update_dt * speed);
        camera->position.y = lerp(camera->position.y, desired.y, game_data->update_dt * speed);
        
        if(input->pressed(KB_home)) {
            level->desired_camera_z_pos = level->default_camera_z_pos;
        }
        else {
            i32 dir = 0;
            if(input->is_down(KB_page_up))   { dir -= 1; }
            if(input->is_down(KB_page_down)) { dir += 1; }
            if(!dir) {
                dir = -input->scroll_move * 20;
            }
            if(dir) {
                level->desired_camera_z_pos += (f32)dir * input->delta_time * 16.0f;
                clamp_v(level->desired_camera_z_pos, 4.0f, 128.0f);
            }
        }
        camera->position.z = lerp(camera->position.z, level->desired_camera_z_pos, input->delta_time * 8.0f);
    }
}

static void
update_snake(Level *level, vec2i move_dir) {
    if(!level->head) {
        return;
    }
    
    SnakePart *head = level->head;
    vec2i new_move_dir = move_dir;
    
    if(new_move_dir.x != 0 || new_move_dir.y != 0) {
        if(-new_move_dir == level->snake_next_dir[level->snake_next_dir_count - 1] || 
           new_move_dir == level->snake_next_dir[level->snake_next_dir_count - 1]) {
            new_move_dir = {};
        }
        else {
            if(level->snake_next_dir_count == size_array(level->snake_next_dir)) {
                level->snake_next_dir[size_array(level->snake_next_dir) - 1] = new_move_dir;
            }
            else {
                ++level->snake_next_dir_count;
                level->snake_next_dir[level->snake_next_dir_count - 1] = new_move_dir;
            }
        }
    }
    
    if((level->move_counter += game_data->update_dt) >= level->move_time) {
        level->move_counter -= level->move_time;
        SnakePart *last_part = nullptr;
        for(u32 i = 0; i < level->snake_count; ++i) {
            SnakePart *part = level->snake_parts + i;
            
            if(part == head) {
                // NOTE: get next move dir
                if(level->snake_next_dir_count > 1) {
                    for(u32 move = 0; move < (level->snake_next_dir_count - 1); ++move) {
                        level->snake_next_dir[move] = level->snake_next_dir[move + 1];
                    }
                    --level->snake_next_dir_count;
                }
                
                part->move_dir = level->snake_next_dir[0];
                part->last_tile_pos = part->tile_pos;
                part->tile_pos += part->move_dir;
                part->last_move_dir = part->move_dir;
                
                level->head_warped = false;
                if(!level->looping) {
                    if(!is_in_bounds(part->tile_pos.x, 0, (i32)(level->width - 1)) ||
                       !is_in_bounds(part->tile_pos.y, 0, (i32)(level->height - 1))) {
                        game_over();
                        return;
                    }
                }
                else {
                    if(maybe_warp(level, part)) {
                        level->head_warped = true;
                    }
                }
            }
            else {
                if(part->just_spawned) {
                    part->just_spawned = false;
                }
                
                part->last_tile_pos = part->tile_pos;
                part->tile_pos += part->move_dir;
                part->last_move_dir = part->move_dir;
                part->move_dir = last_part->last_move_dir;
                
                bool warped = maybe_warp(level, part);
                if(i == (level->snake_count - 1)) {
                    level->tail_warped = warped;
                }
            }
            last_part = part;
        }
        
        // NOTE: check for suicide
        for(u32 i = 1; i < level->snake_count; ++i) {
            SnakePart *part = level->snake_parts + i;
            if(head->tile_pos == part->tile_pos) {
                game_over();
                return;
            }
        }
        
        // NOTE: check for collisions with tiles
        Tile *tile = get_tile(level, head->tile_pos.x, head->tile_pos.y);
        switch(tile->type) {
            case TILE_wall: {
                game_over();
                return;
            } break;
            
            case TILE_apple: {
                tile->type = TILE_none;
                add_snake_part(level);
                
                core->procs->play_sound(game_data->sound);
                
                if((level->snake_count + level->add_snake_parts) >= (level->width * level->height + 1)) {
                    game_won();
                }
                
                place_apple(level);
                ++level->score;
            } break;
            
            default: {
                // NOTE: do nothing
            };
        }
        
        if(level->add_snake_parts > 0) {
            --level->add_snake_parts;
            SnakePart *last_p = &level->snake_parts[level->snake_count - 1];
            SnakePart *new_p  = &level->snake_parts[level->snake_count++];
            new_p->tile_pos = last_p->tile_pos;
            new_p->last_tile_pos = last_p->last_tile_pos;
            new_p->just_spawned = true;
            // new_p->move_dir = last_p->last_move_dir;
        }
    }
}

static void 
update_level(Level *level, vec2i move_dir) {
    update_camera(level);
    if(!game_data->paused) {
        update_snake(level, move_dir);
    }
}

static void
update_and_render_menu(void) {
    GameMenu *menu = &game_data->menu;
    
    if(menu->restart_at_startup) {
        init_game();
        menu->restart_at_startup = false;
    }
    
    vec2 screen_dim = { (f32)game_data->framebuffer.width, (f32)game_data->framebuffer.height };
    
    Font *font = &game_data->font;
    f32 title_height = 92.0f;
    char title[] = "SNAKE";
    vec2 title_dim = get_text_size(title, font, title_height);
    vec2 title_pos = { screen_dim.x - title_dim.x - 16.0f, screen_dim.y - title_dim.y + 16.0f };
    f32  title_color_t = (cosf(input->time_elapsed * 2.0f) + 1.0f) * 0.5f;
    vec4 title_color = WHITE(0.5f); // vec_lerp(, WHITE(0.25f), title_color_t);
    draw_text(core->renderer, title, title_pos, title_height, font, title_color);
    
    ButtonTheme theme = {};
    theme.style = BUTTON_STYLE_old;
    theme.font = font;
    theme.text_scale  = 1.0f;
    theme.border_width = 4.0f;
    
    f32 alpha = 0.9f;
    theme.bg_color       = GRAY(0.55f, alpha);
    theme.bg_color_hover = GRAY(0.65f, alpha);
    theme.bg_color_down  = theme.bg_color; // GRAY(0.8f, alpha);
    theme.bg_color_inactive = theme.bg_color_down; // GRAY(0.8f, alpha);
    theme.color       = GRAY(0.1f, 1.0f);
    theme.color_hover = GRAY(0.1f, 1.0f);
    theme.color_down  = GRAY(0.2f, 1.0f);
    theme.color_inactive = GRAY(0.2f, 1.0f);
    theme.border_color       = GRAY(0.55f, alpha);
    theme.border_color_hover = GRAY(0.65f, alpha);
    theme.border_color_down  = theme.border_color; // GRAY(0.8f, alpha);
    theme.border_color_inactive = theme.border_color; // GRAY(0.8f, alpha);
    
    // NOTE: bug if 43.0f etc
    vec2 button_size = { 192.0f, 34.0f };
    
    bool start_the_game = false;
    game_level start_level = LEVEL_standard;
    if(input->released(KB_enter)) {
        start_the_game = true;
    }
    
    bool quit_game = false;
    if(input->released(KB_esc)) {
        quit_game = true;
    }
    
    // NOTE: meeeh
    f32 space = 4.0f;
    ui_begin(&game_data->ui);
    {
        if(menu->making_custom_level) {
            ButtonLayout layout = setup_button_layout(LAYOUT_vertical, { 10.0f, 10.0f }, space, button_size);
            button_size = { 160.0f, button_size.y };
            
            {
                ButtonLayout layout_row = setup_button_layout(LAYOUT_horizontal, next_button_position(&layout), 4.0f, button_size); 
                
                if(do_button(&game_data->ui, input, "back", next_button_position(&layout_row), 
                             button_size, &theme, { 11231 })) {
                    menu->making_custom_level = false;
                }
                
                if(do_button(&game_data->ui, input, "create", next_button_position(&layout_row), 
                             button_size, &theme, { 1112321 })) {
                    init_game();
                    
                    game_data->win = false;
                    game_data->gameover = false;
                    
                    // NOTE: stupid
                    delete_particles(&game_data->bg_particles, core->procs);
                    PSystemParams bg_particles_params = {};
                    bg_particles_params.add_texture(&game_data->sprite_particle);
                    bg_particles_params.spawn_area = AREA_quad;
                    bg_particles_params.sides = { 160.0f, 160.0f };
                    bg_particles_params.rotated_particles = false;
                    bg_particles_params.rotation.set(0.0f);
                    bg_particles_params.size.set({ 0.2f, 0.2f }, { 0.4f, 0.4f });
                    bg_particles_params.desired_size.set({ 0.05f, 0.05f }, { 0.0f, 0.0f});
                    bg_particles_params.direction.set(0.0f, 360.0f);
                    bg_particles_params.color.set(WHITE(1.0f), GRAY(0.9f, 1.0f));
                    bg_particles_params.desired_color.set(WHITE(0.2f), WHITE(0.4f));
                    bg_particles_params.life_time.set(2.0f, 4.0f);
                    bg_particles_params.fade_in.set(0.2f, 0.4f);
                    init_particles(&game_data->bg_particles, 8096, 256, bg_particles_params, core->procs);
                    
                    
                    menu->custom_level_params.init_apple = true;
                    menu->custom_level_params.snake_info = { { 4, 1 }, { 1, 0 }, 2};
                    game_data->level_custom = make_level(menu->custom_level_params, &game_data->level_arena);
                    start_game(LEVEL_custom);
                }
            }
            
            
            {
                ButtonLayout layout_row = setup_button_layout(LAYOUT_horizontal, next_button_position(&layout), space, button_size); 
                {
                    /* NOTE: size */ {
                        char *strings[] = {
                            "8",
                            "16",
                            "32",
                            "64",
                        };
                        
                        i32 sizes[] = {
                            8,
                            16,
                            32,
                            64,
                        };
                        
                        f32 z_positions[] = {
                            22.0f,
                            25.0f,
                            50.0f,
                            100.0f,
                        };
                        
                        /* NOTE: width */ {
                            i32 which = -1;
                            for(i32 i = 0; i < size_array(sizes); ++i) {
                                if(sizes[i] == menu->custom_level_params.width) {
                                    which = i;
                                }
                            }
                            
                            if(which == -1) {
                                which = 0;
                                menu->custom_level_params.width  = sizes[0];
                                menu->custom_level_params.camera_zpos = z_positions[0];
                            }
                            
                            if(do_button(&game_data->ui, input, strings[which], next_button_position(&layout_row), 
                                         button_size, &theme, { 1135 })) {
                                if(++which >= size_array(sizes)) {
                                    which = 0;
                                }
                                menu->custom_level_params.width  = sizes[which];
                                
                                if(menu->custom_level_params.width >= menu->custom_level_params.height) {
                                    menu->custom_level_params.camera_zpos = z_positions[which];
                                }
                            }
                        }
                        
                        /* NOTE: height */ {
                            i32 which = -1;
                            for(i32 i = 0; i < size_array(sizes); ++i) {
                                if(sizes[i] == menu->custom_level_params.height) {
                                    which = i;
                                }
                            }
                            
                            if(which == -1) {
                                which = 0;
                                menu->custom_level_params.height = sizes[0];
                            }
                            
                            if(do_button(&game_data->ui, input, strings[which], next_button_position(&layout_row), 
                                         button_size, &theme, { 11123135 })) {
                                if(++which >= size_array(sizes)) {
                                    which = 0;
                                }
                                menu->custom_level_params.height = sizes[which];
                                
                                if(menu->custom_level_params.width <= menu->custom_level_params.height) {
                                    menu->custom_level_params.camera_zpos = z_positions[which];
                                }
                            }
                        }
                    }
                }
            }
            
            
            {
                ButtonLayout layout_row = setup_button_layout(LAYOUT_horizontal, next_button_position(&layout), space, button_size); 
                
                char *strings[] = {
                    "0.05s",
                    "0.10s",
                    "0.20s",
                    "0.50s",
                    "1.00s",
                };
                
                f32 times[] = {
                    0.05f,
                    0.1f,
                    0.2f,
                    0.5f,
                    1.0f
                };
                
                i32 which = -1;
                for(i32 i = 0; i < size_array(times); ++i) {
                    if(times[i] == menu->custom_level_params.time_per_move) {
                        which = i;
                    }
                }
                
                if(which == -1) {
                    which = 0;
                    menu->custom_level_params.time_per_move = times[which];
                }
                
                if(do_button(&game_data->ui, input, strings[which], next_button_position(&layout_row), 
                             button_size, &theme, { 5436534 })) {
                    if(++which >= size_array(times)) {
                        which = 0;
                    }
                    menu->custom_level_params.time_per_move = times[which];
                }
            }
            
            {
                ButtonLayout layout_row = setup_button_layout(LAYOUT_horizontal, next_button_position(&layout), space, button_size); 
                if(do_button(&game_data->ui, input, "looping", next_button_position(&layout_row),
                             button_size, &theme, 312137, menu->custom_level_params.looping)) {
                    menu->custom_level_params.looping = true;
                }
                if(do_button(&game_data->ui, input, "no-looping", next_button_position(&layout_row),
                             button_size, &theme, 31211, !menu->custom_level_params.looping)) {
                    menu->custom_level_params.looping = false;
                }
            }
        }
        else {
            ButtonLayout layout = setup_button_layout(LAYOUT_vertical, { 10.0f, 10.0f }, space, button_size);
            
            if(input->is_down(KB_esc)) {
                draw_button(core->renderer, "quit", BUTTON_down,
                            next_button_position(&layout), button_size, &theme);
            }
            else {
                if(do_button(&game_data->ui, input, "quit", 
                             next_button_position(&layout), button_size, &theme, { 12833 })) {
                    game_data->quit_game = true;
                }
            }
            
            vec2 resolution_button_pos = next_button_position(&layout);
            do_button_toggle(&game_data->ui, input, (core->window->fullscreen) ? "resolution" : "window size",
                             &menu->choosing_resolution, resolution_button_pos, button_size, &theme, { 111123 });
            if(menu->choosing_resolution) {
                menu->choosing_level = false;
            }
            if(menu->choosing_resolution) {
                ButtonLayout resolutions_layout = setup_button_layout(LAYOUT_vertical, resolution_button_pos + make_vec2(button_size.x + space, 0.0f), space, button_size, false);
                // offset_for_button_layout(&resolutions_layout, 3);
                
                if(core->window->fullscreen) {
                    u32 x_res = core->window->x_fullscreen;
                    u32 y_res = core->window->y_fullscreen;
                    
                    if(do_button(&game_data->ui, input, "800x600", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 11233 }, (x_res == 800 && y_res == 600))) {
                        core->window->x_fullscreen = 800;
                        core->window->y_fullscreen = 600;
                        // choose_resolution = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "1280x720", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 143 }, (x_res == 1280 && y_res == 720))) {
                        core->window->x_fullscreen = 1280;
                        core->window->y_fullscreen = 720;
                        // choose_resolution = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "1920x1080", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 12664 }, (x_res == 1920 && y_res == 1080))) {
                        core->window->x_fullscreen = 1920;
                        core->window->y_fullscreen = 1080;
                        // choose_resolution = false;
                    }
                }
                else {
                    u32 width  = core->window->window_width;
                    u32 height = core->window->window_height;
                    
                    if(do_button(&game_data->ui, input, "640x640", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 132133 }, (width == 640 && height == 640))) {
                        core->window->window_width  = 640;
                        core->window->window_height = 640;
                        // choose_resolution = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "960x540", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 1264124 }, (width == 960 && height == 540))) {
                        core->window->window_width  = 960;
                        core->window->window_height = 540;
                        // choose_resolution = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "1280x720", next_button_position(&resolutions_layout), 
                                 button_size, &theme, { 1121133 }, (width == 1280 && height == 720))) {
                        core->window->window_width  = 1280;
                        core->window->window_height = 720;
                        // choose_resolution = false;
                    }
                }
            }
            
            do_button_toggle(&game_data->ui, input, "fullscreen", &core->window->fullscreen, next_button_position(&layout), button_size, &theme, 231312312);
            
            
            /* else (?) */ {
                vec2 start_button_pos = next_button_position(&layout);
                
                if(menu->last_played_level == -1) {
                    do_button_toggle(&game_data->ui, input, "start game", &menu->choosing_level, start_button_pos, button_size, &theme, { 12633 });
                    if(menu->choosing_level) {
                        menu->choosing_resolution = false;
                    }
                }
                else {
                    if(do_button(&game_data->ui, input, "continue", start_button_pos, button_size, &theme, { 12633 })) {
                        start_the_game = true;
                    }
                }
                
                if(menu->choosing_level) {
                    ButtonLayout level_layout = setup_button_layout(LAYOUT_vertical, start_button_pos + make_vec2(button_size.x + space, 0.0f), space, button_size, true);
                    
                    u32 width  = core->window->window_width;
                    u32 height = core->window->window_height;
                    
                    if(do_button(&game_data->ui, input, "standard", next_button_position(&level_layout), 
                                 button_size, &theme, { 132133 })) {
                        start_the_game = true;
                        start_level = LEVEL_standard;
                        menu->choosing_level = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "big", next_button_position(&level_layout), 
                                 button_size, &theme, { 1323123 })) {
                        start_the_game = true;
                        start_level = LEVEL_big;
                        menu->choosing_level = false;
                    }
                    
                    if(do_button(&game_data->ui, input, "third", next_button_position(&level_layout), 
                                 button_size, &theme, { 131123 })) {
                        start_the_game = true;
                        start_level = LEVEL_third;
                        menu->choosing_level = false;
                    }
                    
#if 0
                    if(do_button(&game_data->ui, input, "with walls", next_button_position(&level_layout), 
                                 button_size, &theme, { 131213123 })) {
                        start_the_game = true;
                        start_level = LEVEL_with_walls;
                        choose_level = false;
                    }
#endif
                    
                    if(do_button(&game_data->ui, input, "custom level", next_button_position(&level_layout), 
                                 button_size, &theme, { 11231 })) {
                        menu->making_custom_level = true;
                    }
                }
            }
            
            if(menu->last_played_level != -1) {
                if(do_button(&game_data->ui, input, "restart", 
                             next_button_position(&layout), button_size, &theme, { 1213 })) {
                    init_game();
                    init_menu();
                    transition();
                }
            }
        }
    }
    ui_end(&game_data->ui);
    
    if(start_the_game) {
        game_data->win = false;
        game_data->gameover = false;
        
        // TODO:
        delete_particles(&game_data->bg_particles, core->procs);
        PSystemParams bg_particles_params = {};
        bg_particles_params.add_texture(&game_data->sprite_particle);
        bg_particles_params.spawn_area = AREA_quad;
        bg_particles_params.sides = { 160.0f, 160.0f };
        bg_particles_params.rotated_particles = false;
        bg_particles_params.rotation.set(0.0f);
        bg_particles_params.size.set({ 0.2f, 0.2f }, { 0.4f, 0.4f });
        bg_particles_params.desired_size.set({ 0.05f, 0.05f }, { 0.0f, 0.0f});
        bg_particles_params.direction.set(0.0f, 360.0f);
        bg_particles_params.color.set(WHITE(1.0f), GRAY(0.9f, 1.0f));
        bg_particles_params.desired_color.set(WHITE(0.2f), WHITE(0.4f));
        bg_particles_params.life_time.set(2.0f, 4.0f);
        bg_particles_params.fade_in.set(0.2f, 0.4f);
        init_particles(&game_data->bg_particles, 8096, 256, bg_particles_params, core->procs);
        
        
        if(menu->last_played_level == -1) {
            start_game(start_level);
        }
        else {
            start_game((game_level)menu->last_played_level);
            if(menu->colors_rev) {
                game_data->reverse_colors = true;
                menu->colors_rev = false;
            }
        }
    }
    
    if(quit_game) {
        game_data->quit_game = true;
    }
    
    Level *level = &game_data->menu.level;
    Camera *camera = get_level_camera(level);
    set_camera(core->renderer, camera);
    {
        update_level(level, { 1, 0 });
        draw_level(level);
        
        game_data->bg_particles.position.xy = { level->width * 0.5f, level->height * 0.5f };
        game_data->bg_particles.position.z = -1.0f;
        update_and_render_particles(&game_data->bg_particles, core->renderer, input->delta_time);
        if(game_data->debug_state) { draw_psystem_border(&game_data->bg_particles, core->renderer); }
    }
    flush(core->renderer);
}

static void
update_and_render_game(void) {
    bool draw_counter = false;
    Level *level = get_level(game_data->current_level);
    
    if(!level->initialized) {
        ui_begin(&game_data->ui);
        {
            Font *font = &game_data->font;
            vec2 screen_center = make_vec2((f32)game_data->ui.width, (f32)game_data->ui.height) * 0.5f;
            LabelTheme theme = {};
            theme.style = LABEL_STYLE_simple;
            theme.font_height = 64.0f * ((((cosf(input->time_elapsed * 1.0f) + 1.0f) * 0.5f) * 0.5f) + 0.5f);
            theme.color = WHITE(1.0f);
            theme.bg_color = BLACK(1.0f);
            theme.font = font;
            char text[] = "INVALID LEVEL";
            vec2 text_dim = get_text_size(text, font, theme.font_height);
            vec2 size = { text_dim.x * 1.05f , text_dim.y * 1.2f };
            do_label(&game_data->ui, text, screen_center - size * 0.5f, 
                     size, &theme);
        }
        ui_end(&game_data->ui);
        
        update_free_camera();
        set_camera(core->renderer, &game_data->free_camera);
        {
            game_data->bg_particles.position.xy = {};
            game_data->bg_particles.position.z = -1.0f;
            update_and_render_particles(&game_data->bg_particles, core->renderer, input->delta_time);
            if(game_data->debug_state) { draw_psystem_border(&game_data->bg_particles, core->renderer); }
        }
        flush(core->renderer);
    }
    else {
        if((i32)(game_data->start_counter - input->delta_time) != (START_COUNTER_TIME - 1) &&
           ((i32)(game_data->start_counter - input->delta_time) != (i32)game_data->start_counter)) {
            core->procs->play_sound(game_data->sound_counter);
        }
        
        if((game_data->start_counter -= input->delta_time) <= 0.0f) {
            
            clamp_min_v(game_data->start_counter, 0.0f);
            
            if(!game_data->gameover) {
                vec2i move_dir = {};
                if(input->pressed(KB_up)    || game_data->ui_kb_up)    { move_dir = {  0, +1 }; }
                if(input->pressed(KB_down)  || game_data->ui_kb_down)  { move_dir = {  0, -1 }; }
                if(input->pressed(KB_left)  || game_data->ui_kb_left)  { move_dir = { -1,  0 }; }
                if(input->pressed(KB_right) || game_data->ui_kb_right) { move_dir = { +1,  0 }; }
                update_level(level, move_dir);
            }
            else {
                update_camera(level);
            }
        }
        else {
            update_camera(level);
            draw_counter = true;
        }
        
        Camera *camera = get_level_camera(level);
        set_camera(core->renderer, camera);
        {
            draw_level(level); 
            
            game_data->bg_particles.position.xy = { level->width * 0.5f, level->height * 0.5f };
            game_data->bg_particles.position.z = -1.0f;
            update_and_render_particles(&game_data->bg_particles, core->renderer, input->delta_time);
            if(game_data->debug_state) { draw_psystem_border(&game_data->bg_particles, core->renderer); }
        }
        flush(core->renderer);
    }
    
    if(game_data->gameover) {
        ui_begin(&game_data->ui);
        {
            f32 perc = game_data->gameover_wait_counter / WAIT_AFTER_GAMEOVER_TIME;
            Font *font = &game_data->font;
            vec2 screen_dim = make_vec2((f32)game_data->ui.width, (f32)game_data->ui.height);
            LabelTheme theme = {};
            theme.style = LABEL_STYLE_simple;
            theme.font_height = 64.0f * (1.0f - perc) * ((((cosf(input->time_elapsed * 1.0f) + 1.0f) * 0.5f) * 0.5f) + 0.5f);
            theme.color = WHITE(1.0f);
            theme.bg_color = BLACK(1.0f);
            theme.font = font;
            char *text = game_data->win ? "WIN !!!\0" : "GAME OVER\0";
            vec2 text_dim = get_text_size(text, font, theme.font_height);
            vec2 size = { text_dim.x * 1.05f , text_dim.y * 1.2f };
            do_label(&game_data->ui, text, make_vec2(screen_dim.x * 0.5f, screen_dim.y * 0.8f)
                     - size * 0.5f, size, &theme);
        }
        ui_end(&game_data->ui);
        
        if((game_data->gameover_wait_counter -= input->delta_time) <= 0.0f) {
            game_data->game_over_score = get_level(game_data->current_level)->score;
            goto_menu(true, true);
            game_data->state = game_data->win ? STATE_win : STATE_gameover;
        }
    }
    
    vec2 screen_dim = { (f32)game_data->framebuffer.width, (f32)game_data->framebuffer.height };
    
    Font *font = &game_data->font;
    f32 title_height = 48.0f;
    char title[128];
    sprintf_s(title, size_array(title), "SCORE: %lu", level->score);
    vec2 title_dim = get_text_size(title, font, title_height);
    vec2 title_pos = { screen_dim.x - title_dim.x - 16.0f, screen_dim.y - title_dim.y + 2.0f };
    f32  title_color_t = (cosf(input->time_elapsed * 2.0f) + 1.0f) * 0.5f;
    vec4 title_color = WHITE(0.5f); // vec_lerp(, WHITE(0.25f), title_color_t);
    draw_text(core->renderer, title, title_pos, title_height, font, title_color);
    
    ui_begin(&game_data->ui); 
    {
        if(draw_counter) {
#define DIGIT_TO_CHAR(digit) ((char)('9' - (9 - clamp(digit, 0, 9))))
            char buffer[2] = { DIGIT_TO_CHAR(roundf32_to_i32(game_data->start_counter + 0.5f)), '\0' };
            f32 line_height = 128.0f;
            vec2 text_dim = get_text_size(buffer, font, line_height, false);
            vec2 draw_pos = {
                screen_dim.x * 0.5f - text_dim.x * 0.5f,
                screen_dim.y - text_dim.y - 25.0f
            };
            draw_text(core->renderer, buffer, draw_pos, line_height, font);
        }
        
        {
            ui_begin(&game_data->ui);
            {
                ButtonTheme arrow_theme = {};
                arrow_theme.style = BUTTON_STYLE_old;
                arrow_theme.border_width = 3.5f;
                arrow_theme.bg_color       = GRAY(0.5f, 1.0f);
                arrow_theme.bg_color_hover = GRAY(0.7f, 1.0f);
                arrow_theme.bg_color_down  = GRAY(0.9f, 1.0f);
                arrow_theme.color       = BLACK(1.0f);
                arrow_theme.color_hover = BLACK(1.0f);
                arrow_theme.color_down  = BLACK(1.0f);
                arrow_theme.border_color       = GRAY(0.5f, 1.0f);
                arrow_theme.border_color_hover = GRAY(0.7f, 1.0f);
                arrow_theme.border_color_down  = GRAY(0.9f, 1.0f);
                arrow_theme.font = &game_data->font;
                
                arrow_theme.text_scale = 1.0f;
                {
                    f32 m = 4.0f;
                    f32 size = 40.0f;
                    if(do_button(&game_data->ui, input, "x", { m, screen_dim.y - size - m }, make_vec2(size), &arrow_theme, 3019283)) {
                        goto_menu(true, false);
                    }
                    do_button_toggle(&game_data->ui, input, "", &game_data->reverse_colors, make_vec2(m + size * 1.05f, screen_dim.y - size - m), make_vec2(size), &arrow_theme , 30142141);
                }
                
                arrow_theme.text_scale = 0.4f;
                f32  margin = 7.0f;
                f32  button_side = 16.0f;
                vec2 button_size = { button_side, button_side };
                vec2 position = { 
                    button_side + margin,
                    margin,
                };
                
                {
                    char text[] = "";
                    vec2 draw_p = position - make_vec2(button_side, 0.0f);
                    if(input->is_down(KB_left)) {
                        draw_button_old_style(core->renderer, text, BUTTON_down, draw_p,
                                              make_vec2(button_side), &arrow_theme);
                        game_data->ui_kb_left = false;
                    }
                    else {
                        game_data->ui_kb_left = do_button(&game_data->ui, input, text, draw_p,
                                                          button_size, &arrow_theme, { 126213 });
                    }
                }
                
                {
                    char text[] = "";
                    vec2 draw_p = position;
                    if(input->is_down(KB_down)) {
                        draw_button_old_style(core->renderer, text, BUTTON_down, draw_p,
                                              make_vec2(button_side), &arrow_theme);
                        game_data->ui_kb_down = false;
                    }
                    else {
                        game_data->ui_kb_down = do_button(&game_data->ui, input, text, draw_p, 
                                                          button_size, &arrow_theme, { 1263 });
                    }
                }
                
                {
                    char text[] = "";
                    vec2 draw_p = position + make_vec2(button_side, 0.0f);
                    if(input->is_down(KB_right)) {
                        draw_button_old_style(core->renderer, text, BUTTON_down, draw_p,
                                              make_vec2(button_side), &arrow_theme);
                        game_data->ui_kb_right = false;
                    }
                    else {
                        game_data->ui_kb_right = do_button(&game_data->ui, input, text, draw_p, button_size,
                                                           &arrow_theme, { 1321213 });
                    }
                }
                
                {
                    char text[] = "";
                    vec2 draw_p = position + make_vec2(0.0f, button_side);
                    if(input->is_down(KB_up)) {
                        draw_button_old_style(core->renderer, text, BUTTON_down, draw_p,
                                              make_vec2(button_side), &arrow_theme);
                        game_data->ui_kb_up = false;
                    }
                    else {
                        game_data->ui_kb_up = do_button(&game_data->ui, input, text, draw_p, button_size,
                                                        &arrow_theme, { 1213 });
                    }
                }
            }
            ui_end(&game_data->ui);
        }
    }
    ui_end(&game_data->ui);
}

GAME_INIT_PROC(game_init) {
    setup_globals(_core, nullptr);
    
    game_data->initialized = true;
    
    game_data->initialized = true;
    game_data->quit_game = false;
    game_data->random = 2137;
    game_data->state = STATE_menu;
    
    game_data->debug_state = false;
    
    void *permanent_memory_base = core->memory->permanent_memory.ptr  + sizeof(GameData);
    u32   permanent_memory_size = core->memory->permanent_memory.size - sizeof(GameData);
    game_data->permanent_memory = make_arena(permanent_memory_base, permanent_memory_size);
    
    void *temporary_memory_base = core->memory->temporary_memory.ptr;
    u32   temporary_memory_size = core->memory->temporary_memory.size;
    game_data->temporary_memory = make_arena(temporary_memory_base, temporary_memory_size);
    
    game_data->level_arena = make_arena(push_memory(&game_data->permanent_memory, MB(8)), MB(8));
    game_data->menu.arena  = make_arena(push_memory(&game_data->permanent_memory, MB(8)), MB(8));
    
    init_game();
    init_menu();
    init_ui(&game_data->ui, core->renderer);
    game_data->ui.width  = game_data->framebuffer.width;
    game_data->ui.height = game_data->framebuffer.height;
    
    game_data->use_free_camera = false;
    {
        PerspCamera *camera = init_persp_camera(&game_data->free_camera);
        camera->aspect = (f32)game_data->framebuffer.width / (f32)game_data->framebuffer.height;
    }
    
    delete_particles(&game_data->bg_particles, core->procs);
    PSystemParams bg_particles_params = {};
    bg_particles_params.add_texture(&game_data->sprite_particle);
    bg_particles_params.spawn_area = AREA_quad;
    bg_particles_params.sides = { 160.0f, 160.0f };
    bg_particles_params.rotated_particles = false;
    bg_particles_params.rotation.set(0.0f);
    bg_particles_params.size.set({ 0.2f, 0.2f }, { 0.4f, 0.4f });
    bg_particles_params.desired_size.set({ 0.05f, 0.05f }, { 0.0f, 0.0f});
    bg_particles_params.direction.set(0.0f, 360.0f);
    bg_particles_params.color.set(WHITE(1.0f), GRAY(0.9f, 1.0f));
    bg_particles_params.desired_color.set(WHITE(0.2f), WHITE(0.4f));
    bg_particles_params.life_time.set(2.0f, 4.0f);
    bg_particles_params.fade_in.set(0.2f, 0.4f);
    init_particles(&game_data->bg_particles, 8096, 256, bg_particles_params, core->procs);
    
    VarsFile *tweak_file = &game_data->tweak_file;
    *tweak_file = {};
    init_vars_file(DATA_DIR("p_game.tweak"), tweak_file);
    attach_var(&game_data->game_speed, VAR_f32, "game_speed", tweak_file);
    attach_var(&game_data->debug_state, VAR_bool, "debug", tweak_file);
    load_attached_vars(tweak_file, core->procs);
    
    Texture2DParams params = {
        FORMAT_rgba8,
        FORMAT_rgba,
        PIXEL_TYPE_unsigned_byte,
        FILTER_nearest,
        FILTER_nearest,
        WRAP_repeat,
        WRAP_repeat,
    };
    
    game_data->sprites = create_sprite_sheet(core->renderer, DATA_DIR("spritesheet.png"), 64, 64, &params);
    game_data->sprite_particle = create_texture_2d(core->renderer, DATA_DIR("particle.png"), &params);
    
    game_data->framebuffer_shader = create_shader(core->renderer, DATA_DIR("framebuffer.glsl"));
    game_data->level_fb = create_framebuffer(core->renderer, 1, 1);
    
    game_data->transition_t_desired = 0.0f;
    game_data->transition_t         = 1.0f;
    game_data->transition_speed     = 10.0f;
    
    game_data->paused = false;
    
    char font[256] = {};
    VarsFile ini_file;
    init_vars_file(DATA_DIR("p_game.ini"), &ini_file);
    attach_var(&font, VAR_cstr, "font", &ini_file);
    attach_var(&game_data->debug_state, VAR_bool, "debug", &ini_file);
    load_attached_vars(&ini_file, core->procs);
    
    char font_path[256] = {};
    sprintf_s(font_path, size_array(font_path), DATA_DIR("%s"), font);
    
    Texture2DParams font_params = {
        FORMAT_rgba8,
        FORMAT_rgba,
        PIXEL_TYPE_unsigned_byte,
        FILTER_nearest,
        FILTER_nearest,
        WRAP_repeat,
        WRAP_repeat,
    };
    game_data->font = create_font(core->renderer, font_path, 128.0f, &font_params);
    
    {
        LoadedSound sound = load_sound_wav(DATA_DIR("sound.wav"), core->procs);
        game_data->sound = core->procs->create_sound(&sound);
        free_sound(&sound, core->procs);
    }
    
    {
        LoadedSound sound = load_sound_wav(DATA_DIR("counter.wav"), core->procs);
        game_data->sound_counter = core->procs->create_sound(&sound);
        free_sound(&sound, core->procs);
    }
       
    transition(2.0f);
    return true;
}

GAME_FRAME_PROC(game_frame) {
    setup_globals(_core, _input);
    
    if(hotload_attached_vars(&game_data->tweak_file, core->procs)) {
        game_log("vars hotloaded... <%s>\n", game_data->tweak_file.file_path);
    }
    
    // NOTE: setup vars for frame
    game_data->temporary_memory.used = 0;
    game_data->update_dt = input->delta_time * game_data->game_speed;
    
    begin_ui_frame(&game_data->ui);
    
    bind_framebuffer(core->renderer, &game_data->framebuffer);
    bind_shader(core->renderer, &core->renderer->shader_basic->shader);
    
    clear(core->renderer, { 0.1f, 0.1f, 0.1f, 1.0f });
    if(game_data->transition_t_desired == 0.0f) {
        switch(game_data->state) {
            case STATE_menu: {
                update_and_render_menu();
            } break;
            
            case STATE_game: {
                update_and_render_game();
                
                if(input->released(KB_esc) && !game_data->gameover) {
                    goto_menu(true);
                }
            } break;
            
            case STATE_gameover: {
                update_and_render_menu();
                
                ui_begin(&game_data->ui);
                {
                    vec2 screen_dim = { (f32)game_data->ui.width, (f32)game_data->ui.height };
                    Font *font = &game_data->font;
                    LabelTheme label_theme = {};
                    label_theme.style = LABEL_STYLE_simple;
                    label_theme.color = WHITE(1.0f);
                    label_theme.bg_color = GRAY(0.6f, 0.3f);
                    label_theme.font_height = 24.0f;
                    label_theme.font = font;
                    
                    char label[1024];
                    sprintf_s(label, size_array(label), "GAME OVER\nSCORE: %d",
                              game_data->game_over_score);
                    vec2 label_size = get_text_size(label, font, label_theme.font_height, true);
                    f32 label_height = label_size.y * 1.1f; // 128.0f;
                    f32 label_width  = label_size.x * 1.05f;
                    
                    vec2 pos = make_vec2(16.0f, screen_dim.y - 64.0f);
                    
                    do_label(&game_data->ui, label, pos, { label_width, label_height }, &label_theme);
                }
                ui_end(&game_data->ui);
            } break;
            
            case STATE_win: {
                update_and_render_menu();
                
                ui_begin(&game_data->ui);
                {
                    vec2 screen_dim = { (f32)game_data->ui.width, (f32)game_data->ui.height };
                    Font *font = &game_data->font;
                    LabelTheme label_theme = {};
                    label_theme.style = LABEL_STYLE_simple;
                    label_theme.color = WHITE(1.0f);
                    label_theme.bg_color = GRAY(0.6f, 0.3f);
                    label_theme.font_height = 48.0f;
                    label_theme.font = font;
                    
                    char label[1024];
                    sprintf_s(label, size_array(label), "WIN!!!");
                    vec2 label_size = get_text_size(label, font, label_theme.font_height, true);
                    f32 label_height = label_size.y * 1.1f; // 128.0f;
                    f32 label_width  = label_size.x * 1.05f;
                    
                    vec2 pos = make_vec2(16.0f, screen_dim.y - 64.0f);
                    
                    do_label(&game_data->ui, label, pos, { label_width, label_height }, &label_theme);
                }
                ui_end(&game_data->ui);
            } break;
            
            default: assert(false);
        };
    }
    
    if(game_data->debug_state) {
        ui_begin(&game_data->ui);
        {
            Font *font = &game_data->font;
            LabelTheme label_theme = {};
            label_theme.style = LABEL_STYLE_simple;
            label_theme.color = WHITE(1.0f);
            label_theme.bg_color = GRAY(0.125f, 0.4f);
            label_theme.font_height = 14.0f;
            label_theme.font = font;
            
            vec2 screen_dim = { (f32)game_data->ui.width, (f32)game_data->ui.height };
            
            vec2 button_size = { 156.0f, 32.0f };
            
            static f32 frame_times[128];
            for(i32 i = size_array(frame_times) - 1; i >= 1; --i) {
                frame_times[i - 1] = frame_times[i];
            }
            frame_times[size_array(frame_times) - 1] = input->delta_time;
            
            f32 frame_times_combined = 0;
            for(i32 i = 0; i < size_array(frame_times); ++i) {
                frame_times_combined += frame_times[i];
            }
            
            const f32 framerate_update_time = 0.5f;
            static f32 framerate_update_counter = 0.0f;
            static i32 framerate = 0;
            
            if((framerate_update_counter += input->delta_time) >= framerate_update_time) {
                framerate_update_counter -= framerate_update_time;
                framerate = roundf32_to_i32(1.0f / (frame_times_combined / (f32)size_array(frame_times)));
            }
            
            f32 margin = 8.0f;
            char label[1024];
            sprintf_s(label, size_array(label), "level_arena: %d|%d\nmenu_arena: %d|%d\ngame_speed %.4f\nwindow_focused: %s\nstate: %s\nlast_frame_draw_calls: %d\nlast_frame_quads_drawn: %d\nlast_frame_time: %.5f\nframerate: %d", 
                      game_data->level_arena.used, game_data->level_arena.size,
                      game_data->menu.arena.used, game_data->menu.arena.size,
                      game_data->game_speed,
                      core->window->is_focused ? "true" : "false",
                      game_data->state == STATE_game ? "STATE_game" : "STATE_menu",
                      game_data->last_frame_draw_calls,
                      game_data->last_frame_quads_drawn,
                      input->delta_time, framerate);
            vec2 label_size = get_text_size(label, font, label_theme.font_height, true);
            f32 label_height = label_size.y * 1.05f; // 128.0f;
            f32 label_width  = label_size.x;
            do_label(&game_data->ui, label, make_vec2(margin, screen_dim.y - label_size.y - margin),
                     { label_width, label_height }, &label_theme);
            
            ButtonTheme theme = {};
            theme.style = BUTTON_STYLE_shadow;
            theme.shadow_offset = 3.0f;
            theme.shadow_color = { 0.2f, 0.2f, 0.2f, 1.0f };
            theme.text_scale  = 1.0f;
            theme.font = font;
            theme.bg_color       = GRAY(0.55f, 1.0f);
            theme.bg_color_hover = GRAY(0.65f, 1.0f);
            theme.bg_color_down  = GRAY(0.8f, 1.0f);
            theme.color       = BLACK(1.0f);
            theme.color_hover = BLACK(1.0f);
            theme.color_down  = BLACK(1.0f);
            
            static bool debug_buttons = false;
            
            ButtonLayout layout = setup_button_layout(LAYOUT_vertical, { game_data->ui.width - button_size.x - 10.0f, 10.0f }, 0.0f, button_size, true);
            if(debug_buttons) {
                offset_for_button_layout(&layout, 6);
                
                if(do_button(&game_data->ui, input, game_data->paused ? "unpause" : "pause", 
                             next_button_position(&layout), button_size, &theme, { 144113 })) {
                    game_data->paused = !game_data->paused;
                }
                
                if(do_button(&game_data->ui, input, "test_win", next_button_position(&layout), 
                             button_size, &theme, { 14431231 })) {
                    game_over();
                    game_won();
                }
                
                if(do_button(&game_data->ui, input, game_data->use_free_camera ? "lock" : "free", 
                             next_button_position(&layout), button_size, &theme, { 115 })) {
                    game_data->use_free_camera = !game_data->use_free_camera;
                    if(game_data->use_free_camera) {
                        // game_data->free_camera.persp.position.xy = get_desired_camera_xy(get_level(game_data->current_level));
                    }
                }
                
                if(do_button(&game_data->ui, input, "...", next_button_position(&layout), 
                             button_size, &theme, { 11633 })) {
                    game_data->reverse_colors = !game_data->reverse_colors;
                }
                
                if(do_button(&game_data->ui, input, "funny", next_button_position(&layout), 
                             button_size, &theme, { 1413 })) {
                    
                    random_seed seed = 144;
                    delete_particles(&game_data->bg_particles, core->procs);
                    PSystemParams bg_particles_params = {};
                    bg_particles_params.add_texture(&game_data->sprite_particle);
                    bg_particles_params.spawn_area = AREA_quad;
                    bg_particles_params.sides = { 60.0f, 20.0f };
                    bg_particles_params.rotated_particles = false;
                    bg_particles_params.rotation.set(0.0f);
                    bg_particles_params.size.set({ 0.2f, 0.2f }, { 0.8f, 0.8f });
                    bg_particles_params.desired_size.set({ 0.05f, 0.05f }, { 0.0f, 0.0f});
                    bg_particles_params.direction.set(0.0f, 360.0f);
                    bg_particles_params.color.set(random_color(&seed, 1.0f), random_color(&seed, 1.0f));
                    bg_particles_params.desired_color.set(random_color(&seed, 1.0f), random_color(&seed, 1.0f));
                    bg_particles_params.life_time.set(0.2f, 0.4f);
                    bg_particles_params.fade_in.set(0.2f, 0.6f);
                    init_particles(&game_data->bg_particles, 20000, 30000, bg_particles_params, core->procs);
                }
                
                offset_for_button_layout(&layout, 1);
                if(do_button(&game_data->ui, input, "debug", next_button_position(&layout),
                             button_size, &theme, { 111333 })) {
                    debug_buttons = !debug_buttons;
                }
            }
            else {
                offset_for_button_layout(&layout, 1);
                if(do_button(&game_data->ui, input, "debug", next_button_position(&layout),
                             button_size, &theme, { 111333 })) {
                    debug_buttons = !debug_buttons;
                }
            }
        }
        ui_end(&game_data->ui);
    } // NOTE: debug_state
    
    if(game_data->transition_t != game_data->transition_t_desired) {
        f32 t = input->delta_time * game_data->transition_speed;
        game_data->transition_t = lerp(game_data->transition_t, game_data->transition_t_desired, t);
        clamp_v(game_data->transition_t, 0.0f, 1.0f);
        f32 rem = game_data->transition_t_desired - game_data->transition_t;
        f32 min_rem = 0.005f;
        if(rem > -min_rem && rem < +min_rem) {
            game_data->transition_t = game_data->transition_t_desired;
        }
    }
    else {
        game_data->transition_t_desired = 0.0f;
    }
    flush(core->renderer);
    
    unbind_framebuffer(core->renderer);
    clear(core->renderer, { 0.1f, 0.1f, 0.1f, 1.0f });
    
    bind_shader(core->renderer, &game_data->framebuffer_shader->shader);
    f32 desired_factor = game_data->reverse_colors ? /* 1.0f */0.85f : 0.0f;
    game_data->reverse_factor = lerp(game_data->reverse_factor, desired_factor, 5.0f * input->delta_time);
    set_uniform_float(core->renderer, &game_data->framebuffer_shader->shader, 
                      "u_reverse_factor", game_data->reverse_factor);
    mat4x4 proj = mat4x4_orthographic(0.0f, 0.0f, (f32)core->window->width, (f32)core->window->height, -1.0f, 1.0f);
    mat4x4 view = mat4x4_identity();
    set_proj_and_view(core->renderer, proj, view);
    
    vec2 size = make_vec2((f32)core->window->width, (f32)core->window->height);
    draw_quad(core->renderer, make_vec2(0.0f), size, WHITE(1.0f), &game_data->framebuffer.color);
    // draw_quad(core->renderer, make_vec2(0.0f), size, { &game_data->sprites, 3, 3 }, WHITE(1.0f));
    flush(core->renderer);
    
    set_uniform_float(core->renderer, &game_data->framebuffer_shader->shader, "u_reverse_factor", 0.0f);
    set_proj_and_view(core->renderer, proj, view);
    draw_quad(core->renderer, make_vec2(0.0f), size, WHITE(1.0f), &game_data->ui.fb.color);
    flush(core->renderer);
    
    /* NOTE: transition */ {
        mat4x4 _proj = mat4x4_orthographic(0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f);
        mat4x4 _view = mat4x4_identity();
        vec2 _size = make_vec2(1.0f);
        set_proj_and_view(core->renderer, _proj, _view);
        bind_shader(core->renderer, &core->renderer->shader_basic->shader);
        vec4 transition_color = GRAY(0.105f, game_data->transition_t);
        if(game_data->reverse_colors) {
            transition_color = GRAY(0.695f,game_data->transition_t);
        }
        draw_quad(core->renderer, make_vec3(0.0f), _size, transition_color);
    }
    flush(core->renderer);
    
    game_data->last_frame_draw_calls = core->renderer->stats.draw_calls;
    game_data->last_frame_quads_drawn = core->renderer->stats.quad_count;
    
    return !(game_data->quit_game);
} 

GAME_SIZE_CALLBACK_PROC(size_callback) {
    setup_globals(_core, nullptr);
    
    game_data->viewport = { 0, 0, window_width, window_height };
    set_viewport(core->renderer, game_data->viewport);
    
    if(window_width > 0 && window_height > 0) {
        resize_framebuffer(core->renderer, &game_data->framebuffer, (u32)window_width , (u32)window_height);
    }
    
    set_ui_dims(&game_data->ui, window_width, window_height);
}

GAME_HOTLOAD_CALLBACK_PROC(hotload_callback) {
    setup_globals(_core, nullptr);
    
    // transition(10.0f);
    
    game_log("game code hotloaded...\n");
}

GAME_GET_STARTUP_PARAMS_PROC(get_startup_params) {
    char title[256] = "p_game";
    i32 framerate   = 0;
    bool fullscreen = false;
    vec2 resolution = { 0, 0 };
    
    VarsFile ini_file;
    init_vars_file(DATA_DIR("p_game.ini"), &ini_file);
    attach_var(&title,      VAR_cstr, "title",      &ini_file);
    attach_var(&framerate,  VAR_i32,  "framerate",  &ini_file);
    attach_var(&fullscreen, VAR_bool, "fullscreen", &ini_file);
    attach_var(&resolution, VAR_vec2, "resolution", &ini_file);
    load_attached_vars(&ini_file, procs);
    
    WindowCreationParams window = default_window_params();
    window.desired_fps = framerate;
    window.cap_framerate = !(framerate == 0);
    window.resizable = false;
    window.fullscreen = fullscreen;
    window.x_fullscreen = (i32)resolution.x;
    window.y_fullscreen = (i32)resolution.y;
    copy_memory(title, window.title, min_value(size_array(title), size_array(window.title)));
    MemoryAllocationParams memory = default_memory_params();
    memory.permanent_memory_size = MB(32);
    memory.temporary_memory_size = MB(8);
    return {
        window,
        memory
    };
}