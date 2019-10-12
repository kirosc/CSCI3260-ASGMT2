#version 430  // GLSL version your computer supports

in vec2 texcoord;
in vec3 position;
in vec3 color;

out vec2 UV;
out vec3 theColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec4 v = vec4(position, 1.0);
	gl_Position = proj * view * model * v;
	UV = texcoord;
	theColor = color;
}