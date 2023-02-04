#ifndef P_RENDERER_H
#define P_RENDERER_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_truetype.h>

#define GRAY(g,a) make_vec4(g,    g,    g,    a)
#define WHITE(a)  make_vec4(1.0f, 1.0f, 1.0f, a)
#define BLACK(a)  make_vec4(0.0f, 0.0f, 0.0f, a)
#define RED(a)    make_vec4(1.0f, 0.0f, 0.0f, a)
#define GREEN(a)  make_vec4(0.0f, 1.0f, 0.0f, a)
#define BLUE(a)   make_vec4(0.0f, 0.0f, 1.0f, a)
#define CYAN(a)   make_vec4(0.0f, 1.0f, 1.0f, a)
#define YELLOW(a) make_vec4(1.0f, 1.0f, 0.0f, a)
#define ORANGE(a) make_vec4(1.0f, 0.65f, 0.0f, a)

inline vec4
random_color(random_seed *seed, f32 alpha) {
    vec4 color = {
        rand_f32_in_range(seed, 0.0f, 1.0f),
        rand_f32_in_range(seed, 0.0f, 1.0f),
        rand_f32_in_range(seed, 0.0f, 1.0f),
        alpha
    };
    return color;
}

enum projection_type {
    PROJECTION_perspective,
    PROJECTION_orthographic,
};

struct OrthoCamera {
    f32 left;
    f32 bottom;
    f32 right;
    f32 top;
    f32 near;
    f32 far;
    f32 scale;
    f32 rotation;
    vec2 position;
};

struct PerspCamera {
    f32 aspect;
    f32 fov;
    f32 near;
    f32 far;
    vec3 position;
    vec3 up_vector;
    bool focus_on_point;
    union {
        struct { f32 vert_rotation; f32 horiz_rotation; };
        struct { vec3 focus_point; };
    };
};

struct Camera {
    projection_type proj_type;
    union {
        OrthoCamera ortho;
        PerspCamera persp;
    };
};

inline OrthoCamera *init_ortho_camera(Camera *camera);
inline PerspCamera *init_persp_camera(Camera *camera);
static mat4x4       camera_proj(Camera *camera);
static mat4x4       camera_view(Camera *camera);
static void         rotate_persp_camera(PerspCamera *camera, vec2 move_vector, f32 speed);
static vec2         ortho_proj_viewport_p(vec2 vp_p, recti32 viewport, OrthoCamera *camera);

enum pixel_data_type {
    PIXEL_TYPE_unsigned_byte,
    PIXEL_TYPE_unsigned_int_24_8,
};

enum tex_filter {
    FILTER_linear,
    FILTER_nearest
};

enum tex_wrap {
    WRAP_repeat,
    WRAP_clamp_to_edge
};

enum tex_format {
    FORMAT_rgb,
    FORMAT_rgb8,
    FORMAT_rgba,
    FORMAT_rgba8,
    FORMAT_depth_stencil,
    FORMAT_depth24_stencil8,
};

struct Texture2DParams {
    tex_format internal_format;   // NOTE: specifies components
    tex_format pixel_data_format; // NOTE: specifies format of the pixel data
    pixel_data_type data_type;    // NOTE: specifies data type of pixel data
    tex_filter min_filter;
    tex_filter mag_filter;
    tex_wrap wrap_s;
    tex_wrap wrap_t;
};

struct Texture2D {
    u32 id;
    i32 width;
    i32 height;
    i32 channels;
};

#define BIND_TEXTURE_2D_PROC(name) void name(u32 tex_id, u32 unit)
typedef BIND_TEXTURE_2D_PROC(bind_texture_2d_proc);

#define UNBIND_TEXTURE_2D_PROC(name) void name(u32 unit)
typedef UNBIND_TEXTURE_2D_PROC(unbind_texture_2d_proc);

#define CREATE_TEXTURE_2D_PROC(name) Texture2D name(u8 *data, i32 width, i32 height, i32 channels, Texture2DParams params)
typedef CREATE_TEXTURE_2D_PROC(create_texture_2d_proc);

#define DELETE_TEXTURE_2D_PROC(name) void name(Texture2D *tex)
typedef DELETE_TEXTURE_2D_PROC(delete_texture_2d_proc);

struct Glyph {
    Texture2D tex;
    i32 y_offset;
    i32 left_side_bearing;
    i32 advance;
};

struct KerningEntry {
    i32 advance;
};

// NOTE: alloc
#define FONT_GLYPH_COUNT 255
struct Font {
    bool valid;
    Glyph *glyphs;
    
    f32 height;
    f32 scale_factor;
    
    i32 ascent;
    i32 descent;
    i32 line_gap;
    
    KerningEntry *kerning_entries;
};

struct ShaderProgram {
    u32 id;
};

struct ShaderRef {
    bool loaded_from_file;
    union {
        struct { // NOTE: true
            PTime last_write_time;
            char path[64];
        };
        struct { // NOTE: false
            char name[64];
        };
    };
    u32 array_id;
    ShaderProgram shader;
};

struct ShaderSource {
    char  *vertex;
    char  *fragment;
    size_t vertex_len;
    size_t fragment_len;
};

static bool get_shader_source_info(ShaderSource *src, char *file_data, size_t file_len);

#define BIND_SHADER_PROC(name) void name(ShaderProgram *shader)
typedef BIND_SHADER_PROC(bind_shader_proc);

#define UNBIND_SHADER_PROC(name) void name(void)
typedef UNBIND_SHADER_PROC(unbind_shader_proc);

#define CREATE_SHADER_PROC(name) ShaderProgram name(const char *vertex, i32 vertex_len, const char *fragment, i32 fragment_len)
typedef CREATE_SHADER_PROC(create_shader_proc);

#define DELETE_SHADER_PROC(name) void name(ShaderProgram *shader)
typedef DELETE_SHADER_PROC(delete_shader_proc);

#define SET_UNIFORM_INT_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, i32 v)
typedef SET_UNIFORM_INT_PROC(set_uniform_int_proc);

#define SET_UNIFORM_FLOAT_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, f32 v)
typedef SET_UNIFORM_FLOAT_PROC(set_uniform_float_proc);

#define SET_UNIFORM_FLOAT2_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, vec2 v)
typedef SET_UNIFORM_FLOAT2_PROC(set_uniform_float2_proc);

#define SET_UNIFORM_FLOAT3_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, vec3 v)
typedef SET_UNIFORM_FLOAT3_PROC(set_uniform_float3_proc);

#define SET_UNIFORM_FLOAT4_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, vec4 v)
typedef SET_UNIFORM_FLOAT4_PROC(set_uniform_float4_proc);

#define SET_UNIFORM_MAT4X4_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, mat4x4 v)
typedef SET_UNIFORM_MAT4X4_PROC(set_uniform_mat4x4_proc);

#define SET_UNIFORM_INT_ARRAY_PROC(name) bool name(ShaderProgram *shader, const char *uniform_name, i32 *v, u32 count)
typedef SET_UNIFORM_INT_ARRAY_PROC(set_uniform_int_array_proc);

enum layout_data_type {
    LAYOUT_int32,
    LAYOUT_float32,
};

inline u32 size_of_layout_data_type(layout_data_type type) {
    switch(type) {
        case LAYOUT_int32:    return sizeof(i32);
        case LAYOUT_float32:  return sizeof(f32);
        default: assert(0);   return 0;
    }
}

#define LAYOUT_ELEMENT_NAME_MAX 64
struct LayoutElement {
    char name[LAYOUT_ELEMENT_NAME_MAX];
    u32 count;
    u32 offset;
    bool normalized;
    layout_data_type type;
};

#define MAX_BUFFER_LAYOUT_ELEMENTS 8
struct BufferLayout {
    u32 stride;
    u32 element_count;
    LayoutElement elements[MAX_BUFFER_LAYOUT_ELEMENTS];
    
    void push(u32 count, layout_data_type type, const char *name, bool normalized = false) {
        LayoutElement *e = &this->elements[this->element_count];
        u32 name_len = (u32)strlen(name);
        copy_memory(name, e->name, min_value(name_len, (LAYOUT_ELEMENT_NAME_MAX - 1)));
        e->offset = this->stride;
        e->count = count;
        e->type = type;
        e->normalized = normalized;
        
        u32 layout_type_size = size_of_layout_data_type(type);
        this->stride += count * layout_type_size;
        ++this->element_count;
    }
};

struct VertexBuffer {
    u32 id;
    u32 size; // NOTE: in bytes
    BufferLayout layout;
};

enum vb_usage {
    VB_static,
    VB_dynamic
};

#define CREATE_VERTEX_BUFFER_PROC(name) VertexBuffer name(void *data, u32 size, vb_usage usage)
typedef CREATE_VERTEX_BUFFER_PROC(create_vertex_buffer_proc);

#define SET_VERTEX_BUFFER_DATA_PROC(name) void name(VertexBuffer *vb, void *data, u32 size, u32 offset)
typedef SET_VERTEX_BUFFER_DATA_PROC(set_vertex_buffer_data_proc);

#define DELETE_VERTEX_BUFFER_PROC(name) void name(VertexBuffer *vb)
typedef DELETE_VERTEX_BUFFER_PROC(delete_vertex_buffer_proc);

struct IndexBuffer {
    u32 id;
    u32 count;
};

#define CREATE_INDEX_BUFFER_PROC(name) IndexBuffer name(void *data, u32 count)
typedef CREATE_INDEX_BUFFER_PROC(create_index_buffer_proc);

#define DELETE_INDEX_BUFFER_PROC(name) void name(IndexBuffer *ib)
typedef DELETE_INDEX_BUFFER_PROC(delete_index_buffer_proc);

#define VERTEX_ARRAY_VBS_MAX 8
struct VertexArray {
    u32 id;
    IndexBuffer  ib;
    u32          vb_count;
    VertexBuffer vbs[VERTEX_ARRAY_VBS_MAX];
};

#define CREATE_VERTEX_ARRAY_PROC(name) VertexArray name(void)
typedef CREATE_VERTEX_ARRAY_PROC(create_vertex_array_proc);

#define DELETE_VERTEX_ARRAY_PROC(name) void name(VertexArray *va)
typedef DELETE_VERTEX_ARRAY_PROC(delete_vertex_array_proc);

#define ATTACH_VERTEX_BUFFER_PROC(name) void name(VertexArray *va, VertexBuffer *vb)
typedef ATTACH_VERTEX_BUFFER_PROC(attach_vertex_buffer_proc);

#define ATTACH_INDEX_BUFFER_PROC(name) void name(VertexArray *va, IndexBuffer *ib)
typedef ATTACH_INDEX_BUFFER_PROC(attach_index_buffer_proc);

struct Framebuffer {
    u32 id;
    u32 width;
    u32 height;
    
    // NOTE: attachments
    Texture2D color;
    Texture2D depth;
};

#define CREATE_FRAMEBUFFER_PROC(name) Framebuffer name(u32 width, u32 height)
typedef CREATE_FRAMEBUFFER_PROC(create_framebuffer_proc);

#define RESIZE_FRAMEBUFFER_PROC(name) void name(Framebuffer *fb, u32 width, u32 height)
typedef RESIZE_FRAMEBUFFER_PROC(resize_framebuffer_proc);

#define DELETE_FRAMEBUFFER_PROC(name) void name(Framebuffer *fb)
typedef DELETE_FRAMEBUFFER_PROC(delete_framebuffer_proc);

#define BIND_FRAMEBUFFER_PROC(name) void name(Framebuffer *fb)
typedef BIND_FRAMEBUFFER_PROC(bind_framebuffer_proc);

#define UNBIND_FRAMEBUFFER_PROC(name) void name(void)
typedef UNBIND_FRAMEBUFFER_PROC(unbind_framebuffer_proc);

#define SET_VIEWPORT_PROC(name) void name(i32 x, i32 y, i32 width, i32 height)
typedef SET_VIEWPORT_PROC(set_viewport_proc);

#define CLEAR_PROC(name) void name(f32 r, f32 g, f32 b, f32 a)
typedef CLEAR_PROC(clear_proc);

#define SET_CLIP_RECT(name) void name(i32 x, i32 y, i32 width, i32 height)
typedef SET_CLIP_RECT(set_clip_rect_proc);

#define DISABLE_CLIP_RECT(name) void name(void)
typedef DISABLE_CLIP_RECT(disable_clip_rect_proc);

#define DRAW_BUFFERS_PROC(name) void name(VertexBuffer *vbs, u32 vb_count, ShaderProgram *shader)
typedef DRAW_BUFFERS_PROC(draw_buffers_proc);

#define DRAW_BUFFERS_INDEXED_PROC(name) void name(VertexBuffer *vbs, u32 vb_count, IndexBuffer *ib, ShaderProgram *shader)
typedef DRAW_BUFFERS_INDEXED_PROC(draw_buffers_indexed_proc);

#define DRAW_INDEXED_PROC(name) void name(VertexArray *va, u32 count)
typedef DRAW_INDEXED_PROC(draw_indexed_proc);

#define INITIALIZE_PROC(name) bool name(void)
typedef INITIALIZE_PROC(initialize_proc);

enum rendering_backend {
    BACKEND_none,
    BACKEND_opengl,
};

struct RendererAPI {
    rendering_backend backend;
    
    bind_texture_2d_proc              *bind_texture_2d;
    unbind_texture_2d_proc            *unbind_texture_2d;
    create_texture_2d_proc            *create_texture_2d;
    delete_texture_2d_proc            *delete_texture_2d;
    bind_shader_proc                  *bind_shader;
    unbind_shader_proc                *unbind_shader;
    create_shader_proc                *create_shader;
    delete_shader_proc                *delete_shader;
    set_uniform_int_proc              *set_uniform_int;
    set_uniform_float_proc            *set_uniform_float;
    set_uniform_float2_proc           *set_uniform_float2;
    set_uniform_float3_proc           *set_uniform_float3;
    set_uniform_float4_proc           *set_uniform_float4;
    set_uniform_mat4x4_proc           *set_uniform_mat4x4;
    set_uniform_int_array_proc        *set_uniform_int_array;
    create_vertex_buffer_proc         *create_vertex_buffer;
    delete_vertex_buffer_proc         *delete_vertex_buffer;
    set_vertex_buffer_data_proc       *set_vertex_buffer_data;
    create_index_buffer_proc          *create_index_buffer;
    delete_index_buffer_proc          *delete_index_buffer;
    create_vertex_array_proc          *create_vertex_array;
    delete_vertex_array_proc          *delete_vertex_array;
    attach_vertex_buffer_proc         *attach_vertex_buffer;
    attach_index_buffer_proc          *attach_index_buffer;
    create_framebuffer_proc           *create_framebuffer;
    resize_framebuffer_proc           *resize_framebuffer;
    delete_framebuffer_proc           *delete_framebuffer;
    bind_framebuffer_proc             *bind_framebuffer;
    unbind_framebuffer_proc           *unbind_framebuffer;
    set_viewport_proc                 *set_viewport;
    clear_proc                        *clear;
    set_clip_rect_proc                *set_clip_rect;
    disable_clip_rect_proc            *disable_clip_rect;
    draw_buffers_proc                 *draw_buffers;
    draw_buffers_indexed_proc         *draw_buffers_indexed;
    draw_indexed_proc                 *draw_indexed;
    initialize_proc                   *initialize;
};

// NOTE: :(
#if 1
inline bool
is_renderer_api_valid(RendererAPI *api) {
    // u64 *first = (u64 *)&api->bind_texture_2d;
    // u64 *last  = (u64 *)&api->initialize;
    // for(u64 *ptr = first; ptr <= last; ptr += 1) {
    //     u64 *func_ptr = (u64 *)(*ptr);
    //     // print_ptr(func_ptr);
    //     if(func_ptr == nullptr) {
    //         return false;
    //     }
    // }
    return true;
}
#endif

struct SpriteSheet {
    Texture2D tex;
    u32 ss_width;
    u32 ss_height;
    u32 x_pixels_per_sprite;
    u32 y_pixels_per_sprite;
    u32 x_sprites;
    u32 y_sprites;
    // vec2 sprite_coords[][];
};

struct SpriteSheetTile {
    SpriteSheet *ss;
    i32 x;
    i32 y;
};

static vec2_4x get_tex_coords(SpriteSheet *ss, u32 x, u32 y);

struct SceneData {
    mat4x4 proj;
    mat4x4 view;
};

struct QuadVertex {
    vec3  position;
    vec4  color;
    vec2  tex_coord;
    float tex_slot;
    vec2  tiling_factor;
};

struct RenderStats {
    u32 quad_count;
    u32 draw_calls;
};

struct Renderer {
    RendererAPI api;
    PlatformProcs *procs;
    
    RenderStats    stats;
    recti32        viewport;
    SceneData      scene_data;
    
    bool clipping_rect;
    recti32 clip_rect;
    
    Texture2D  white_texture; 
    ShaderRef *shader_basic;
    
    u32 max_quads;
    u32 max_quad_verts;
    u32 max_quad_indices;
    u32 max_texture_slots;
    VertexArray  quad_va;
    VertexBuffer quad_vb;
    IndexBuffer  quad_ib;
    QuadVertex  *quad_vb_data;
    u32         *quad_indices;
    u32         *texture_slots;
    u32          texture_count;
    u32          quad_count;
    ShaderProgram *bound_shader;
    Framebuffer   *bound_framebuffer;
    
    u32 stack_id;
#define RENDERER_STACK_SIZE 16
    recti32        stack_vp    [RENDERER_STACK_SIZE];
    Framebuffer   *stack_fb    [RENDERER_STACK_SIZE];
    SceneData      stack_scene [RENDERER_STACK_SIZE];
    ShaderProgram *stack_shader[RENDERER_STACK_SIZE];
    
    f32        shaders_hotload_counter;
    u32        shader_count;
    ShaderRef  shaders[4];
};

static void init_renderer(Renderer *r, RendererAPI *api, PlatformProcs *procs);
static void hotload_shaders(Renderer *r, f32 dt);
static void begin_renderer_frame(Renderer *r);

static void push(Renderer *r);
static void pop(Renderer *r);

static void set_batch_params(Renderer *r, u32 max_quads, u32 max_texture_slots);
static void set_viewport(Renderer *r, recti32 vp);
static void set_proj_and_view(Renderer *r, mat4x4 proj, mat4x4 view);
static void set_camera(Renderer *r, Camera *camera);
static void set_scene_data(Renderer *r, SceneData data);
static void set_clip_rect(Renderer *r, recti32 clip_rect);
static void disable_clip_rect(Renderer *r);
static void clear(void);
static void flush(Renderer *r);

static u32  bind_next_batch_texture_slot(Renderer *r, Texture2D *tex);
static void draw_quad_base(Renderer *r, vec3 positions[4], vec2 tex_coords[4], vec4 color, Texture2D *tex, vec2 tiling_factor);

struct QuadExParams {
    bool spritesheet;
    union {
        struct {
            Texture2D *texture;
        };
        struct {
            SpriteSheetTile ss_tile;
        };
    };
    
    vec4 color;
    bool flip_x;
    bool flip_y;
    vec2 tiling_factor;
    f32  rotation;
};

inline QuadExParams quad_ex_default(void) {
    QuadExParams params = {};
    params.spritesheet = false;
    params.texture = nullptr;
    params.color = WHITE(1.0f);
    params.flip_x = false;
    params.flip_y = false;
    params.tiling_factor = { 1.0f, 1.0f };
    params.rotation = 0.0f;
    return params;
}

static void draw_quad_ex(Renderer *r, vec3 position, vec2 size, QuadExParams *params);
static void draw_quad_ex(Renderer *r, vec2 position, vec2 size, QuadExParams *params);
static void draw_quad(Renderer *r, vec3 positions[4], vec4 color = WHITE(1.0f), Texture2D *tex = nullptr, vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad(Renderer *r, vec2 positions[4], vec4 color = WHITE(1.0f), Texture2D *tex = nullptr, vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad(Renderer *r, vec3 position, vec2 size, vec4 color = WHITE(1.0f), Texture2D *tex = nullptr, vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad(Renderer *r, vec2 position, vec2 size, vec4 color = WHITE(1.0f), Texture2D *tex = nullptr, vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad(Renderer *r, vec3 position, vec2 size, SpriteSheetTile ss_tile, vec4 color = WHITE(1.0f), vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad(Renderer *r, vec2 position, vec2 size, SpriteSheetTile ss_tile, vec4 color = WHITE(1.0f), vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad_rotated(Renderer *r, vec3 position, vec2 size, f32 rotation, vec4 color = WHITE(1.0f), Texture2D *tex = nullptr,vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad_rotated(Renderer *r, vec2 position, vec2 size, f32 rotation, vec4 color = WHITE(1.0f), Texture2D *tex = nullptr, vec2 tiling_factor = {1.0f, 1.0f});
static void draw_quad_rotated(Renderer *r, vec3 position, vec2 size, f32 rotation, SpriteSheetTile ss_tile, vec4 color = WHITE(1.0f));
static void draw_quad_rotated(Renderer *r, vec2 position, vec2 size, f32 rotation, SpriteSheetTile ss_tile, vec4 color = WHITE(1.0f));
static void draw_quad_outline(Renderer *r, vec3 position, vec2 size, f32 width, vec4 color = WHITE(1.0f));
static void draw_quad_outline(Renderer *r, vec2 position, vec2 size, f32 width, vec4 color = WHITE(1.0f));
static void draw_text(Renderer *r, char *buffer, vec3 position, f32 line_height, Font *loaded_font, vec4 color = WHITE(1.0f), bool break_lines = false);
static void draw_text(Renderer *r, char *buffer, vec2 position, f32 line_height, Font *loaded_font, vec4 color = WHITE(1.0f), bool break_lines = false);

static Texture2D   create_texture_2d(Renderer *r, u8 *data, i32 width, i32 height, i32 channels, Texture2DParams *params = nullptr);
static Texture2D   create_texture_2d(Renderer *r, const char *path, Texture2DParams *params = nullptr);
static void        delete_texture_2d(Renderer *r, Texture2D *tex);
static void        bind_texture_2d(Renderer *r, u32 tex_id, u32 unit = 0);
static void        unbind_texture_2d(Renderer *r, u32 unit = 0);

static SpriteSheet create_sprite_sheet(Renderer *r, u8 *data, i32 width, i32 height, i32 channels, u32 x_pixels_per_sprite, u32 y_pixels_per_sprite, Texture2DParams *params);
static SpriteSheet create_sprite_sheet(Renderer *r, const char *path, u32 x_pixels_per_sprite, u32 y_pixels_per_sprite, Texture2DParams *params);
static void        delete_sprite_sheet(Renderer *r, SpriteSheet *ss);

static Font create_font(Renderer *r, char *path, f32 height_in_pixels, Texture2DParams *glyph_params = nullptr);
static void delete_font(Renderer *r, Font *font);
static vec2 get_text_size(const char *string, Font *font, f32 line_height, bool break_lines = false);
static i32  get_kerning_advance(Font *font, char c0, char c1);

static VertexBuffer create_vertex_buffer(Renderer *r, void *data, u32 size, vb_usage usage = VB_static);
static void         delete_vertex_buffer(Renderer *r, VertexBuffer *vb);
static void         set_vertex_buffer_data(Renderer *r, VertexBuffer *vb, void *data, u32 size, u32 offset = 0);

static IndexBuffer create_index_buffer(Renderer *r, u32 *data, u32 count);
static void        delete_index_buffer(Renderer *r, IndexBuffer *ib);

static VertexArray create_vertex_array(Renderer *r);
static void        delete_vertex_array(Renderer *r, VertexArray *va);
static void        attach_index_buffer(Renderer *r, VertexArray *va, IndexBuffer *ib);
static void        attach_vertex_buffer(Renderer *r, VertexArray *va, VertexBuffer *vb);

static Framebuffer create_framebuffer(Renderer *r, u32 width, u32 height);
static void        resize_framebuffer(Renderer *r, Framebuffer *fb, u32 width, u32 height);
static void        delete_framebuffer(Renderer *r, Framebuffer *fb);
static void        bind_framebuffer(Renderer *r, Framebuffer *fb);
static void        unbind_framebuffer(Renderer *r);

static ShaderProgram create_shader_program(Renderer *r, char *vertex, i32 vertex_len, char *fragment, i32 fragment_len);
static ShaderProgram create_shader_program(Renderer *r, const char *path);
static void          delete_shader_program(Renderer *r, ShaderProgram *shader_program);

static ShaderRef *create_shader(Renderer *r, char *vertex, i32 vertex_len, char *fragment, i32 fragment_len);
static ShaderRef *create_shader(Renderer *r, const char *path);
static void       bind_shader(Renderer *r, ShaderProgram *shader_program);
static void       unbind_shader(Renderer *r);

static bool set_uniform_int(Renderer *r, ShaderProgram *shader,    const char *name, i32 v);
static bool set_uniform_float(Renderer *r, ShaderProgram *shader,  const char *name, f32 v);
static bool set_uniform_float2(Renderer *r, ShaderProgram *shader, const char *name, vec2 v);
static bool set_uniform_float3(Renderer *r, ShaderProgram *shader, const char *name, vec3 v);
static bool set_uniform_float4(Renderer *r, ShaderProgram *shader, const char *name, vec4 v);
static bool set_uniform_mat4x4(Renderer *r, ShaderProgram *shader, const char *name, mat4x4 v);
static bool set_uniform_int_array(Renderer *r, ShaderProgram *shader, const char *name, i32 *v, u32 count);

#endif /* P_RENDERER_H */
