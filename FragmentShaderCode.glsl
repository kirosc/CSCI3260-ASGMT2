#version 430 //GLSL version your computer supports

in vec2 UV;
in vec3 theColor;

out vec4 outputColor;

uniform sampler2D tex;
uniform vec3 ambient;

void main()
{
	vec3 objectColor = texture(tex, UV).xyz;
	vec3 phong = ambient * objectColor;
	outputColor = vec4(phong, 1.0f); 
}
