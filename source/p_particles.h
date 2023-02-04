#ifndef P_PARTICLES_H
#define P_PARTICLES_H

struct Particle {
    bool is_alive;
    Texture2D *tex;
    vec2 position;
    vec2 size;
    vec2 desired_size;
    f32 rotation;
    f32 rotation_speed;
    f32 direction;
    f32 move_speed;
    f32 desired_move_speed;
    f32 life_time;
    f32 life_time_counter;
    f32 fade_in;
    vec4 color;
    vec4 desired_color;
};

enum particle_spawn_area {
    AREA_quad,
    AREA_circle,
};

struct PSystemParams {
    particle_spawn_area spawn_area;
    union {
        struct { // NOTE: AREA_quad
            vec2 sides; 
        }; 
        struct {  // NOTE: AREA_circle
            f32 radius; 
        };
    };
    
    template <typename T> struct PSystemParam {
        bool initialized;
        T value0;
        T value1;
        bool value1_active;
        
        void set(T v0, T v1) {
            this->initialized = true;
            this->value0 = v0;
            this->value1 = v1;
            this->value1_active = true;
        }
        
        
        void set(T v0) {
            this->initialized = true;
            this->value0 = v0;
            this->value1_active = false;
        }
    };
    
    u32 texture_count;
    Texture2D *textures[4];
    
    void add_texture(Texture2D *tex) {
        textures[texture_count++] = tex;
    }
    
    void no_textures(void) {
        texture_count = 0;
    }
    
    bool rotated_particles;
    PSystemParam<vec2> size;
    PSystemParam<vec2> desired_size;
    PSystemParam<f32>  rotation;
    PSystemParam<f32>  rotation_speed;
    PSystemParam<f32>  move_speed;
    PSystemParam<f32>  desired_move_speed;
    PSystemParam<f32>  direction;
    PSystemParam<vec4> color;
    PSystemParam<vec4> desired_color;
    PSystemParam<f32>  life_time;
    PSystemParam<f32>  fade_in;
};

inline PSystemParams 
default_psystem_params(void) {
    PSystemParams params = {};
    params.spawn_area = AREA_quad;
    params.sides = {1.0f, 1.0f};
    params.size.set({0.1f, 0.1f}, {0.4f, 0.4f});
    params.move_speed.set(1.0f);
    params.direction.set(0.0f, 360.0f);
    params.color.set(RED(1.0f));
    params.life_time.set(0.1f);
    return params;
}

struct PSystem {
    bool initialized;
    random_seed random;
    
    vec3 rel_position;
    vec3 position;
    
    f32 spawn_counter;
    u32 spawns_per_second;
    f32 seconds_per_spawn;
    
    u32 particle_count;
    Particle *particles;
    PSystemParams params;
};

static void init_particles(PSystem *particles, u32 max, u32 sps, PSystemParams params, PlatformProcs *procs);
static void update_and_render_particles(PSystem *particles, Renderer *renderer, f32 dt);
static void draw_psystem_border(PSystem *particles, Renderer *renderer);
static void delete_particles(PSystem *particles, PlatformProcs *procs);
static void emit_next_particle(PSystem *particles);

#endif /* P_PARTICLES_H */
