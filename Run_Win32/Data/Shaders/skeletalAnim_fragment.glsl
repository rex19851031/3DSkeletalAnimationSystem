#version 130
in vec4 v_color;
in vec2 v_textureCoords;

uniform sampler2D u_diffuseMap;

out vec4 fragment_color;

void main(void)
{
	fragment_color = texture2D( u_diffuseMap , v_textureCoords ) * v_color;
	//fragment_color = v_color;
}