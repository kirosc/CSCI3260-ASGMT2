#version 430  // GLSL version your computer supports

in vec3 position;
in vec3 color;

out vec3 theColor;

void main()
{
	vec4 v = vec4(position, 1.0);
	gl_Position = v;	
	theColor = color;
}