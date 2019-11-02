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
	// Texture
	vec3 objectColor = texture(tex, UV).xyz;

	// SpotLight
	vec3 spotLightPos = vec3(0.0, 0.53, 0.0);
	vec3 spotLightNorm = vec3(0.0, -1.0, 0.0);
	float cutOff = cos(16.0);
	
	vec3 spotLightDir = normalize(spotLightPos - vertexPos);
	float theta = dot(spotLightDir, normalize(spotLightNorm));

	if(theta < cutOff)
	{
		vec3 spotLightColor = vec3(0.7098, 0.1569, 0.6748);
		outputColor = vec4(spotLightColor * objectColor, 1.0); 
	}
	else
	{
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
}
