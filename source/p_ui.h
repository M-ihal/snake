#ifndef P_UI_H
#define P_UI_H

enum button_state {
    BUTTON_up,
    BUTTON_hot,
    BUTTON_down,
    BUTTON_inactive
};

#define DRAW_BUTTON_PROC(name) void name(Renderer *renderer, char *text, button_state state, vec2 pos, vec2 size, struct ButtonTheme *theme)
typedef DRAW_BUTTON_PROC(draw_button_proc);

#define DRAW_LABEL_PROC(name) void name(Renderer *renderer, char *text, vec2 pos, vec2 size, struct LabelTheme *theme)
typedef DRAW_LABEL_PROC(draw_label_proc);

enum button_style {
    BUTTON_STYLE_simple,
    BUTTON_STYLE_old,
    BUTTON_STYLE_shadow,
    BUTTON_STYLE_custom
};

struct ButtonTheme {
    button_style style;
    union { // NOTE: button_style order
        struct {
            u32 _unused;
        };
        struct {
            u32 _unused;
        };
        struct {
            f32  shadow_offset;
            vec4 shadow_color;
        };
        struct {
            draw_button_proc *custom_draw_proc;
        };
    };
    
    Font *font;
    
    f32 text_scale;
    f32 border_width;
    
    vec4 color;
    vec4 color_hover;
    vec4 color_down;
    vec4 color_inactive;
    
    vec4 bg_color;
    vec4 bg_color_hover;
    vec4 bg_color_down;
    vec4 bg_color_inactive;
    
    vec4 border_color;
    vec4 border_color_hover;
    vec4 border_color_down;
    vec4 border_color_inactive;
};

enum label_style {
    LABEL_STYLE_simple,
    LABEL_STYLE_shadow,
    LABEL_STYLE_custom
};

struct LabelTheme {
    label_style style;
    union { // NOTE: label_style order
        struct {
            u32 _unused;
        };
        struct {
            f32 shadow_offset;
            vec4 shadow_color;
        };
        struct {
            draw_label_proc *custom_draw_proc;
        };
    };
    
    Font *font;
    f32 font_height;
    vec4 color;
    vec4 bg_color;
};

enum layout_type {
    LAYOUT_vertical,
    LAYOUT_horizontal
};

struct ButtonLayout {
    layout_type type;
    
    vec2 position;
    bool reversed;
    
    f32  space;
    vec2 button_size;
};

inline ButtonLayout setup_button_layout(layout_type type, vec2 position, f32 space, vec2 button_size, bool reversed = false);
static vec2 next_button_position(ButtonLayout *layout);
static void offset_for_button_layout(ButtonLayout *layout, u32 num);

struct Panel {
    bool dragging;
    
    vec2 position;
    vec2 size;
};

static void update_and_render_panel(Panel *panel, struct UI *ui, Input *input);

typedef u32 uiid;
struct UI {
    Renderer *renderer;
    
    Framebuffer fb;
    Framebuffer *last_fb;
    
    i32 width;
    i32 height;
    
    uiid hot;
    uiid active;
};

static void init_ui(UI *ui, Renderer *r);
static void set_ui_dims(UI *ui, i32 width, i32 height);
static void begin_ui_frame(UI *ui);
static void ui_begin(UI *ui);
static void ui_end(UI *ui);
static bool do_button(UI *ui, Input *input, char *text, vec2 pos, vec2 size, ButtonTheme *theme, uiid id, bool inactive = false);
static void do_button_toggle(UI *ui, Input *input, char *text, bool *boolean, vec2 pos, vec2 size, ButtonTheme *theme, uiid id);
static void do_label(UI *ui, char *text, vec2 pos, vec2 size, LabelTheme *theme);
static DRAW_BUTTON_PROC(draw_button); 
static DRAW_LABEL_PROC(draw_label);

DRAW_BUTTON_PROC(draw_button_simple);
DRAW_BUTTON_PROC(draw_button_old_style);
DRAW_BUTTON_PROC(draw_button_with_shadow);

DRAW_LABEL_PROC(draw_label_simple);
DRAW_LABEL_PROC(draw_label_with_shadow);

#endif /* P_UI_H */
