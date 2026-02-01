#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TextureCoordinate;
layout (location = 2) in uint a_Face;

out      vec2  v_TextureCoordinate;
out flat float v_Light;

uniform mat4  u_ModelViewProjection;

float[6] g_AmbientFaceLights =
{
    0.6f, 0.6f, 0.2f, 1.0f, 0.8f, 0.8f,
};

void main()
{
    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0f);

    v_TextureCoordinate = a_TextureCoordinate;

    v_Light = g_AmbientFaceLights[a_Face];
}
