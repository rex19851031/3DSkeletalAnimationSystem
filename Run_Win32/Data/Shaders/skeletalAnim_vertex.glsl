#version 150
#define MAX_BONE_SUPPORT 128
#define MAX_ATTACHMENT_SUPPORT 8

in vec3 a_position;
in vec4 a_color;
in vec2 a_textureCoords;
in int a_boneId0;
in int a_boneId1;
in int a_boneId2;
in int a_boneId3;
in int a_boneId4;
in int a_boneId5;
in int a_boneId6;
in int a_boneId7;
in vec4 a_boneWeights0;
in vec4 a_boneWeights1;

out vec4 v_color; 
out vec2 v_textureCoords;

uniform mat4 u_worldToClipMatrix;
uniform mat4 u_objectToWorldMatrix;
uniform mat4 u_boneMatrixs[MAX_BONE_SUPPORT];


void main(void)
{
	v_color = a_color;
	v_textureCoords = a_textureCoords;
	
	//if(a_boneWeights0.y != 0.0)
	//	v_color = vec4(1,0,1,1);
	
	vec4 vertex = vec4(a_position,1.0);
	vec4 newVertex = vertex;
	
	newVertex = (u_boneMatrixs[a_boneId0] * vertex) * a_boneWeights0.x;
	newVertex = (u_boneMatrixs[a_boneId1] * vertex) * a_boneWeights0.y + newVertex;
	newVertex = (u_boneMatrixs[a_boneId2] * vertex) * a_boneWeights0.z + newVertex;
	newVertex = (u_boneMatrixs[a_boneId3] * vertex) * a_boneWeights0.w + newVertex;
	newVertex = (u_boneMatrixs[a_boneId4] * vertex) * a_boneWeights1.x + newVertex;
	newVertex = (u_boneMatrixs[a_boneId5] * vertex) * a_boneWeights1.y + newVertex;
	newVertex = (u_boneMatrixs[a_boneId6] * vertex) * a_boneWeights1.z + newVertex;
	newVertex = (u_boneMatrixs[a_boneId7] * vertex) * a_boneWeights1.w + newVertex;
	
	gl_Position = u_worldToClipMatrix * u_objectToWorldMatrix * vec4(newVertex.xyz,1.0);
}