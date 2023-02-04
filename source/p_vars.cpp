#define IS_WHITE_SPACE(c) (c == '\n' || c == '\r' || c == '\0')

static bool
get_text_lines(TextLines *lines, char *text) {
    zero_struct(lines);
    
    i32 text_len = (i32)strlen(text);
    i32 line_size_counter = 0;
    bool first = true;
    char *c = text;
    for(c = text; true; ++c) {
        if(IS_WHITE_SPACE(*c)) {
            if(line_size_counter <= 1) {
                line_size_counter = 1;
                continue;
            }
            
            i32 new_line_count = lines->line_count + 1;
            ASSERT(new_line_count <= size_array(lines->lines), "memory overflow...\n");
            
            TextLine *line = &lines->lines[lines->line_count];
            line->ptr = c - line_size_counter;
            
            if(!first) {
                line->ptr += 1;
            }
            
            line->len = line_size_counter - 1;
            if(first) {
                line->len += 1;
            }
            
            lines->line_count = new_line_count;
            line_size_counter = 0;
            first = false;
        }
        line_size_counter++;
        
        if(c >= text + text_len) {
            break;
        }
    }
    return (lines->line_count > 0);
}

inline bool
get_file_lines(TextLines *lines, char *file_path, PlatformProcs *procs) {
    FileContents file;
    if(!procs->read_file(&file, file_path, true))
        return false;
    
    bool success = get_text_lines(lines, (char *)file.contents);
    procs->free_file(&file);
    return success;
}

static void 
init_vars_file(char *file_path, VarsFile *vars_file) {
    vars_file->initialized = true;
    copy_memory(file_path, vars_file->file_path, size_array(vars_file->file_path));
    vars_file->file_mod_time = {};
    vars_file->var_count = 0;
    zero_array(vars_file->vars, VarsFileVar);
}

static void 
attach_var(void *var_ptr, var_type type, char *var_name, VarsFile *vars_file) {
    VarsFileVar *var = &vars_file->vars[vars_file->var_count++];
    var->ptr = var_ptr;
    var->type = type;
    copy_memory(var_name, var->name, size_array(var->name));
}

static i32 
search_for_var_line(TextLines lines, char *var_name) {
    for(i32 i = 0; i < lines.line_count; ++i) {
        char *line_begin = (char *)lines.lines[i].ptr;
        char *line_end = line_begin + lines.lines[i].len;
        bool started = false;
        for(char *c = line_begin; c < line_end; ++c) {
            if(!started) {
                if(*c != ' ' && *c != '\r') {
                    started = true;
                }
                else {
                    line_begin++;
                }
            }
            if(started) {
                if(*c == ' ') {
                    char *end = c - 1;
                    i32 size = (i32)(end - line_begin + 1);
                    i32 var_name_size = (i32)strlen(var_name);
                    
                    if(size != var_name_size) break;
                    if(compare_memory(var_name, line_begin, size)) {
                        return i;
                    }
                    else {
                        break;
                    }
                }
            }
        }     
    }
    return -1;
}

#define IS_VALID_VALUE_CHAR(c) ((c >= 'a' && c <= 'z') || \
(c >= 'A' && c <= 'Z') || \
(c >= '0' && c <= '9'))
static LineVarValue
get_line_var_value(TextLine line) {
    char *c = line.ptr;
    
    if(*c == '#') {
        return {};
    }
    
    char *var_begin = nullptr;
    char *var_end = nullptr;
    
    bool started = false;
    bool equal_sign_found = false;
    
    for(i32 i = 0; i < line.len; ++i) {
        if(!started) {
            if(*c != ' ' && *c != '\r') {
                started = true;
            }
        }
        if(started) {
            if(!equal_sign_found) {
                if(*c == '=') {
                    equal_sign_found = true;
                }
            }
            else {
                if(!var_begin) {
                    if(*c != ' ') {
                        var_begin = c;
                    }
                }
                else {
                    if(*c == ';') {
                        var_end = c - 1;
                    }
                }
                
                if(var_begin && var_end) {
                    LineVarValue value = {};
                    i32 var_size = (i32)(var_end - var_begin + 1);
                    value.length = var_size;
                    copy_memory(var_begin, value.value, min_value(var_size, size_array(LineVarValue::value)));
                    return value;
                }
            }
        }
        c++;
    }
    return {};
}

static bool 
get_vec_from_value(char *value, f32 *v, i32 v_num) {
    i32 len = (i32)strlen(value);
    if(len < 2 || (value[0] != '{' || value[len - 1] != '}') && 
       (value[0] != '(' || value[len - 1] != ')')) {
        return false;
    }
    
    i32 found = 0;
    char *t = value;
    for(char *c = (value + 1); c <= (value + (len - 1)); ++c) {
        if(*c == ',' || *c == '}' || *c == ')') {
            i32 diff = (i32)((c - 1) - (t + 1));
            char *e_ptr = c - 1 - diff;
            if(found < v_num) {
                v[found] = (f32)atof(e_ptr);
            }
            found++;
            t = c;
        }
    }
    return (found == v_num);
}

#define ASCII_QUOTATION_MARK 34
static bool
get_var_from_file(void *var, char *var_name, var_type type, TextLines lines) {
    bool success = false;
    i32 var_line = search_for_var_line(lines, var_name);
    if(var_line != -1)
    {
        LineVarValue found_value = get_line_var_value(lines.lines[var_line]);
        if(found_value.length) {
            char *value = found_value.value;
            switch(type) {
                case VAR_f32: {
                    *((f32 *)var) = (f32)atof(value);
                } break;
                
                case VAR_i32: {
                    *((i32 *)var) = (i32)atoi(value);
                } break;
                
                case VAR_cstr: { // NOTE: ?
                    i32 value_str_len = (i32)strlen(value);
                    if(value_str_len < 3) {
                        return false;
                    }
                    
                    if(value[0] != ASCII_QUOTATION_MARK || 
                       value[value_str_len - 1] != ASCII_QUOTATION_MARK) {
                        return false;
                    }
                    
                    value_str_len -= 2;
                    value += 1;
                    
                    copy_memory(value, var, value_str_len);
                    ((char *)var)[value_str_len] = '\0';
                } break; 
                
                case VAR_bool: {
                    b32 found = false;
                    for(i32 i = 0; i < size_array(bool_true_strings); ++i) {
                        if(compare_strings(value, bool_true_strings[i])) {
                            *((b32 *)var) = true;
                            found = true;
                            break;
                        }
                    }
                    if(found) break;
                    
                    for(i32 i = 0; i < size_array(bool_false_strings); ++i)
                    {
                        if(compare_strings(value, bool_false_strings[i])) {
                            *((b32 *)var) = false;
                            break;
                        }
                    }
                } break;
                
                case VAR_vec2: {
                    vec2 result = {0};
                    if(get_vec_from_value(value, result.e, 2)) {
                        *((vec2 *)var) = result;
                    }
                } break;
                
                case VAR_vec3: {
                    vec3 result = {0};
                    if(get_vec_from_value(value, result.e, 3)) {
                        *((vec3 *)var) = result;
                    }
                } break;
                
                case VAR_vec4: {
                    vec4 result = {0};
                    if(get_vec_from_value(value, result.e, 4)) {
                        *((vec4 *)var) = result;
                    }
                } break;
                
                default: ASSERT(false, "");
            };
            success = true;
        }
    }
    return success;
}

static bool
load_attached_vars(VarsFile *vars_file, PlatformProcs *procs) {
    bool loaded = false;
    FileContents vars_file_contents;
    if(procs->read_file(&vars_file_contents, vars_file->file_path, true)) {
        vars_file->file_mod_time = procs->last_write_time(vars_file->file_path);
        TextLines file_lines;
        if(get_text_lines(&file_lines, (char *)vars_file_contents.contents)) {
            for(i32 i = 0; i < vars_file->var_count; ++i) {
                VarsFileVar *var = &vars_file->vars[i];
                get_var_from_file(var->ptr, var->name, var->type, file_lines);
            }
        }
        procs->free_file(&vars_file_contents);
        loaded = true;
    }
    return loaded;
}

static bool
hotload_attached_vars(VarsFile *vars_file, PlatformProcs *procs) {
    PTime mod_time = procs->last_write_time(vars_file->file_path);
    if(!p_time_cmp(&mod_time, &vars_file->file_mod_time)) {
        return load_attached_vars(vars_file, procs);
    }
    return false;
}
