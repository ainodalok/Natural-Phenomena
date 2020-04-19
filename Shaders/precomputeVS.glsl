//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(location = 0) out int instanceID;

void main()
{
	vec2 st = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(st * 2.0f + -1.0f, 0.0f, 1.0f);
	instanceID = gl_InstanceID;
}