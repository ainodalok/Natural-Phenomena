//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out vec4 colour;

void main()
{
	vec2 st = gl_FragCoord.xy / vec2(E_W, E_H);
    colour = texture(irradianceDeltaTex, st);
}