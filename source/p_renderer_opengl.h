#ifndef P_RENDERER_OPENGL_H
#define P_RENDERER_OPENGL_H

#include <GL/glew.h>

#define opengl_printf(str, ...) printf(str, __VA_ARGS__)

inline const char *opengl_get_error_msg(GLenum error_code);
inline GLenum opengl_layout_data_type(layout_data_type type);
inline GLenum opengl_tex_format(tex_format format);
inline GLenum opengl_pixel_data_type(pixel_data_type type);
static u32  opengl_compile_shader_source(const char *source, GLint source_len, GLenum source_type);
static void opengl_set_attrib_pointers(VertexBuffer *vbs, u32 vb_count);
static void opengl_set_attrib_pointers(VertexBuffer vb, u32 starting_location = 0);
static u32  opengl_get_vbs_element_count(VertexBuffer *vbs, u32 vb_count);

SET_UNIFORM_INT_PROC(opengl_set_uniform_int);
SET_UNIFORM_FLOAT_PROC(opengl_set_uniform_float);
SET_UNIFORM_FLOAT2_PROC(opengl_set_uniform_float2);
SET_UNIFORM_FLOAT3_PROC(opengl_set_uniform_float3);
SET_UNIFORM_FLOAT4_PROC(opengl_set_uniform_float4);
SET_UNIFORM_MAT4X4_PROC(opengl_set_uniform_mat4x4);
SET_UNIFORM_INT_ARRAY_PROC(opengl_set_uniform_int_array);
BIND_TEXTURE_2D_PROC(opengl_bind_texture_2d);
UNBIND_TEXTURE_2D_PROC(opengl_unbind_texture_2d);
CREATE_TEXTURE_2D_PROC(opengl_create_texture_2d);
DELETE_TEXTURE_2D_PROC(opengl_delete_texture_2d);
BIND_SHADER_PROC(opengl_bind_shader);
UNBIND_SHADER_PROC(opengl_unbind_shader);
CREATE_SHADER_PROC(opengl_create_shader);
DELETE_SHADER_PROC(opengl_delete_shader);
CREATE_VERTEX_BUFFER_PROC(opengl_create_vertex_buffer);
SET_VERTEX_BUFFER_DATA_PROC(opengl_set_vertex_buffer_data);
DELETE_VERTEX_BUFFER_PROC(opengl_delete_vertex_buffer);
CREATE_INDEX_BUFFER_PROC(opengl_create_index_buffer);
DELETE_INDEX_BUFFER_PROC(opengl_delete_index_buffer);
CREATE_VERTEX_ARRAY_PROC(opengl_create_vertex_array);
DELETE_VERTEX_ARRAY_PROC(opengl_delete_vertex_array);
ATTACH_VERTEX_BUFFER_PROC(opengl_attach_vertex_buffer);
ATTACH_INDEX_BUFFER_PROC(opengl_attach_index_buffer);
SET_VIEWPORT_PROC(opengl_set_viewport);
CLEAR_PROC(opengl_clear);
SET_CLIP_RECT(opengl_set_clip_rect);
DISABLE_CLIP_RECT(opengl_disable_clip_rect);
CREATE_FRAMEBUFFER_PROC(opengl_create_framebuffer);
RESIZE_FRAMEBUFFER_PROC(opengl_resize_framebuffer);
DELETE_FRAMEBUFFER_PROC(opengl_delete_framebuffer);
BIND_FRAMEBUFFER_PROC(opengl_bind_framebuffer);
UNBIND_FRAMEBUFFER_PROC(opengl_unbind_framebuffer);
DRAW_BUFFERS_PROC(opengl_draw_buffers);
DRAW_BUFFERS_INDEXED_PROC(opengl_draw_buffers_indexed);
DRAW_INDEXED_PROC(opengl_draw_indexed);
INITIALIZE_PROC(opengl_initialize);

static void get_renderer(Renderer *r);

#endif /* P_RENDERER_OPENGL_H */
