inline ButtonLayout
setup_button_layout(layout_type type, vec2 position, f32 space, 
                    vec2 button_size, bool reversed) {
    ButtonLayout layout = {};
    layout.type = type;
    layout.position = position;
    layout.reversed = reversed;
    layout.space = space;
    layout.button_size = button_size;
    return layout;
}

static vec2 
next_button_position(ButtonLayout *layout) {
    vec2 position = layout->position;
    f32 mult = layout->reversed ? -1.0f : 1.0f;
    switch(layout->type) {
        case LAYOUT_vertical: {
            layout->position.y += mult * (layout->button_size.y + layout->space);
        } break;
        
        case LAYOUT_horizontal: {
            layout->position.x += mult * (layout->button_size.x + layout->space);
        } break;
        
        default: ASSERT(false, "");
    }
    return position;
}

static void
offset_for_button_layout(ButtonLayout *layout, u32 num) {
    f32 mult = layout->reversed ? 1.0f : -1.0f;
    switch(layout->type) {
        case LAYOUT_vertical: {
            layout->position.y += mult * (num - 1) * (layout->button_size.y + layout->space);
        } break;
        
        case LAYOUT_horizontal: {
            layout->position.x += mult * (num - 1) * (layout->button_size.x + layout->space);
        } break;
        
        default: ASSERT(false, "");
    }
}

static void // NOTE: temp
update_and_render_panel(Panel *panel, UI *ui, Input *input) {
    bool inside = collision_point_rect(input->mouse, panel->position, panel->size);
    if(inside) {
        if(!panel->dragging && input->pressed(MOUSE_left)) {
            panel->dragging = true;
        }
    }
    
    if(panel->dragging) {
        if(input->released(MOUSE_left)) {
            panel->dragging = false;
        }
        else {
            vec2 mouse_last_p = { input->mouse_last.x, ui->height - input->mouse_last.y };
            panel->position += input->mouse - mouse_last_p;
        }
    }
    
    draw_quad(ui->renderer, panel->position, panel->size, WHITE(1.0f), nullptr);
}

static void
init_ui(UI *ui, Renderer *r) {
    ui->renderer = r;
    
    ui->fb = {};
    ui->last_fb = nullptr;
    
    ui->width = 0;
    ui->height = 0;
    
    ui->hot = {};
    ui->active = {};
}

static void
set_ui_dims(UI *ui, i32 width, i32 height) {
    if(width > 0 && height > 0) {
        ui->width = width;
        ui->height = height;
        resize_framebuffer(ui->renderer, &ui->fb, (u32)width, (u32)height);
    }
}

static void 
begin_ui_frame(UI *ui) {
    ui_begin(ui);
    clear(ui->renderer, { 0.2f, 0.2f, 0.2f, 0.0f });
    ui_end(ui);
}

static void 
ui_begin(UI *ui) {
    ui->last_fb = ui->renderer->bound_framebuffer;
    bind_framebuffer(ui->renderer, &ui->fb);
    mat4x4 view = mat4x4_identity();
    mat4x4 proj = mat4x4_orthographic(0.0f, 0.0f, (f32)ui->width, (f32)ui->height, -1.0f, 1.0f);
    set_proj_and_view(ui->renderer, proj, view);
}

static void 
ui_end(UI *ui) {
    flush(ui->renderer);
    if(ui->last_fb) {
        bind_framebuffer(ui->renderer, ui->last_fb); 
    }
    else {
        unbind_framebuffer(ui->renderer);
    }
    ui->last_fb = nullptr;
}

static bool 
do_button(UI *ui, Input *input, char *text, vec2 pos, vec2 size, 
          ButtonTheme *theme, uiid id, bool inactive) {
    bool down    = false;
    bool clicked = false;
    button_state state = BUTTON_inactive;
    if(inactive) {
        // state = BUTTON_inactive;
    }
    else {
        if(input) {
            bool inside = ((input->mouse.x > pos.x && input->mouse.x < (pos.x + size.x)) &&
                           input->mouse.y > pos.y && input->mouse.y < (pos.y + size.y));
            if(inside) {
                ui->hot = id;
                if(input->is_down(MOUSE_left)) {
                    ui->active = id;
                }
            }
            
            if(id == ui->active) {
                if(id == ui->hot) {
                    if(input->released(MOUSE_left)) {
                        if(inside) {
                            clicked = true;
                        }
                        ui->active = 0;
                    }
                }
                down = true;
            }
            
            if(down && inside) {
                state = BUTTON_down;
            }
            else if(inside || (!inside && down)) {
                state = BUTTON_hot;
            }
            else {
                state = BUTTON_up;
            }
        }
    }
    draw_button(ui->renderer, text, state, pos, size, theme);
    return clicked;
}

static void
do_button_toggle(UI *ui, Input *input, char *text, bool *boolean, 
                 vec2 pos, vec2 size, ButtonTheme *theme, uiid id) {
    bool down    = false;
    bool clicked = false;
    button_state state = BUTTON_inactive;
    if(input) {
        bool inside = ((input->mouse.x > pos.x && input->mouse.x < (pos.x + size.x)) &&
                       input->mouse.y > pos.y && input->mouse.y < (pos.y + size.y));
        if(inside) {
            ui->hot = id;
            if(input->is_down(MOUSE_left)) {
                ui->active = id;
            }
        }
        
        if(id == ui->active) {
            if(id == ui->hot) {
                if(input->released(MOUSE_left)) {
                    if(inside) {
                        clicked = true;
                    }
                    ui->active = 0;
                }
            }
            down = true;
        }
        
        if(down && inside) {
            state = BUTTON_down;
        }
        else if(inside || (!inside && down)) {
            state = BUTTON_hot;
        }
        else {
            state = BUTTON_up;
        }
    }
    
    if(clicked) {
        *boolean = !*boolean;
    }
    
    if(*boolean) { 
        state = BUTTON_down;
    }
    
    draw_button(ui->renderer, text, state, pos, size, theme);
}

static void 
do_label(UI *ui, char *text, vec2 pos, vec2 size, LabelTheme *theme) {
    vec4 color = theme->color;
    
    draw_label(ui->renderer, text, pos, size, theme);
}

static DRAW_BUTTON_PROC(draw_button) {
    switch(theme->style) {
        case BUTTON_STYLE_simple: {
            draw_button_simple(renderer, text, state, pos, size, theme);
        } break;
        
        case BUTTON_STYLE_old: {
            draw_button_old_style(renderer, text, state, pos, size, theme);
        } break;
        
        case BUTTON_STYLE_shadow: {
            draw_button_with_shadow(renderer, text, state, pos, size, theme);
        } break;
        
        case BUTTON_STYLE_custom: {
            if(theme->custom_draw_proc) {
                theme->custom_draw_proc(renderer, text, state, pos, size, theme);
            }
        } break;
        
        default: ASSERT(false, "invalid button style...\n");
    }
}

static DRAW_LABEL_PROC(draw_label) {
    switch(theme->style) {
        case LABEL_STYLE_simple: {
            draw_label_simple(renderer, text, pos, size, theme);
        } break;
        
        case LABEL_STYLE_shadow: {
            draw_label_with_shadow(renderer, text, pos, size, theme);
        } break;
        
        case LABEL_STYLE_custom: {
            if(theme->custom_draw_proc) {
                theme->custom_draw_proc(renderer, text, pos, size, theme);
            }
        } break;
        
        default: ASSERT(false, "invalid label style...\n");
    }
}

#define get_button_colors(color, bg_color, border_color, state)\
switch(state) {\
case BUTTON_up: {\
color    = theme->color;\
bg_color = theme->bg_color;\
border_color = theme->border_color;\
} break;\
\
case BUTTON_hot: {\
color    = theme->color_hover;\
bg_color = theme->bg_color_hover;\
border_color = theme->border_color_hover;\
} break;\
\
case BUTTON_down: {\
color    = theme->color_down;\
bg_color = theme->bg_color_down;\
border_color = theme->border_color_down;\
} break;\
\
case BUTTON_inactive: {\
color    = theme->color_inactive;\
bg_color = theme->bg_color_inactive;\
border_color = theme->border_color_inactive;\
} break;\
\
default: ASSERT(false, "invalid button state...\n");\
}


DRAW_BUTTON_PROC(draw_button_simple) {
    vec4 color = {};
    vec4 bg_color = {};
    vec4 border_color = {};
    get_button_colors(color, bg_color, border_color, state);
    
    draw_quad(renderer, pos, size, bg_color);
    draw_quad_outline(renderer, pos, size, theme->border_width, border_color);
    
    f32 scale = 0.8f;
    Font *font = theme->font;
    f32 text_height = size.y * scale * theme->text_scale;
    f32 text_width = get_text_size(text, font, text_height).x;
    vec2 text_pos = { 
        max_value(pos.x + 3.0f, pos.x + (size.x * 0.5f) - (text_width * 0.5f)), 
        pos.y + size.y * 0.5f - text_height * 0.5f + (size.y * theme->text_scale * ((1.0f - scale) * 0.5f))
    };
    
    if(text_width >= size.x) {
        recti32 clip_rect = {
            (i32)(pos.x + theme->border_width), (i32)(pos.y + theme->border_width), 
            (i32)(size.x - theme->border_width * 2.0f), (i32)(size.y - theme->border_width * 2.0f)
        };
        flush(renderer);
        set_clip_rect(renderer, clip_rect);
    }
    
    f32 offset = 3.0f;
    draw_text(renderer, text, text_pos + make_vec2(offset, -offset), text_height, theme->font, make_vec4(color.rgb, 0.1f));
    draw_text(renderer, text, text_pos, text_height, theme->font, color);
    
    flush(renderer);
    disable_clip_rect(renderer);
}

DRAW_BUTTON_PROC(draw_button_old_style) {
    vec4 color = {};
    vec4 bg_color = {};
    vec4 border_color = {};
    get_button_colors(color, bg_color, border_color, state);
    
    vec4 color_center  = bg_color;
    vec4 color_border0 = vec_lerp(border_color, BLACK(1.0f), 0.2f);
    vec4 color_border1 = vec_lerp(border_color, BLACK(1.0f), 0.7f);
    
    if(state == BUTTON_down || state == BUTTON_inactive) {
        vec4 temp = color_border0;
        color_border0 = color_border1;
        color_border1 = temp;
    }
    
    f32 border_width = theme->border_width;
    
    // NOTE: center quad
    vec2 center_quad_size = size - (border_width * 2.0f);
    vec2 center_quad_pos  = pos + border_width;
    {
        draw_quad(renderer, center_quad_pos, center_quad_size, color_center);
    }
    
    // NOTE: "border"
    // NOTE: left
    {
        vec2 positions[4] = {
            { pos.x,                pos.y },
            { pos.x + border_width, pos.y + border_width },
            { pos.x + border_width, pos.y + size.y - border_width },
            { pos.x,                pos.y + size.y },
        };
        draw_quad(renderer, positions, color_border0);
    }
    // NOTE: top
    {
        vec2 positions[4] = {
            { pos.x,                         pos.y + size.y },
            { pos.x + size.x,                pos.y + size.y },
            { pos.x + size.x - border_width, pos.y + size.y - border_width },
            { pos.x + border_width,          pos.y + size.y - border_width },
        };
        draw_quad(renderer, positions, color_border0);
    }
    
    // NOTE: right
    {
        vec2 positions[4] = {
            { pos.x + size.x,                pos.y },
            { pos.x + size.x - border_width, pos.y + border_width },
            { pos.x + size.x - border_width, pos.y + size.y - border_width },
            { pos.x + size.x,                pos.y + size.y },
        };
        draw_quad(renderer, positions, color_border1);
    }
    // NOTE: bottom
    {
        vec2 positions[4] = {
            { pos.x,                         pos.y },
            { pos.x + size.x,                pos.y },
            { pos.x + size.x - border_width, pos.y + border_width },
            { pos.x + border_width,          pos.y + border_width },
        };
        draw_quad(renderer, positions, color_border1);
    }
    
    Font *font = theme->font;
    f32 text_height = center_quad_size.y * 0.8f * theme->text_scale;
    f32 text_width = get_text_size(text, font, text_height, false).x;
    vec2 text_pos = { 
        max_value(center_quad_pos.x + (center_quad_size.x * 0.5f) - (text_width * 0.5f), center_quad_pos.x), 
        center_quad_pos.y - (font->descent * font->scale_factor) * (center_quad_size.y / font->height) + center_quad_size.y * 0.05f 
    };
    vec4 text_color = color;
    
    if(text_width >= center_quad_size.x) {
        recti32 clip_rect = {
            (i32)center_quad_pos.x, (i32)center_quad_pos.y, 
            (i32)center_quad_size.x, (i32)center_quad_size.y
        };
        flush(renderer);
        set_clip_rect(renderer, clip_rect);
    }
    
    draw_text(renderer, text, text_pos, text_height, theme->font, text_color);
    
    flush(renderer);
    disable_clip_rect(renderer);
}

DRAW_BUTTON_PROC(draw_button_with_shadow) {
    vec4 color = {};
    vec4 bg_color = {};
    vec4 border_color = {};
    get_button_colors(color, bg_color, border_color, state);
    
    vec2 offset = { theme->shadow_offset, -theme->shadow_offset };
    if(state == BUTTON_down || state == BUTTON_inactive) {
        pos += offset;
        draw_quad(renderer, pos, size, bg_color);
    }
    else {
        draw_quad(renderer, make_vec3(pos + offset, 0.1f), size, theme->shadow_color);
        draw_quad(renderer, pos, size, bg_color);
    }
    
    f32 scale = 0.8f;
    Font *font = theme->font;
    f32 text_height = size.y * scale * theme->text_scale;
    f32 text_width = get_text_size(text, font, text_height).x;
    vec2 text_pos = { 
        max_value(pos.x + 3.0f, pos.x + (size.x * 0.5f) - (text_width * 0.5f)), 
        pos.y + size.y * 0.5f - text_height * 0.5f + (size.y * theme->text_scale * ((1.0f - scale) * 0.5f))
    };
    
    if(text_width >= size.x) {
        recti32 clip_rect = {
            (i32)pos.x, (i32)pos.y, 
            (i32)size.x, (i32)size.y
        };
        flush(renderer);
        set_clip_rect(renderer, clip_rect);
    }
    
    draw_text(renderer, text, text_pos + offset, text_height, theme->font, make_vec4(color.rgb, 0.1f));
    draw_text(renderer, text, text_pos, text_height, theme->font, color);
    
    flush(renderer);
    disable_clip_rect(renderer);
}

DRAW_LABEL_PROC(draw_label_simple) {
    vec4 color    = theme->color;
    vec4 bg_color = theme->bg_color;
    draw_quad(renderer, pos, size, bg_color);
    
    Font *font = theme->font;
    f32 text_height = theme->font_height;
    vec2 text_size = get_text_size(text, font, text_height, true);
    vec2 text_pos = { 
        pos.x + 3.0f,
        pos.y + size.y - theme->font_height
    };
    
    if(text_size.x >= size.x || text_size.y >= size.y) {
        recti32 clip_rect = {
            (i32)pos.x, (i32)pos.y, 
            (i32)size.x, (i32)size.y
        };
        set_clip_rect(renderer, clip_rect);
    }
    
    draw_text(renderer, text, text_pos, text_height, font, color, true);
    disable_clip_rect(renderer);
}

DRAW_LABEL_PROC(draw_label_with_shadow) {
    vec4 color    = theme->color;
    vec4 bg_color = theme->bg_color;
    
    vec2 offset = { theme->shadow_offset, -theme->shadow_offset };
    draw_quad(renderer, make_vec3(pos + offset, 0.1f), size, theme->shadow_color);
    draw_quad(renderer, pos, size, bg_color);
    
    Font *font = theme->font;
    f32 text_height = theme->font_height;
    f32 text_width = get_text_size(text, font, text_height).x;
    vec2 text_pos = { 
        pos.x + 3.0f,
        pos.y 
    };
    
    if(text_width >= size.x) {
        recti32 clip_rect = {
            (i32)pos.x, (i32)pos.y, 
            (i32)size.x, (i32)size.y
        };
        set_clip_rect(renderer, clip_rect);
    }
    
    draw_text(renderer, text, text_pos, text_height, font, color, true);
    disable_clip_rect(renderer);
}