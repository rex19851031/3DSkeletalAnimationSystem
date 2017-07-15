#version 130
in vec3 a_position;
in vec4 a_color; 

out vec4 v_color;

uniform mat4 u_worldToClipMatrix;
uniform mat4 u_objectToWorldMatrix;

void main(void)
{
	v_color = a_color;
	gl_Position = u_worldToClipMatrix * u_objectToWorldMatrix * vec4(a_position,1.0);
}