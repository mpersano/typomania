#version 300 es

precision highp float;

in vec4 frag_color;

out vec4 out_color;

void main(void)
{
	out_color = frag_color;
}
