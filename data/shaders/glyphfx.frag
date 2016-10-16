#version 300 es

precision highp float;

uniform sampler2D tex;

in vec2 frag_texcoord;
in vec4 frag_color;

out vec4 out_color;

void main(void)
{
	vec4 c = texture(tex, frag_texcoord);
	out_color = c.a*frag_color;
}
