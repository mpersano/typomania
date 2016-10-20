#version 300 es

precision highp float;

uniform sampler2D tex;

const float resolution = 256.;

in vec2 frag_texcoord;
in vec4 frag_color;

out vec4 out_color;

void main()
{
	vec2 direction = frag_color.xy;

	vec2 d = direction/resolution;

	float c =  texture(tex, frag_texcoord - 7.*d).a*0.0044299121055113265
		 + texture(tex, frag_texcoord - 6.*d).a*0.00895781211794
		 + texture(tex, frag_texcoord - 5.*d).a*0.0215963866053
		 + texture(tex, frag_texcoord - 4.*d).a*0.0443683338718
		 + texture(tex, frag_texcoord - 3.*d).a*0.0776744219933
		 + texture(tex, frag_texcoord - 2.*d).a*0.115876621105
		 + texture(tex, frag_texcoord - 1.*d).a*0.147308056121
		 + texture(tex, frag_texcoord       ).a*0.159576912161
		 + texture(tex, frag_texcoord + 1.*d).a*0.147308056121
		 + texture(tex, frag_texcoord + 2.*d).a*0.115876621105
		 + texture(tex, frag_texcoord + 3.*d).a*0.0776744219933
		 + texture(tex, frag_texcoord + 4.*d).a*0.0443683338718
		 + texture(tex, frag_texcoord + 5.*d).a*0.0215963866053
		 + texture(tex, frag_texcoord + 6.*d).a*0.00895781211794
		 + texture(tex, frag_texcoord + 7.*d).a*0.0044299121055113265;

	out_color = vec4(c);
}
