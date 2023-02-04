static void 
init_particles(PSystem *particles, u32 max, u32 sps, 
               PSystemParams params, PlatformProcs *procs) {
    *particles = {};
    
    particles->initialized = true;
    particles->random = 2137;
    particles->spawns_per_second = sps;
    particles->seconds_per_spawn = 1.0f / (f32)sps;
    particles->particle_count = max;
    particles->particles = (Particle *)procs->alloc(max * sizeof(Particle));
    zero_memory(particles->particles, max * sizeof(Particle));
    particles->params = params;
}

static void 
update_and_render_particles(PSystem *particles, Renderer *renderer, f32 dt) {
    if(!particles->initialized || !particles->particles) {
        return;
    }
    
    particles->spawn_counter += dt;
    while(particles->spawn_counter >= particles->seconds_per_spawn) {
        particles->spawn_counter -= particles->seconds_per_spawn;
        emit_next_particle(particles);
    }
    
    for(u32 i = 0; i < particles->particle_count; ++i) {
        Particle *p = particles->particles + i;
        if(!p->is_alive) {
            continue;
        }
        
        if((p->life_time_counter += dt) >= p->life_time) {
            p->is_alive = false;
            continue;
        }
        f32 life_time_perc = p->life_time_counter / p->life_time;
        
        vec2 size = vec_lerp(p->size, p->desired_size, life_time_perc);
        vec4 color = vec_lerp(p->color, p->desired_color, life_time_perc);
        if(p->fade_in > 0.0f) {
            color.a *= min_value(life_time_perc / p->fade_in, 1.0f);
        }
        
        f32 move_speed = lerp(p->move_speed, p->desired_move_speed, life_time_perc);
        p->position += vec_from_angle(deg_to_rad(p->direction)) * move_speed * dt;
        p->rotation += p->rotation_speed * dt;
        
        vec3 particle_position = make_vec3(p->position + particles->position.xy - (size * 0.5f), particles->position.z);
        if(particles->params.rotated_particles) {
            draw_quad_rotated(renderer, particle_position, size, deg_to_rad(p->rotation), color, p->tex);
        }
        else {
            draw_quad(renderer, particle_position, size, color, p->tex);
        }
    }
}

static void
draw_psystem_border(PSystem *particles, Renderer *renderer) {
    if(particles->params.spawn_area == AREA_quad) {
        vec3 position = make_vec3(particles->position.xy - particles->params.sides * 0.5f, particles->position.z);
        draw_quad_outline(renderer, position, particles->params.sides, 0.5f, CYAN(0.6f));
    }
}

static void 
delete_particles(PSystem *particles, PlatformProcs *procs) {
    if(particles->particles) { 
        procs->free(particles->particles); 
    }
    particles->particles = nullptr;
    particles->particle_count = 0;
}

static void 
emit_next_particle(PSystem *particles) {
    PSystemParams *params = &particles->params;
    
#define next_random_01 (rand_i32_in_range(&particles->random, 0, 10000) * 0.0001f)
    Particle *p = nullptr;
    for(u32 i = 0; i < particles->particle_count; ++i) {
        p = particles->particles + i;
        if(!p->is_alive) {
            p->is_alive = true;
            
            switch(params->spawn_area) {
                case AREA_quad: {
                    vec2 perc_t = {
                        next_random_01,
                        next_random_01
                    };
                    vec2 offset = particles->rel_position.xy - params->sides * 0.5f;
                    p->position = offset + (perc_t * params->sides);
                } break;
                
                case AREA_circle: {
                    vec2 offset = particles->position.xy;
                    vec2 dir = vec_from_angle(next_random_01 * (PI32 * 2.0f));
                    p->position = offset - dir * (next_random_01 * params->radius) * 0.5f;
                } break;
                
                default: {
                    p->position = {};
                };
            }
            
#define random_value_f32(value,pair)\
if(pair.initialized) {\
if(pair.value1_active) {\
f32 t = next_random_01;\
value = lerp(pair.value0, pair.value1, t);\
}\
else {\
value = pair.value0;\
}\
}
            
#define random_value_vec(value,pair)\
if(pair.initialized) {\
if(pair.value1_active) {\
f32 t = next_random_01;\
value = vec_lerp(pair.value0, pair.value1, t);\
}\
else {\
value = pair.value0;\
}\
}
            
            random_value_vec(p->size, params->size);         
            random_value_vec(p->desired_size, params->desired_size);
            random_value_f32(p->rotation, params->rotation);
            random_value_f32(p->rotation_speed, params->rotation_speed);
            random_value_f32(p->direction, params->direction);
            random_value_f32(p->move_speed, params->move_speed);
            random_value_f32(p->desired_move_speed, params->desired_move_speed);
            random_value_vec(p->color, params->color);
            random_value_vec(p->desired_color, params->desired_color);
            random_value_f32(p->life_time, params->life_time);
            random_value_f32(p->fade_in, params->fade_in);
            
            p->life_time_counter = 0.0f;
            if(!params->desired_size.initialized) { 
                p->desired_size = p->size; 
            }
            if(!params->desired_color.initialized) { 
                p->desired_color = p->color; 
            }
            if(!params->desired_move_speed.initialized) { 
                p->desired_move_speed = p->move_speed; 
            }
            
            if(params->texture_count) {
                if(params->texture_count == 1) {
                    p->tex = params->textures[0];
                }
                else {
                    p->tex = params->textures[rand_u32_in_range(&particles->random, 0, params->texture_count - 1)];
                }
            }
            else {
                p->tex = nullptr;
            }
            
            return;
        }
    }
#undef random_value_f32
#undef random_value_vec
#undef next_random_01
}
