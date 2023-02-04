inline const char 
*opengl_get_error_msg(GLenum error_code) {
    switch (error_code) {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        default:                   return "GL_UNKNOWN_ERROR";
    }
}

inline GLenum 
opengl_layout_data_type(layout_data_type type) {
    switch(type) {
        case LAYOUT_int32:   return GL_INT;
        case LAYOUT_float32: return GL_FLOAT;
        default: ASSERT(false, "opengl: layout data type...");
    }
    return 0;
}

inline GLenum 
opengl_tex_format(tex_format format) {
    switch(format) {
        case FORMAT_rgb:              return GL_RGB;
        case FORMAT_rgb8:             return GL_RGB8;
        case FORMAT_rgba:             return GL_RGBA;
        case FORMAT_rgba8:            return GL_RGBA8;
        case FORMAT_depth_stencil:    return GL_DEPTH_STENCIL;
        case FORMAT_depth24_stencil8: return GL_DEPTH24_STENCIL8;
        default: ASSERT(false, "opengl: invalid tex format...");
    }
    return 0;
}

inline GLenum
opengl_pixel_data_type(pixel_data_type type) {
    switch(type) {
        case PIXEL_TYPE_unsigned_byte:     return GL_UNSIGNED_BYTE;
        case PIXEL_TYPE_unsigned_int_24_8: return GL_UNSIGNED_INT_24_8;
        default: ASSERT(false, "opengl: pixel data format...");
    }
    return 0;
}

static u32
opengl_compile_shader_source(const char *source, GLint source_len, GLenum source_type) {
    GLuint id = glCreateShader(source_type);
    glShaderSource(id, 1, &source, &source_len);
    glCompileShader(id);
    
    GLint did_compile;
    glGetShaderiv(id, GL_COMPILE_STATUS, &did_compile);
    if(did_compile != GL_TRUE) {
        char message[1024];
        glGetShaderInfoLog(id, 1024, 0, message);
        if(source_type == GL_VERTEX_SHADER) {
            opengl_printf("Vertex shader errors:\n");
        }
        else {
            opengl_printf("Fragment shader errors:\n");
        }
        opengl_printf(message);
        glDeleteShader(id);
    }
    return id;
}

static void
opengl_set_attrib_pointers(VertexBuffer *vbs, u32 vb_count) {
    u32 location = 0;
    for(u32 vb_id = 0; vb_id < vb_count; ++vb_id) {
        VertexBuffer *vb = &vbs[vb_id];
        BufferLayout *layout = &vb->layout;
        glBindBuffer(GL_ARRAY_BUFFER, vb->id);
        for(u32 element_id = 0; element_id < layout->element_count; ++element_id) {
            LayoutElement *e = &layout->elements[element_id];
            GLenum type = opengl_layout_data_type(e->type);
            GLboolean normalized = e->normalized ? GL_TRUE : GL_FALSE;
            
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, e->count, type, normalized, layout->stride, (void *)((u64)e->offset));
            ++location;
        }
    }
}

static void
opengl_set_attrib_pointers(VertexBuffer vb, u32 starting_location) {
    BufferLayout layout = vb.layout;
    glBindBuffer(GL_ARRAY_BUFFER, vb.id);
    for(u32 element_id = 0; element_id < layout.element_count; ++element_id) {
        LayoutElement e = layout.elements[element_id];
        GLenum type = opengl_layout_data_type(e.type);
        GLboolean normalized = e.normalized ? GL_TRUE : GL_FALSE;
        
        u32 location = starting_location + element_id;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, e.count, type, normalized, layout.stride, (void *)((u64)e.offset));
        ++location;
    }
}

static u32
opengl_get_vbs_element_count(VertexBuffer *vbs, u32 vb_count) {
    u32 element_count = 0;
    for(u32 i = 0; i < vb_count; ++i) {
        VertexBuffer *vb = &vbs[i];
        element_count += vb->layout.element_count;
    }
    return element_count;
}

SET_UNIFORM_INT_PROC(opengl_set_uniform_int) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform1i(uniform_location, v);
    return true;
}

SET_UNIFORM_FLOAT_PROC(opengl_set_uniform_float) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform1f(uniform_location, v);
    return true;
}

SET_UNIFORM_FLOAT2_PROC(opengl_set_uniform_float2) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform2f(uniform_location, v.e[0], v.e[1]);
    return true;
}

SET_UNIFORM_FLOAT3_PROC(opengl_set_uniform_float3) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform3f(uniform_location, v.e[0], v.e[1], v.e[2]);
    return true;
}

SET_UNIFORM_FLOAT4_PROC(opengl_set_uniform_float4) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform4f(uniform_location, v.e[0], v.e[1], v.e[2], v.e[3]);
    return true;
}

SET_UNIFORM_MAT4X4_PROC(opengl_set_uniform_mat4x4) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniformMatrix4fv(uniform_location, 1, false, v.e);
    return true;
}

SET_UNIFORM_INT_ARRAY_PROC(opengl_set_uniform_int_array) {
    GLint uniform_location = glGetUniformLocation(shader->id, uniform_name);
    if(uniform_location == -1) { return false; }
    glUniform1iv(uniform_location, count, v);
    return true;
}

BIND_TEXTURE_2D_PROC(opengl_bind_texture_2d) {
    if(tex_id) {
        glBindTextureUnit(unit, tex_id);
    }
}

UNBIND_TEXTURE_2D_PROC(opengl_unbind_texture_2d) {
    glBindTextureUnit(unit, 0);
}

CREATE_TEXTURE_2D_PROC(opengl_create_texture_2d) {
    Texture2D tex = {};
    tex.width = width;
    tex.height = height;
    tex.channels = channels;
    
    GLenum internal_format = opengl_tex_format(params.internal_format);
    GLenum data_format     = opengl_tex_format(params.pixel_data_format);
    GLenum data_type       = opengl_pixel_data_type(params.data_type);
    
    glCreateTextures(GL_TEXTURE_2D, 1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, data_format, data_type, data);
    
    GLenum min_filter = (params.min_filter == FILTER_linear) ? GL_LINEAR : GL_NEAREST;
    GLenum mag_filter = (params.mag_filter == FILTER_linear) ? GL_LINEAR : GL_NEAREST;
    GLenum wrap_s = (params.wrap_s == WRAP_repeat) ? GL_REPEAT : GL_CLAMP;
    GLenum wrap_t = (params.wrap_t == WRAP_repeat) ? GL_REPEAT : GL_CLAMP;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    // glTextureSubImage2D(tex.id, 0, 0, 0, width, height, data_format, data_type, data);
    
    return tex;
}

DELETE_TEXTURE_2D_PROC(opengl_delete_texture_2d) {
    glDeleteTextures(1, &tex->id);
    zero_struct(tex);
}

BIND_SHADER_PROC(opengl_bind_shader) {
    glUseProgram(shader->id);
}

UNBIND_SHADER_PROC(opengl_unbind_shader) {
    glUseProgram(0);
}

CREATE_SHADER_PROC(opengl_create_shader) {
    ShaderProgram shader = {};
    u32 vert_id = opengl_compile_shader_source(vertex, vertex_len, GL_VERTEX_SHADER);
    u32 frag_id = opengl_compile_shader_source(fragment, fragment_len, GL_FRAGMENT_SHADER);
    if(glIsShader(vert_id) == GL_TRUE && 
       glIsShader(frag_id) == GL_TRUE) {
        shader.id = glCreateProgram();
        glAttachShader(shader.id, vert_id);
        glAttachShader(shader.id, frag_id);
        glLinkProgram(shader.id);
    }
    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
    return shader;
}

DELETE_SHADER_PROC(opengl_delete_shader) {
    glDeleteProgram(shader->id);
}

CREATE_VERTEX_BUFFER_PROC(opengl_create_vertex_buffer) {
    GLenum buffer_usage = (usage == VB_static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    
    VertexBuffer vb = {};
    vb.size = size;
    vb.layout = {};
    glGenBuffers(1, &vb.id);
    glBindBuffer(GL_ARRAY_BUFFER, vb.id);
    glBufferData(GL_ARRAY_BUFFER, size, data, buffer_usage);
    // glBindBuffer(GL_ARRAY_BUFFER, 0); TODO: ?
    return vb;
}

SET_VERTEX_BUFFER_DATA_PROC(opengl_set_vertex_buffer_data) {
    glBindBuffer(GL_ARRAY_BUFFER, vb->id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

DELETE_VERTEX_BUFFER_PROC(opengl_delete_vertex_buffer) {
    glDeleteBuffers(1, &vb->id);
    zero_struct(vb);
}

CREATE_INDEX_BUFFER_PROC(opengl_create_index_buffer) {
    IndexBuffer ib = {};
    ib.count = count;
    glGenBuffers(1, &ib.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), data, GL_STATIC_DRAW);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // TODO: ?
    return ib;
}

DELETE_INDEX_BUFFER_PROC(opengl_delete_index_buffer) {
    glDeleteBuffers(1, &ib->id);
    zero_struct(ib);
}

CREATE_VERTEX_ARRAY_PROC(opengl_create_vertex_array) {
    VertexArray va = {};
    glGenVertexArrays(1, &va.id);
    return va;
}

DELETE_VERTEX_ARRAY_PROC(opengl_delete_vertex_array) {
    glDeleteVertexArrays(1, &va->id);
    zero_struct(va);
}

ATTACH_VERTEX_BUFFER_PROC(opengl_attach_vertex_buffer) {
    u32 element_count = opengl_get_vbs_element_count(va->vbs, va->vb_count);
    va->vbs[va->vb_count++] = *vb;
    glBindVertexArray(va->id);
    // glBindBuffer(GL_ARRAY_BUFFER, vb->id);
    opengl_set_attrib_pointers(*vb, element_count);
}

ATTACH_INDEX_BUFFER_PROC(opengl_attach_index_buffer) {
    assert(!glIsBuffer(va->ib.id));
    glBindVertexArray(va->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id);
    va->ib = *ib;
}

SET_VIEWPORT_PROC(opengl_set_viewport) {
    glViewport(x, y, width, height);
}

CLEAR_PROC(opengl_clear) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

SET_CLIP_RECT(opengl_set_clip_rect) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);
}

DISABLE_CLIP_RECT(opengl_disable_clip_rect) {
    glDisable(GL_SCISSOR_TEST);
}

CREATE_FRAMEBUFFER_PROC(opengl_create_framebuffer) {
    u32 fb_id = 0;
    glCreateFramebuffers(1, &fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
    
#if 1
    Texture2DParams color_params = {
        FORMAT_rgba8,
        FORMAT_rgba,
        PIXEL_TYPE_unsigned_byte,
        FILTER_linear,
        FILTER_linear,
        WRAP_clamp_to_edge,
        WRAP_clamp_to_edge,
    };
#else
    Texture2DParams color_params = {
        FORMAT_rgb8,
        FORMAT_rgb,
        PIXEL_TYPE_unsigned_byte,
        FILTER_linear,
        FILTER_linear,
        WRAP_repeat,
        WRAP_repeat,
    };
#endif
    
    Texture2DParams depth_params = {
        FORMAT_depth24_stencil8,
        FORMAT_depth_stencil,
        PIXEL_TYPE_unsigned_int_24_8,
        FILTER_linear,
        FILTER_linear,
        WRAP_clamp_to_edge,
        WRAP_clamp_to_edge,
    };
    
    Texture2D color = opengl_create_texture_2d(nullptr, width, height, 4, color_params);
    ASSERT(color.id != 0, "color attachement...");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color.id, 0);
    
    Texture2D depth = opengl_create_texture_2d(nullptr, width, height, 4, depth_params);
    ASSERT(depth.id != 0, "depth attachement...");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.id, 0);
    
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer creation failed...\nopengl error: %s\n", opengl_get_error_msg(glGetError()));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    Framebuffer fb = {};
    fb.width = width;
    fb.height = height;
    fb.id = fb_id;
    fb.color = color;
    fb.depth = depth;
    return fb;
}

RESIZE_FRAMEBUFFER_PROC(opengl_resize_framebuffer) {
    // if(fb->id && glIsFramebuffer(fb->id)) 
    {
        opengl_delete_framebuffer(fb);
    }
    *fb = opengl_create_framebuffer(width, height);
}

DELETE_FRAMEBUFFER_PROC(opengl_delete_framebuffer) {
    opengl_delete_texture_2d(&fb->color);
    opengl_delete_texture_2d(&fb->depth);
    glDeleteFramebuffers(1, &fb->id);
    zero_struct(fb);
}

BIND_FRAMEBUFFER_PROC(opengl_bind_framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);
}

UNBIND_FRAMEBUFFER_PROC(opengl_unbind_framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DRAW_BUFFERS_PROC(opengl_draw_buffers) {
    glUseProgram(shader->id);
    opengl_set_attrib_pointers(vbs, vb_count);
    u32 size_per_vertex = 0;
    u32 total_size = 0;
    for(u32 i = 0; i < vb_count; ++i) {
        VertexBuffer *vb = &vbs[i];
        total_size += vb->size;
        for(u32 j = 0; j < vb->layout.element_count; ++j) {
            u32 element_size = vb->layout.elements[j].count * size_of_layout_data_type(vb->layout.elements[j].type);
            size_per_vertex += element_size;
        }
    }
    assert(size_per_vertex);
    glDrawArrays(GL_TRIANGLES, 0, total_size / size_per_vertex);
}

DRAW_BUFFERS_INDEXED_PROC(opengl_draw_buffers_indexed) {
    glUseProgram(shader->id);
    opengl_set_attrib_pointers(vbs, vb_count);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id);
    glDrawElements(GL_TRIANGLES, ib->count, GL_UNSIGNED_INT, nullptr);
}

DRAW_INDEXED_PROC(opengl_draw_indexed) {
    u32 draw_count = count ? count : va->ib.count;
    
    glBindVertexArray(va->id);
    glDrawElements(GL_TRIANGLES, draw_count, GL_UNSIGNED_INT, nullptr);
}

INITIALIZE_PROC(opengl_initialize) {
#if 0    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
#endif
    glEnable(GL_MULTISAMPLE);  
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    return true;
}

static void
get_renderer_api(RendererAPI *api) {
    api->backend = BACKEND_opengl;
    api->bind_texture_2d        = opengl_bind_texture_2d;
    api->unbind_texture_2d      = opengl_unbind_texture_2d;
    api->create_texture_2d      = opengl_create_texture_2d;
    api->delete_texture_2d      = opengl_delete_texture_2d;
    api->bind_shader            = opengl_bind_shader;
    api->unbind_shader          = opengl_unbind_shader;
    api->create_shader          = opengl_create_shader;
    api->delete_shader          = opengl_delete_shader;
    api->set_uniform_int        = opengl_set_uniform_int;
    api->set_uniform_float      = opengl_set_uniform_float;
    api->set_uniform_float2     = opengl_set_uniform_float2;
    api->set_uniform_float3     = opengl_set_uniform_float3;
    api->set_uniform_float4     = opengl_set_uniform_float4;
    api->set_uniform_mat4x4     = opengl_set_uniform_mat4x4;
    api->set_uniform_int_array  = opengl_set_uniform_int_array;
    api->create_vertex_buffer   = opengl_create_vertex_buffer;
    api->set_vertex_buffer_data = opengl_set_vertex_buffer_data;
    api->delete_vertex_buffer   = opengl_delete_vertex_buffer;
    api->create_index_buffer    = opengl_create_index_buffer;
    api->delete_index_buffer    = opengl_delete_index_buffer;
    api->create_vertex_array    = opengl_create_vertex_array;
    api->delete_vertex_array    = opengl_delete_vertex_array;
    api->attach_vertex_buffer   = opengl_attach_vertex_buffer;
    api->attach_index_buffer    = opengl_attach_index_buffer;
    api->create_framebuffer     = opengl_create_framebuffer;
    api->resize_framebuffer     = opengl_resize_framebuffer;
    api->delete_framebuffer     = opengl_delete_framebuffer;
    api->bind_framebuffer       = opengl_bind_framebuffer;
    api->unbind_framebuffer     = opengl_unbind_framebuffer;
    api->set_viewport           = opengl_set_viewport;
    api->clear                  = opengl_clear;
    api->set_clip_rect          = opengl_set_clip_rect;
    api->disable_clip_rect      = opengl_disable_clip_rect;
    api->draw_buffers           = opengl_draw_buffers;
    api->draw_buffers_indexed   = opengl_draw_buffers_indexed;
    api->draw_indexed           = opengl_draw_indexed;
    api->initialize             = opengl_initialize;
}