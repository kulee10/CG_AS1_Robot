#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;

in vec4 model_color;
in vec4 body_texture;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;

void main()
{
    vec3 texColor = texture(tex,vertexData.texcoord).rgb;
	if (body_texture == vec4(1.0, 1.0, 1.0, 1.0))
		fragColor = vec4(texColor, 1.0);
	else
		fragColor = model_color;
}