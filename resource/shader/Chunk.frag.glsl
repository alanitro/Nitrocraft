#version 460 core

in vec2  v_TextureCoordinate;
in float v_Light;

out vec4 f_Color;

uniform sampler2D u_Texture;

void main()
{
    vec4 block_color = texture(u_Texture, v_TextureCoordinate);

    if (block_color.a == 0.0f) discard;

    f_Color = block_color * v_Light;
}
