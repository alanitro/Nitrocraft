#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_Texcoord;

out vec2 v_Texcoord;

uniform mat4 u_ModelViewProjection;

void main()
{
    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0f);

    v_Texcoord = a_Texcoord;
}
