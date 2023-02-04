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

void main()
{
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

void main()
{
	int tex_slot = int(v_tex_slot + 0.5);
	vec4 tex_color = texture(u_textures[tex_slot], v_tex_coord * v_tiling_factor);
	f_color = tex_color * v_color;
}