#ifndef P_VARS_H
#define P_VARS_H

static const char *bool_true_strings[] = {
    "1",
    "yes",
    "true",
    "TRUE"
};

static const char *bool_false_strings[] = {
    "0",
    "no",
    "false",
    "FALSE"
};

struct TextLine {
    i32 len;
    char *ptr;
};

struct TextLines {
    i32 line_count;
    TextLine lines[1024];
};

enum var_type {
    VAR_f32,
    VAR_i32,
    VAR_bool,
    VAR_cstr,
    VAR_vec2,
    VAR_vec3,
    VAR_vec4
};

struct LineVarValue {
    u32  length;
    char value[256];
};

struct VarsFileVar {
    void *ptr;
    char name[128];
    var_type type;
};

struct VarsFile {
    bool  initialized;
    char  file_path[128];
    PTime file_mod_time;
    
    i32         var_count;
    VarsFileVar vars[256];
};

static void init_vars_file(char *file_path, VarsFile *vars_file);
static void attach_var(void *var_ptr, var_type type, char *var_name, VarsFile *vars_file);
static bool load_attached_vars(VarsFile *vars_file, PlatformProcs *procs);
static bool hotload_attached_vars(VarsFile *vars_file, PlatformProcs *procs);

// NOTE:

static bool get_text_lines(TextLines *lines, char *text);
inline bool get_file_lines(TextLines *lines, char *file_path);
static i32 search_for_var_line(TextLines lines, char *var_name);
static LineVarValue get_line_var_value(TextLine line);
static bool get_vec_from_value(char *value, f32 *v, i32 v_num);
static bool get_var_from_file(void *var, char *var_name, var_type type, TextLines lines);

#endif /* P_VARS_H */
