#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4p;	// projection matrix
uniform mat4 um4v;	// camera viewing transformation matrix
uniform mat4 um4m;	// rotation matrix
uniform vec4 part_color;
uniform vec4 tex_color;


out vec4 model_color;
out vec4 body_texture;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

void main()
{
	gl_Position = um4p * um4v * um4m * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
	model_color = part_color;
	body_texture = tex_color;
}