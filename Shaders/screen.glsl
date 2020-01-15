#ifdef VERTEX
layout(location = 0) out vec2 uv;

void main()
{
	uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);
}
#endif

#ifdef FRAGMENT
layout(binding = COUNT) uniform sampler2D screenTexture;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = vec4(vec3(texture(screenTexture, uv)), 1.0);
}
#endif