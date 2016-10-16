#version 300 es

precision highp float;

uniform vec2 resolution;
uniform sampler2D tex;

in vec2 frag_texcoord;
in vec4 frag_color;

out vec4 out_color;

const vec2 origin = vec2(-20., -10.);
const float feather = 400.;
const float cell_size = 22.;

void main(void)
{
	float level = frag_color.a;

	float l0 = length(origin) - feather;
	float l1 = distance(resolution, origin);

	float lm = mix(l0, l1, level);

	vec2 p = frag_texcoord*resolution;

	vec2 ps = mod(p, cell_size);

	float r = cell_size*smoothstep(lm, lm + feather, distance(p - ps, origin));

	float d = distance(ps, .5*vec2(cell_size, cell_size));
	float t = smoothstep(r - 1., r, d);

	vec4 c = texture(tex, frag_texcoord);

	out_color = vec4(c.rgb, c.a*t);
}
