inline OrthoCamera *
init_ortho_camera(Camera *camera) {
    zero_struct(camera);
    camera->proj_type = PROJECTION_orthographic;
    
    OrthoCamera *ortho = &camera->ortho;
    ortho->left     = -10;
    ortho->bottom   = -10;
    ortho->right    = 10;
    ortho->top      = 10;
    ortho->near     = -1.0f;
    ortho->far      = +1.0f;
    ortho->scale    = 1.0f;
    ortho->rotation = 0.0f;
    ortho->position = {};
    return ortho;
}

inline PerspCamera *
init_persp_camera(Camera *camera) {
    zero_struct(camera);   
    camera->proj_type = PROJECTION_perspective;
    
    PerspCamera *persp = &camera->persp;
    persp->fov  = 45.0f;
    persp->near = 0.1f;
    persp->far  = 1000.0f;
    persp->position  = { 0.0f, 0.0f, 10.0f };
    persp->up_vector = { 0.0f, 1.0f, 0.0f };
    persp->focus_on_point = false;
    persp->horiz_rotation = -1.570796f;
    persp->vert_rotation  = +1.570796f;
    return persp;
}

static mat4x4 
camera_proj(Camera *camera) {
    mat4x4 projection = mat4x4_identity();
    switch(camera->proj_type) {
        case PROJECTION_perspective: {
            PerspCamera *persp = &camera->persp;
            projection = mat4x4_perspective(persp->fov, persp->aspect, 
                                            persp->near, persp->far);
        } break;
        
        case PROJECTION_orthographic: {
            OrthoCamera *ortho = &camera->ortho;
            projection = mat4x4_orthographic(ortho->left, ortho->bottom, 
                                             ortho->right, ortho->top, 
                                             ortho->near, ortho->far);
        } break;
        
        default: { assert(0); }
    }
    return projection;
}

static mat4x4 
camera_view(Camera *camera) {
    mat4x4 view = mat4x4_identity();
    switch(camera->proj_type) {
        case PROJECTION_perspective: {
            PerspCamera *persp = &camera->persp;
            vec3 camera_to = persp->focus_point;
            if(!persp->focus_on_point) {
                vec3 camera_dir = vec_from_spherical(persp->horiz_rotation, persp->vert_rotation);
                camera_to = persp->position + camera_dir;
            }
            view = mat4x4_look_at(persp->position, camera_to, persp->up_vector);
        } break;
        
        case PROJECTION_orthographic: {
            OrthoCamera *ortho = &camera->ortho;
            
            view = mat4x4_translate(-ortho->position.x, -ortho->position.y, 0.0f);
            view *= mat4x4_scale(ortho->scale, ortho->scale, 1.0f);
            view *= mat4x4_zaxis_rotate(ortho->rotation);
        } break;
        
        default: assert(false);
    }
    return view;
}

static void 
rotate_persp_camera(PerspCamera *camera, vec2 move_vector, f32 speed) {
    if(!camera->focus_on_point) {
        camera->horiz_rotation += move_vector.x * speed;
        camera->vert_rotation  -= move_vector.y * speed;
        clamp_v(camera->vert_rotation, 0.1f, PI32 * 0.9f);
    }
}

static vec2 // NOTE: temp
ortho_proj_viewport_p(vec2 vp_p, recti32 viewport, OrthoCamera *camera) {
    vec2 camera_dim = {
        camera->right- camera->left, 
        camera->top - camera->bottom
    };
    vec2 cursor_perc = {
        (vp_p.x - viewport.x) / (viewport.x + viewport.width),
        (vp_p.y - viewport.y) / (viewport.y + viewport.height)
    };
    vec2 ortho_cursor_p = {
        (cursor_perc.x * camera_dim.x + camera->left) / camera->scale + camera->position.x,
        (cursor_perc.y * camera_dim.y + camera->bottom) / camera->scale + camera->position.y
    };
    ortho_cursor_p = vec_rotate_around_p(ortho_cursor_p, camera->rotation, camera->position);
    return ortho_cursor_p;
}

static bool 
get_shader_source_info(ShaderSource *src, char *file_data, size_t file_len) {
    src->vertex = nullptr;
    src->fragment = nullptr;
    src->vertex_len = 0;
    src->fragment_len = 0;
    
    char vertex_token[] = "@vertex";
    char fragment_token[] = "@fragment";
    
    char *base_byte = (char *)file_data;
    char *file_eof_byte = base_byte + file_len;
    u32 counter = 0;
    for(char *byte = base_byte; byte < file_eof_byte; ++byte) {
        if(!src->vertex) {
            if(*byte == vertex_token[counter]) {
                ++counter;
                if(counter == (size_array(vertex_token) - 1)) {
                    ++byte;
                    counter = 0;
                    src->vertex = byte;
                }
            }
        }
        else {
            if(*byte == fragment_token[counter]) {
                ++counter;
                if(counter == (size_array(fragment_token) - 1)) {
                    ++byte;
                    counter = 0;
                    src->fragment = byte;
                    src->fragment_len = -((i32)(byte - file_eof_byte));
                    src->vertex_len = file_eof_byte - src->vertex - src->fragment_len - size_array(fragment_token) - 1;
                }
            }
            else { counter = 0; }
        }
    }
    return (src->vertex && src->fragment && src->vertex_len && src->fragment_len);
}

static vec2_4x 
get_tex_coords(SpriteSheet *ss, u32 x, u32 y) {
    if(x >= ss->x_sprites || x < 0) {
        x = 0;
    }
    if(y >= ss->y_sprites || y < 0) {
        y = 0;
    }
    
    f32 padding = 1.0f;
    vec2_4x tex_coords;
    tex_coords.vecs[0] = {
        ((f32)(x * ss->x_pixels_per_sprite) + padding) / (f32)ss->ss_width,
        ((f32)(y * ss->y_pixels_per_sprite) + padding) / (f32)ss->ss_height,
    };
    tex_coords.vecs[1] = {
        ((f32)((x + 1) * ss->x_pixels_per_sprite) - padding) / (f32)ss->ss_width,
        ((f32)(y * ss->y_pixels_per_sprite)       + padding) / (f32)ss->ss_height,
    };
    tex_coords.vecs[2] = {
        ((f32)((x + 1) * ss->x_pixels_per_sprite) - padding) / (f32)ss->ss_width,
        ((f32)((y + 1) * ss->y_pixels_per_sprite) - padding) / (f32)ss->ss_height,
    };
    tex_coords.vecs[3] = {
        ((f32)(x * ss->x_pixels_per_sprite)       + padding) / (f32)ss->ss_width,
        ((f32)((y + 1) * ss->y_pixels_per_sprite) - padding) / (f32)ss->ss_height,
    };
    
    return tex_coords;
}

// NOTE: Renderer

static void 
init_renderer(Renderer *r, RendererAPI *api, PlatformProcs *platform_procs) {
    r->api = *api;
    
    r->api.initialize();
    r->procs = platform_procs;
    
    r->shaders_hotload_counter = 0.0f;
    r->shader_count = 0;
    // zero_array(r->shaders, ShaderRef);
    
    r->quad_count = 0;
    r->texture_count = 0;
    
    // NOTE: create white texture
    {
        const i32 width = 1;
        const i32 height = 1;
        const i32 channels = 4;
        const i32 mem_size = width * height * channels * sizeof(u8);
        u8 *data = (u8 *)r->procs->alloc(mem_size);
        u8 *data_ptr = data;
        for(i32 y = 0; y < height; ++y) {
            for(i32 x = 0; x < width; ++x) {
                for(i32 c = 0; c < channels; ++c) {
                    *data_ptr++ = 255;
                }
            }
        }
        r->white_texture = create_texture_2d(r, data, width, height, channels);
        r->procs->free(data);
    }
    
    char shader_path[] = DATA_DIR("p_basic.glsl");
    r->shader_basic = create_shader(r, shader_path);
    
    // r->def_font = create_font(r, DATA_DIR("joystix.ttf"), 128);
    // r->def_font = create_font(r, DATA_DIR("KarminaBoldItalic.ttf"), 128);
    // r->def_font = create_font(r, DATA_DIR("arial.ttf"), 128);
    
    set_batch_params(r, 6000, 16);
}

static void 
hotload_shaders(Renderer *r, f32 dt) {
    bool hotloaded = false;
    const f32 wait_span_in_seconds = 0.1f;
    if((r->shaders_hotload_counter += dt) >= wait_span_in_seconds) {
        r->shaders_hotload_counter -= wait_span_in_seconds;
        for(u32 i = 0; i < r->shader_count; ++i) {
            ShaderRef *ref = &r->shaders[i];
            if(ref->loaded_from_file) {
                PTime last_write_time = r->procs->last_write_time(ref->path);
                if(!p_time_cmp(&ref->last_write_time, &last_write_time, false)) {
                    delete_shader_program(r, &ref->shader);
                    ref->shader = create_shader_program(r, ref->path);
                    printf("shader <%s> hotloaded... id(%lu)\n", ref->path, ref->shader.id);
                    ref->last_write_time = last_write_time;
                    hotloaded = true;
                }
            }
        }
    }
    
    // NOTE: reset texture ids and stuff
    if(hotloaded) {
        set_batch_params(r, r->max_quads, r->max_texture_slots);
    }
}

static void
begin_renderer_frame(Renderer *r) {
    r->stats.quad_count = 0;
    r->stats.draw_calls = 0;
}

static void 
push(Renderer *r) {
    ASSERT((r->stack_id + 1) <= RENDERER_STACK_SIZE, "");
    
    r->stack_vp[r->stack_id] = r->viewport;
    r->stack_fb[r->stack_id] = r->bound_framebuffer;
    r->stack_scene[r->stack_id] = r->scene_data;
    r->stack_shader[r->stack_id] = r->bound_shader;
    r->stack_id += 1;
}

static void 
pop(Renderer *r) {
    r->stack_id -= 1;
    set_viewport(r, r->stack_vp[r->stack_id]);
    bind_framebuffer(r, r->stack_fb[r->stack_id]);
    set_scene_data(r, r->stack_scene[r->stack_id]);
    bind_shader(r, r->stack_shader[r->stack_id]);
}

static void 
set_batch_params(Renderer *r, u32 new_max_quads, u32 new_max_texture_slots) {
    flush(r);
    
    r->max_quads = new_max_quads;
    r->max_quad_verts = new_max_quads * 4;
    r->max_quad_indices = new_max_quads * 6;
    r->max_texture_slots = new_max_texture_slots;
    
    if(r->quad_vb_data) { r->procs->free(r->quad_vb_data); }
    r->quad_vb_data = (QuadVertex *)r->procs->alloc(r->max_quad_verts * sizeof(QuadVertex));
    
    if(r->quad_indices) { r->procs->free(r->quad_indices); }
    r->quad_indices = (u32 *)r->procs->alloc(r->max_quad_indices * sizeof(u32));
    u32 verts = 0;
    for(u32 i = 0; i < r->max_quad_indices; i += 6) {
        u32 j = i;
        r->quad_indices[j++] = verts + 0;
        r->quad_indices[j++] = verts + 1;
        r->quad_indices[j++] = verts + 2;
        
        r->quad_indices[j++] = verts + 2;
        r->quad_indices[j++] = verts + 3;
        r->quad_indices[j++] = verts + 0;
        verts += 4;
    }
    
    for(u32 i = 0; i < size_array(r->shaders); ++i) {
        ShaderProgram *shader = &r->shaders[i].shader;
        bind_shader(r, shader);
        if(r->texture_slots) { r->procs->free(r->texture_slots); }
        r->texture_slots = (u32 *)r->procs->alloc(r->max_texture_slots * sizeof(u32));
        u32 slots[256] = {};
        for(u32 j = 0; j < r->max_texture_slots; ++j) {
            slots[j] = j;
        }
        set_uniform_int_array(r, shader, "u_textures", (i32 *)slots, r->max_texture_slots);
    }
    
    if(r->quad_vb.id) { r->api.delete_vertex_buffer(&r->quad_vb); }
    if(r->quad_ib.id) { r->api.delete_index_buffer(&r->quad_ib); }
    if(r->quad_va.id) { r->api.delete_vertex_array(&r->quad_va); }
    
    r->quad_va = r->api.create_vertex_array();
    r->quad_vb = r->api.create_vertex_buffer(nullptr, r->max_quad_verts * sizeof(QuadVertex), VB_dynamic);
    r->quad_vb.layout.push(3, LAYOUT_float32, "position");
    r->quad_vb.layout.push(4, LAYOUT_float32, "color");
    r->quad_vb.layout.push(2, LAYOUT_float32, "tex_coord");
    r->quad_vb.layout.push(1, LAYOUT_float32, "tex_slot");
    r->quad_vb.layout.push(2, LAYOUT_float32, "tiling_factor");
    r->api.attach_vertex_buffer(&r->quad_va, &r->quad_vb);
    r->quad_ib = r->api.create_index_buffer(r->quad_indices, r->max_quad_indices);
    r->api.attach_index_buffer(&r->quad_va, &r->quad_ib);
}

static void 
set_viewport(Renderer *r, recti32 vp) {
    r->api.set_viewport(vp.x, vp.y, vp.width, vp.height);
    r->viewport = vp;
}

static void
set_proj_and_view(Renderer *r, mat4x4 proj, mat4x4 view) {
    SceneData *scene = &r->scene_data;
    scene->proj = proj;
    scene->view = view;
}

static void 
set_camera(Renderer *r, Camera *camera) {
    SceneData *scene = &r->scene_data;
    scene->proj = camera_proj(camera);
    scene->view = camera_view(camera);
}

static void
set_scene_data(Renderer *r, SceneData data) {
    r->scene_data = data;
}

static void 
set_clip_rect(Renderer *r, recti32 clip_rect) {
    // flush(r);
    r->clipping_rect = true;
    r->clip_rect = clip_rect;
    r->api.set_clip_rect(clip_rect.x, clip_rect.y, clip_rect.width, clip_rect.height);
}

static void 
disable_clip_rect(Renderer *r) {
    if(!r->clipping_rect) {
        return;
    }
    // flush(r);
    r->api.disable_clip_rect();
}

static void 
clear(Renderer *r, vec4 color) {
    r->api.clear(color.r, color.g, color.b, color.a);
}

static void 
flush(Renderer *r) {
    if(r->quad_count == 0) {
        return;
    }
    
    ShaderProgram *shader = r->bound_shader; //_basic->shader;
    // bind_shader(r, shader);
    for(u32 i = 0; i < r->texture_count; ++i) {
        bind_texture_2d(r, r->texture_slots[i], i);
    }
    set_uniform_mat4x4(r, shader, "u_proj", r->scene_data.proj);
    set_uniform_mat4x4(r, shader, "u_view", r->scene_data.view);
    
    u32 quad_verts_count = r->quad_count * 4;
    u32 quad_index_count = r->quad_count * 6;
    u32 size = quad_verts_count * sizeof(QuadVertex);
    r->api.set_vertex_buffer_data(&r->quad_vb, r->quad_vb_data, size, 0);
    r->api.draw_indexed(&r->quad_va, quad_index_count);
    
    r->stats.draw_calls += 1;
    r->stats.quad_count += r->quad_count;
    
    r->quad_count = 0;
    r->texture_count = 0;
}

static u32
bind_next_batch_texture_slot(Renderer *r, Texture2D *tex) {
    // NOTE: if nullptr then return 1x1 white texture
    if(tex == nullptr) {
        return bind_next_batch_texture_slot(r, &r->white_texture);
    }
    
    u32 slot = 0;
    bool found = false;
    for(u32 i = 0; i < r->texture_count; ++i) {
        if(r->texture_slots[i] == tex->id) {
            found = true;
            slot = i;
            break;
        }
    }
    
    if(!found) {
        assert((r->texture_count + 1) <= r->max_texture_slots);
        slot = r->texture_count;
        r->texture_slots[slot] = tex->id;
        r->texture_count++;
    }
    return slot;
}

static void 
draw_quad_base(Renderer *r, vec3 positions[4], vec2 tex_coords[4], 
               vec4 color, Texture2D *tex, vec2 tiling_factor) {
    if(r->quad_count >= r->max_quads
       || r->texture_count >= r->max_texture_slots) {
        flush(r);
    }
    f32 slot = (f32)bind_next_batch_texture_slot(r, tex);
    
    QuadVertex *vertex = &r->quad_vb_data[r->quad_count * 4];
    for(u32 i = 0; i < 4; ++i) {
        vertex->position = positions[i];
        vertex->color = color;
        vertex->tex_coord = tex_coords[i];
        vertex->tex_slot = slot;
        vertex->tiling_factor = tiling_factor;
        vertex++;
    }
    r->quad_count += 1;
}

static void
draw_quad_ex(Renderer *r, vec3 position, vec2 size, QuadExParams *params) {
    f32 half_size_x = size.x * 0.5f;
    f32 half_size_y = size.y * 0.5f;
    vec3 positions[4] = {
        {-half_size_x, -half_size_y, 0.0f},
        {+half_size_x, -half_size_y, 0.0f},
        {+half_size_x, +half_size_y, 0.0f},
        {-half_size_x, +half_size_y, 0.0f},
    };
    
    vec2 tex_coords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    
    if(params->spritesheet) {
        vec2_4x _tex_coords = get_tex_coords(params->ss_tile.ss, params->ss_tile.x, params->ss_tile.y);
#if 0
        copy_memory(_tex_coords.vecs, tex_coords, size_array(tex_coords) * sizeof(vec2));
#else
        tex_coords[0] = _tex_coords.vecs[0];
        tex_coords[1] = _tex_coords.vecs[1];
        tex_coords[2] = _tex_coords.vecs[2];
        tex_coords[3] = _tex_coords.vecs[3];
#endif
    }
    
    f32 rotate_x = params->flip_y ? PI32 : 0.0f;
    f32 rotate_y = params->flip_x ? PI32 : 0.0f;
    f32 rotate_z = params->rotation;
    
    mat4x4 translate = mat4x4_translate(position.x + half_size_x, position.y + half_size_y, position.z);
    mat4x4 rotate    = mat4x4_rotate(rotate_x, rotate_y, rotate_z);
    mat4x4 transform = rotate * translate;
    
    for(u32 i = 0; i < 4; ++i) {
        positions[i] = transform * positions[i];
    }
    
    Texture2D *texture = params->spritesheet ? &params->ss_tile.ss->tex : params->texture;
    draw_quad_base(r, positions, tex_coords, params->color, texture, params->tiling_factor);
}

static void
draw_quad_ex(Renderer *r, vec2 position, vec2 size, QuadExParams *params) {
    draw_quad_ex(r, { position.x, position.y, 0.0f }, size, params);
}

static void 
draw_quad(Renderer *r, vec3 positions[4], vec4 color, Texture2D *tex, vec2 tiling_factor) {
    vec2 tex_coords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    draw_quad_base(r, positions, tex_coords, color, tex, tiling_factor);
}

static void 
draw_quad(Renderer *r, vec2 positions[4], vec4 color, Texture2D *tex, vec2 tiling_factor) {
    vec3 _positions[4] = {
        make_vec3(positions[0], 0.0f),
        make_vec3(positions[1], 0.0f),
        make_vec3(positions[2], 0.0f),
        make_vec3(positions[3], 0.0f)
    };
    draw_quad(r, _positions, color, tex, tiling_factor);
}

static void 
draw_quad(Renderer *r, vec3 position, vec2 size, vec4 color, Texture2D *tex, vec2 tiling_factor) {
    vec3 positions[4] = {
        position,
        position + make_vec3(size.x, 0.0f, 0.0f),
        position + make_vec3(size.x, size.y, 0.0f),
        position + make_vec3(0.0f, size.y, 0.0f)
    };
    vec2 tex_coords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    draw_quad_base(r, positions, tex_coords, color, tex, tiling_factor);   
}

static void 
draw_quad(Renderer *r, vec2 position, vec2 size, vec4 color, Texture2D *tex, vec2 tiling_factor) {
    draw_quad(r, {position.x, position.y, 0.0f}, size, color, tex, tiling_factor);
}

static void 
draw_quad(Renderer *r, vec3 position, vec2 size, SpriteSheetTile ss_tile, 
          vec4 color, vec2 tiling_factor) {
    assert(ss_tile.ss);   
    vec3 positions[4] = {
        position,
        position + make_vec3(size.x, 0.0f, 0.0f),
        position + make_vec3(size.x, size.y, 0.0f),
        position + make_vec3(0.0f, size.y, 0.0f)
    };
    
    vec2_4x tex_coords = get_tex_coords(ss_tile.ss, ss_tile.x, ss_tile.y);
    draw_quad_base(r, positions, tex_coords.vecs, color, &ss_tile.ss->tex, tiling_factor);   
}

static void 
draw_quad(Renderer *r, vec2 position, vec2 size, SpriteSheetTile ss_tile, 
          vec4 color, vec2 tiling_factor) {
    draw_quad(r, {position.x, position.y, 0.0f}, size, ss_tile, color, tiling_factor);
}

static void 
draw_quad_rotated(Renderer *r, vec3 position, vec2 size, f32 rotation, 
                  vec4 color, Texture2D *tex, vec2 tiling_factor) {
    f32 half_size_x = size.x * 0.5f;
    f32 half_size_y = size.y * 0.5f;
    vec3 positions[4] = {
        {-half_size_x, -half_size_y, 0.0f},
        {+half_size_x, -half_size_y, 0.0f},
        {+half_size_x, +half_size_y, 0.0f},
        {-half_size_x, +half_size_y, 0.0f},
    };
    vec2 tex_coords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    
    mat4x4 translate = mat4x4_translate(position.x + half_size_x, position.y + half_size_y, position.z);
    mat4x4 rotate = mat4x4_zaxis_rotate(rotation);
    mat4x4 transform = rotate * translate;
    
    for(u32 i = 0; i < 4; ++i) {
        positions[i] = transform * positions[i];
    }
    draw_quad_base(r, positions, tex_coords, color, tex, tiling_factor);
}

static void 
draw_quad_rotated(Renderer *r, vec2 position, vec2 size, f32 rotation, 
                  vec4 color, Texture2D *tex, vec2 tiling_factor) {
    draw_quad_rotated(r, {position.x, position.y, 0.0f}, size, rotation, color, tex, tiling_factor);
}

static void 
draw_quad_rotated(Renderer *r, vec3 position, vec2 size, f32 rotation, 
                  SpriteSheetTile ss_tile, vec4 color) {
    assert(ss_tile.ss); // TODO:
    f32 half_size_x = size.x * 0.5f;
    f32 half_size_y = size.y * 0.5f;
    vec3 positions[4] = {
        {-half_size_x, -half_size_y, 0.0f},
        {+half_size_x, -half_size_y, 0.0f},
        {+half_size_x, +half_size_y, 0.0f},
        {-half_size_x, +half_size_y, 0.0f},
    };
    
    mat4x4 translate = mat4x4_translate(position.x + half_size_x, position.y + half_size_y, position.z);
    mat4x4 rotate = mat4x4_zaxis_rotate(rotation);
    mat4x4 transform = rotate * translate;
    
    for(u32 i = 0; i < 4; ++i) { positions[i] = transform * positions[i]; }
    
    vec2_4x tex_coords = get_tex_coords(ss_tile.ss, ss_tile.x, ss_tile.y);
    draw_quad_base(r, positions, tex_coords.vecs, color, &ss_tile.ss->tex, {1.0f, 1.0f});
}

static void 
draw_quad_rotated(Renderer *r, vec2 position, vec2 size, f32 rotation, 
                  SpriteSheetTile ss_tile, vec4 color) {
    draw_quad_rotated(r, {position.x, position.y, 0.0f}, size, rotation, ss_tile, color);
}

static void 
draw_quad_outline(Renderer *r, vec3 position, vec2 size, f32 width, vec4 color) {
    // NOTE: left
    vec3 rect0[4] = {
        { position.x,         position.y + width,          position.z },
        { position.x + width, position.y + width,          position.z },
        { position.x + width, position.y + size.y - width, position.z },
        { position.x,         position.y + size.y - width, position.z },
    };
    draw_quad(r, rect0, color);
    
    // NOTE: right
    vec3 rect1[4] = {
        { position.x + size.x,         position.y + width,          position.z },
        { position.x + size.x - width, position.y + width,          position.z },
        { position.x + size.x - width, position.y + size.y - width, position.z },
        { position.x + size.x,         position.y + size.y - width, position.z },
    };
    draw_quad(r, rect1, color);
    
    // NOTE: bottom
    vec3 rect2[4] = {
        { position.x,          position.y,         position.z },
        { position.x + size.x, position.y,         position.z },
        { position.x + size.x, position.y + width, position.z },
        { position.x,          position.y + width, position.z },
    };
    draw_quad(r, rect2, color);
    
    // NOTE: top
    vec3 rect3[4] = {
        { position.x,          position.y + size.y,         position.z },
        { position.x + size.x, position.y + size.y,         position.z },
        { position.x + size.x, position.y + size.y - width, position.z },
        { position.x,          position.y + size.y - width, position.z },
    };
    draw_quad(r, rect3, color);
}

static void 
draw_quad_outline(Renderer *r, vec2 position, vec2 size, f32 width, vec4 color) {
    draw_quad_outline(r, { position.x, position.y, 0.0f }, size, width, color);
}

static void 
draw_text(Renderer *r, char *buffer, vec3 position, f32 line_height, 
          Font *loaded_font, vec4 color, bool break_lines) {
    Font *font = loaded_font;
    if(!font) {
        return;
    }
    if(!font->valid) {
        return;
    }
    
    f32 scale = line_height / font->height;
    f32 space = scale * font->height;
    
    f32 x_cursor = position.x;
    f32 y_cursor = position.y;
    char *text_cursor = buffer;
    while(true) {
        if(*text_cursor == '\0') {
            break;
        }
        if(*text_cursor == '\n') {
            if(break_lines) {
                y_cursor -= line_height;
                x_cursor = position.x;
            }
            ++text_cursor;
            continue;
        }
        
        Glyph *glyph = &font->glyphs[*text_cursor];
        if(!glyph) {
            continue;
        }
        
        vec2 draw_size = make_vec2(glyph->tex.width * scale,
                                   glyph->tex.height * scale);
        
        vec3 draw_p = {x_cursor, y_cursor, position.z};
        // draw_p.y = position.y + draw_size.y * 0.5f;
        draw_p.y -= draw_size.y;
        draw_p.y -= glyph->y_offset * scale;
        draw_p.x += ((f32)glyph->left_side_bearing * font->scale_factor) * scale;
        // draw_p.x += draw_size.x * 0.5f;
        
        draw_quad(r, draw_p, draw_size, color, &glyph->tex);
        // r->quad(draw_p, draw_size, color, 0.0f);
        
        x_cursor += (glyph->advance * font->scale_factor) * scale;
        if(text_cursor + 1) {
            i32 kern = get_kerning_advance(font, *text_cursor, *(text_cursor + 1));
            x_cursor += ((f32)kern * font->scale_factor) * scale;
        }
        ++text_cursor;
    }
}

static void 
draw_text(Renderer *r, char *buffer, vec2 position, f32 line_height, 
          Font *loaded_font, vec4 color, bool break_lines) {
    draw_text(r, buffer, {position.x, position.y, 0.0f}, line_height, loaded_font, color, break_lines);
}

static Texture2D
create_texture_2d(Renderer *r, u8 *data, i32 width, i32 height, i32 channels, 
                  Texture2DParams *params) {
    Texture2DParams tex_params = {};
    if(params) {
        tex_params = *params;
    }
    else { 
        tex_params.min_filter = FILTER_nearest;
        tex_params.mag_filter = FILTER_nearest;
        tex_params.wrap_s = WRAP_repeat;
        tex_params.wrap_t = WRAP_repeat;
    }
    
    Texture2D tex = r->api.create_texture_2d(data, width, height, channels, tex_params);
    return tex;
}

static Texture2D
create_texture_2d(Renderer *r, const char *path, Texture2DParams *params) {
    stbi_set_flip_vertically_on_load(true);   
    
    i32 width;
    i32 height;
    i32 channels;
    stbi_uc *data = stbi_load(path, &width, &height, &channels, 0);
    assert(data);
    Texture2D tex = create_texture_2d(r, data, width, height, channels, params);
    stbi_image_free(data);
    return tex;
}

static void 
delete_texture_2d(Renderer *r, Texture2D *tex) {
    r->api.delete_texture_2d(tex);
}

static void 
bind_texture_2d(Renderer *r, u32 tex_id, u32 unit) {
    r->api.bind_texture_2d(tex_id, unit);
}

static void 
unbind_texture_2d(Renderer *r, u32 unit) {
    r->api.unbind_texture_2d(unit);
}

static SpriteSheet
create_sprite_sheet(Renderer *r, u8 *data, i32 width, i32 height, i32 channels, 
                    u32 x_pixels_per_sprite, u32 y_pixels_per_sprite, Texture2DParams *params) {
    SpriteSheet ss = {};
    ss.tex = create_texture_2d(r, data, width, height, channels, params);
    if(ss.tex.id) {
        ss.x_pixels_per_sprite = x_pixels_per_sprite;
        ss.y_pixels_per_sprite = y_pixels_per_sprite;
        ss.x_sprites = ss.tex.width / x_pixels_per_sprite;
        ss.y_sprites = ss.tex.height / y_pixels_per_sprite;
        ss.ss_width = ss.x_sprites * x_pixels_per_sprite;
        ss.ss_height = ss.y_sprites * y_pixels_per_sprite;
    }
    return ss;
}

static SpriteSheet
create_sprite_sheet(Renderer *r, const char *path, u32 x_pixels_per_sprite, 
                    u32 y_pixels_per_sprite, Texture2DParams *params) {
    stbi_set_flip_vertically_on_load(true);   
    
    i32 width;
    i32 height;
    i32 channels;
    stbi_uc *data = stbi_load(path, &width, &height, &channels, 0);
    assert(data);
    SpriteSheet ss = create_sprite_sheet(r, data, width, height, channels,x_pixels_per_sprite, y_pixels_per_sprite, params);
    stbi_image_free(data);
    return ss;
}

static void 
delete_sprite_sheet(Renderer *r, SpriteSheet *ss) {
    delete_texture_2d(r, &ss->tex);
    zero_struct(ss);
}

static Font
create_font(Renderer *r, char *path, f32 height_in_pixels, Texture2DParams *glyph_params) {
    Font font = {};
    
    Texture2DParams params = {
        FORMAT_rgba8,
        FORMAT_rgba,
        PIXEL_TYPE_unsigned_byte,
        FILTER_linear,
        FILTER_linear,
        WRAP_clamp_to_edge,
        WRAP_clamp_to_edge
    };
    
    if(glyph_params) {
        params = *glyph_params;
    }
    
    FileContents font_file;
    if(r->procs->read_file(&font_file, path, true)) {
        u8 *font_data = (u8 *)font_file.contents;
        
        stbtt_fontinfo stbtt_font;
        stbtt_InitFont(&stbtt_font, font_data, stbtt_GetFontOffsetForIndex(font_data, 0));
        
        i32 width = 0;
        i32 height = 0;
        i32 x_offset = 0;
        i32 y_offset = 0;
        
        f32 scale_for_pixel_height = stbtt_ScaleForPixelHeight(&stbtt_font, height_in_pixels);
        
        // NOTE: temp
        font.kerning_entries = (KerningEntry *)r->procs->alloc(sizeof(KerningEntry) * square(FONT_GLYPH_COUNT));
        for(u8 c0 = 0; c0 < FONT_GLYPH_COUNT; ++c0) {
            for(u8 c1 = 0; c1 < FONT_GLYPH_COUNT; ++c1) {
                KerningEntry *entry = &font.kerning_entries[c0 * FONT_GLYPH_COUNT + c1];
                entry->advance = stbtt_GetCodepointKernAdvance(&stbtt_font, c0, c1);
            }
        }
        
        // NOTE: alloc
        font.glyphs = (Glyph *)r->procs->alloc(sizeof(Glyph) * FONT_GLYPH_COUNT);
        for(u32 _char = 0; _char < FONT_GLYPH_COUNT; ++_char) {
            Glyph *glyph = &font.glyphs[_char];
            
            u8 *bitmap = stbtt_GetCodepointBitmap(&stbtt_font, 0, scale_for_pixel_height,
                                                  _char, &width, &height, &x_offset, &y_offset);
            
            for(i32 x = 0; x < width; ++x) {
                u8 *col_temp = (u8 *)r->procs->alloc(height);
                for(i32 y = 0; y < height; ++y) {
                    col_temp[height - y - 1] = bitmap[y * width + x];
                }
                for(i32 y = 0; y < height; ++y) {
                    bitmap[y * width + x] = col_temp[y];
                }
                r->procs->free(col_temp);
            }
            
            // TODO: hhh
            i32 bmp_bpp = 4;
            i32 tex_size = width * height * bmp_bpp;
            u8 *tex_bitmap = (u8 *)r->procs->alloc(tex_size);
            memset(tex_bitmap, 1, tex_size);
            for(i32 h = 1; h < height - 1; ++h) {
                for(i32 w = 1; w < width - 1; ++w) {
                    i32 begin = (h * width + w) * bmp_bpp;
                    tex_bitmap[begin]     = bitmap[(h * width + w)];
                    tex_bitmap[begin + 1] = bitmap[(h * width + w)];
                    tex_bitmap[begin + 2] = bitmap[(h * width + w)];
                    tex_bitmap[begin + 3] = bitmap[(h * width + w)];
                }
            }
            
            glyph->tex = create_texture_2d(r, tex_bitmap, width, height, bmp_bpp, &params);
            glyph->y_offset = y_offset;
            
            i32 advance, left_side_bearing;
            stbtt_GetCodepointHMetrics(&stbtt_font, _char, &advance, &left_side_bearing);
            glyph->left_side_bearing = left_side_bearing;
            glyph->advance = advance;
            
            r->procs->free(tex_bitmap);
            stbtt_FreeBitmap(bitmap, 0);
        }
        font.height = height_in_pixels;
        font.scale_factor = scale_for_pixel_height;
        stbtt_GetFontVMetrics(&stbtt_font, &font.ascent, &font.descent, &font.line_gap);
        
        font.valid = true;
        r->procs->free_file(&font_file);
    }
    else {
        ASSERT(0, "couldn't load <%s>...\n", path);
    }
    return font;
}

static void 
delete_font(Renderer *r, Font *font) {
    r->procs->free(font->kerning_entries);
    for(i32 i = 0; i < FONT_GLYPH_COUNT; ++i) {
        Glyph *glyph = &font->glyphs[i];
        delete_texture_2d(r, &glyph->tex);
    }
    r->procs->free(font->glyphs);
}

static vec2 // TODO, NOTE: temp temp temp temp temp
get_text_size(const char *string, Font *font, f32 line_height, bool break_lines) {
    if(!font) {
        return { 0.0f, 0.0f };
    }
    f32 scale = line_height / font->height;
    f32 space = scale * font->height;
    
    f32 old_width = 0.0f;
    
    f32 x_cursor_start = 0.0f;
    f32 y_cursor_start = 0.0f;
    f32 x_cursor = x_cursor_start;
    f32 y_cursor = y_cursor_start;
    y_cursor += line_height;
    char *text_cursor = (char *)string;
    while(true) {
        if(*text_cursor == '\0') {
            break;
        }
        if(*text_cursor == '\n') {
            if(break_lines) {
                old_width = max_value(old_width, (x_cursor - x_cursor_start));
                x_cursor = x_cursor_start;
                y_cursor += line_height;
            }
        }
        
        Glyph *glyph = &font->glyphs[*text_cursor];
        if(!glyph) continue;
        
        vec2 draw_size = make_vec2(glyph->tex.width * scale,
                                   glyph->tex.height * scale);
        
        x_cursor += (glyph->advance * font->scale_factor) * scale;
        if(text_cursor + 1) {
            i32 kern = get_kerning_advance(font, *text_cursor, *(text_cursor + 1));
            x_cursor += ((f32)kern * font->scale_factor) * scale;
        }
        ++text_cursor;
    }
    return make_vec2(max_value(old_width, (x_cursor - x_cursor_start)),
                     y_cursor - y_cursor_start);
}

static i32 // NOTE: temp
get_kerning_advance(Font *font, char c0, char c1) {
    i32 advance = font->kerning_entries[c0 * FONT_GLYPH_COUNT + c1].advance;
    return advance;
}

static VertexBuffer 
create_vertex_buffer(Renderer *r, void *data, u32 size, vb_usage usage) {
    VertexBuffer vb = r->api.create_vertex_buffer(data, size, usage);
    return vb;
}

static void 
delete_vertex_buffer(Renderer *r, VertexBuffer *vb) {
    r->api.delete_vertex_buffer(vb);
}

static void 
set_vertex_buffer_data(Renderer *r, VertexBuffer *vb, void *data, u32 size, u32 offset) {
    r->api.set_vertex_buffer_data(vb, data, size, offset);
}

static IndexBuffer 
create_index_buffer(Renderer *r, u32 *data, u32 count) {
    IndexBuffer ib = r->api.create_index_buffer(data, count);
    return ib;
}

static void 
delete_index_buffer(Renderer *r, IndexBuffer *ib) {
    r->api.delete_index_buffer(ib);
}

static VertexArray 
create_vertex_array(Renderer *r) {
    VertexArray va = r->api.create_vertex_array();
    return va;
}

static void 
delete_vertex_array(Renderer *r, VertexArray *va) {
    r->api.delete_vertex_array(va);
}

static void
attach_index_buffer(Renderer *r, VertexArray *va, IndexBuffer *ib) {
    r->api.attach_index_buffer(va, ib);
}

static void 
attach_vertex_buffer(Renderer *r, VertexArray *va, VertexBuffer *vb) {
    r->api.attach_vertex_buffer(va, vb);
}

static Framebuffer 
create_framebuffer(Renderer *r, u32 width, u32 height) {
    Framebuffer fb = r->api.create_framebuffer(width, height);
    return fb;
}

static void 
resize_framebuffer(Renderer *r, Framebuffer *fb, u32 width, u32 height) {
    r->api.resize_framebuffer(fb, width, height);
}

static void 
delete_framebuffer(Renderer *r, Framebuffer *fb) {
    r->api.delete_framebuffer(fb);
}

static void 
bind_framebuffer(Renderer *r, Framebuffer *fb) {
    r->bound_framebuffer = fb;
    r->api.bind_framebuffer(fb);
}

static void 
unbind_framebuffer(Renderer *r) {
    r->bound_framebuffer = nullptr;
    r->api.unbind_framebuffer();
}

static ShaderProgram 
create_shader_program(Renderer *r, char *vertex, i32 vertex_len, char *fragment, i32 fragment_len) {
    ShaderProgram shader_program = r->api.create_shader(vertex, vertex_len, fragment, fragment_len);
    return shader_program;
}

static ShaderProgram 
create_shader_program(Renderer *r, const char *path) {
    ShaderProgram shader_program = {};
    FileContents shader_file;
    if(r->procs->read_file(&shader_file, path, true)) {
        ShaderSource shader_src;
        if(get_shader_source_info(&shader_src, (char *)shader_file.contents, shader_file.size)) {
            shader_program = r->api.create_shader(shader_src.vertex, (i32)shader_src.vertex_len, 
                                                  shader_src.fragment, (i32)shader_src.fragment_len);
        }
        r->procs->free_file(&shader_file);
    }
    return shader_program;
}

static void
delete_shader_program(Renderer *r, ShaderProgram *shader_program) {
    r->api.delete_shader(shader_program);
}

static ShaderRef *
create_shader(Renderer *r, const char *path) {
    ASSERT(r->shader_count <= size_array(r->shaders), "shader count exceeded...");
    
    ShaderProgram shader = create_shader_program(r, path);
    ASSERT(shader.id != 0, "couldn't create shader <%s>", path);
    if(shader.id == 0) {
        return nullptr;
    }
    
    ShaderRef *ref = &r->shaders[r->shader_count];
    ref->loaded_from_file = true;
    copy_memory((void *)path, (void *)ref->path, min_value(strlen(path), size_array(ref->path)));
    ref->array_id = r->shader_count;
    ref->last_write_time = r->procs->last_write_time(path);
    ref->shader = shader;
    ++r->shader_count;
    return ref;
}

static ShaderRef *
create_shader(Renderer *r, char *vertex, i32 vertex_len, char *fragment, i32 fragment_len, char *name) {
    ASSERT(r->shader_count <= size_array(r->shaders), "shader count exceeded...");
    
    ShaderProgram shader = {};
    shader = r->api.create_shader(vertex, vertex_len, fragment, fragment_len);
    ASSERT(shader.id != 0, "couldn't create shader <%s>", name);
    
    ShaderRef *ref = &r->shaders[r->shader_count];
    ref->loaded_from_file = false;
    copy_memory((void *)name, (void *)ref->name, min_value(strlen(name), size_array(ref->name)));
    ref->array_id = r->shader_count;
    ref->shader = shader;
    ++r->shader_count;
    return ref;
}

static void 
delete_shader(Renderer *r, ShaderProgram *shader) {
    r->api.delete_shader(shader);
}

static void 
bind_shader(Renderer *r, ShaderProgram *shader_program) {
    // NOTE: flush before binding new shader
    // flush(r);
    r->bound_shader = shader_program;
    r->api.bind_shader(shader_program);
}

static void 
unbind_shader(Renderer *r) {
    r->bound_shader = nullptr;
    r->api.unbind_shader();
}

static bool set_uniform_int(Renderer *r, ShaderProgram *shader,    const char *name, i32 v)    { return r->api.set_uniform_int(shader, name, v); }
static bool set_uniform_float(Renderer *r, ShaderProgram *shader,  const char *name, f32 v)    { return r->api.set_uniform_float(shader, name, v); }
static bool set_uniform_float2(Renderer *r, ShaderProgram *shader, const char *name, vec2 v)   { return r->api.set_uniform_float2(shader, name, v); }
static bool set_uniform_float3(Renderer *r, ShaderProgram *shader, const char *name, vec3 v)   { return r->api.set_uniform_float3(shader, name, v); }
static bool set_uniform_float4(Renderer *r, ShaderProgram *shader, const char *name, vec4 v)   { return r->api.set_uniform_float4(shader, name, v); }
static bool set_uniform_mat4x4(Renderer *r, ShaderProgram *shader, const char *name, mat4x4 v) { return r->api.set_uniform_mat4x4(shader, name, v); }
static bool set_uniform_int_array(Renderer *r, ShaderProgram *shader, const char *name, i32 *v, u32 count) { return r->api.set_uniform_int_array(shader, name, v, count); }