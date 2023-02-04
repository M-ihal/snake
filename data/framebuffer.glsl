@vertex #version 330 core

layout(location = 0) in vec3  position;
layout(location = 1) in vec4  color;
layout(location = 2) in vec2  tex_coord;
layout(location = 3) in float tex_slot;
layout(location = 4) in vec2  tiling_factor;

uniform mat4 u_proj;
uniform mat4 u_view;

out vec2  v_tex_coord;
out vec4  v_color;
out float v_tex_slot;
out vec2  v_tiling_factor;

void main() {
	gl_Position = u_proj * u_view * vec4(position, 1.0);
    
    v_tex_coord = tex_coord;
	v_color = color;
	v_tex_slot = tex_slot;
    v_tiling_factor = tiling_factor;
}

@fragment #version 330 core

in vec2  v_tex_coord;
in vec4  v_color;
in float v_tex_slot;
in vec2  v_tiling_factor;

out vec4 f_color;

uniform sampler2D u_textures[32];

uniform float u_reverse_factor;

float clamp(float v, float min, float max) {
    float result = v;
    if(result < min) {
        result = min;
    }
    if(result > max) {
        result = max;
    }
    return result;
}

vec3 lerp_vec3(vec3 a, vec3 b, float t) {
    vec3 result;
    result.r = (1.0 - t) * a.r + t * b.r;
    result.g = (1.0 - t) * a.g + t * b.g;
    result.b = (1.0 - t) * a.b + t * b.b;
    return result;
}

void main() {
	int tex_slot = int(v_tex_slot + 0.5);
	vec4 tex_color = texture(u_textures[tex_slot], v_tex_coord * v_tiling_factor);
	f_color = tex_color * v_color;
    float reverse_factor = clamp(u_reverse_factor, 0.0, 1.0);
    f_color = vec4(lerp_vec3(f_color.rgb, 1.0 - f_color.rgb, reverse_factor), f_color.a);
	if(f_color.a < 0.1) {
		discard;
    }
}