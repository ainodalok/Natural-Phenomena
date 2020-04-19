//Special GLSL language integration VS extension directive
//! #include "common.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;


layout(location = 0) in int instanceID[3];

//Emit vertices at correct layer of a 3D texture
void main() 
{
    gl_Position = gl_in[0].gl_Position;
    gl_Layer = instanceID[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_Layer = instanceID[1];
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    gl_Layer = instanceID[2];
    EmitVertex();

    EndPrimitive();
}