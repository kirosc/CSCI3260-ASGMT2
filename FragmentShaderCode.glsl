#version 430 //GLSL version your computer supports

in vec2 UV;
in vec3 theColor;

out vec4 daColor;

uniform sampler2D tex;

void main()
{
	daColor = texture(tex, UV);
}
