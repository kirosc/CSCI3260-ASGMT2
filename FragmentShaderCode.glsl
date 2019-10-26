#version 430 //GLSL version your computer supports

in vec2 UV;
in vec3 theColor;
in vec3 normalWorld; 
in vec3 vertexPos;

out vec4 outputColor;

uniform sampler2D tex;
uniform vec3 ambient;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
	vec3 objectColor = texture(tex, UV).xyz;

	// Diffuse
	vec3 normal = normalize(normalWorld);
    vec3 lightDir = normalize(lightPos - vertexPos);
    float brightness = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = brightness * lightColor;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(viewPos - vertexPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = spec * lightColor;  

	vec3 phong = (ambient + diffuse + specular) * objectColor;
	outputColor = vec4(phong, 1.0); 
}
