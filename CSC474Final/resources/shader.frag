#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform float red, green;	// control points

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
color.a=1;
color.rgb = vec3(red,green,0);
}
