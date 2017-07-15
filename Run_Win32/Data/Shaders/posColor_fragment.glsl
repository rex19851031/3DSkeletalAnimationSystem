#version 130
in vec4 v_color;

out vec4 fragment_color;

void main(void)
{
	fragment_color = v_color;
}