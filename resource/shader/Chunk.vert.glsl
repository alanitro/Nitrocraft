#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TextureCoordinate;
layout (location = 2) in uint a_Face;

out      vec2  v_TextureCoordinate;
out flat float v_Light;

uniform mat4  u_ModelViewProjection;
uniform float u_SunlightIntensity;

float[6] AMBIENT_FACE_LIGHTS =
{
    0.6f, 0.6f, 0.2f, 1.0f, 0.8f, 0.8f,
};

void main()
{
    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0f);

    v_TextureCoordinate = a_TextureCoordinate;

    float ambient = AMBIENT_FACE_LIGHTS[a_Face] * u_SunlightIntensity;

    v_Light = clamp(ambient, 0.1f, 1.0f);
}
