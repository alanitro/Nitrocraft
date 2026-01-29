#version 460 core

in vec2 v_Texcoord;

out vec4 f_Color;

uniform sampler2D u_Texture;

void main()
{
    //vec4 block_color = texture(u_Texture, v_Texcoord);

    //if (block_color.a == 0.0f) { discard; }

    f_Color = texture(u_Texture, v_Texcoord);
}
