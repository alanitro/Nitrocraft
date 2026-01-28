#version 460 core

in vec3 v_Color;

out vec4 f_Color;

//uniform sampler2D u_Texture;

void main()
{
    //f_Color = sample(u_Texture, v_Texcoord);

    f_Color = vec4(v_Color, 1.0f);
}
