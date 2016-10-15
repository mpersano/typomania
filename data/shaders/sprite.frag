#version 300 es

precision highp float;

uniform sampler2D tex;

in vec2 frag_texcoord;
in vec4 frag_color;

out vec4 out_color;

void main(void)
{
	out_color = texture(tex, frag_texcoord)*frag_color;
}
