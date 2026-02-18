#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TextureCoordinate;
layout (location = 2) in uint a_Face;
layout (location = 3) in uint a_Light;
layout (location = 4) in uint a_AmbientOcclusion;

out vec2  v_TextureCoordinate;
out float v_Light;

uniform mat4  u_ModelViewProjection;
uniform float u_SunlightIntensity;

//float[6] AMBIENT_FACE_LIGHTS =
//{
//    0.05f, 0.05f, 0.02f, 0.1f, 0.08f, 0.08f,
//};

float[4] AMBIENT_OCCLUSION_VALUES =
{
    0.1f, 0.12f, 0.14f, 0.22f,
};

void main()
{
    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0f);

    v_TextureCoordinate = a_TextureCoordinate;

    float sunlight   = (0.4f / 16.0f) * float(((a_Light >> 0) & 0x0F)) * u_SunlightIntensity;
    float pointlight = (0.4f / 12.0f) * float(((a_Light >> 4) & 0x0F));
    float ambient = AMBIENT_OCCLUSION_VALUES[a_AmbientOcclusion];

    v_Light = clamp(sunlight + pointlight + ambient, 0.08f, 1.0f);
}
