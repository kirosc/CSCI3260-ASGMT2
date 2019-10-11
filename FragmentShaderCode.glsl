#version 430 //GLSL version your computer supports

out vec4 daColor;
in vec3 theColor;

void main()
{
	daColor = vec4(theColor,1.0);
}
