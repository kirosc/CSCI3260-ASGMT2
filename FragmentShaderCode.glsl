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

vec3 phong;

void main()
{
	// Texture
	vec3 objectColor = texture(tex, UV).xyz;
	// Light Color
	vec3 mLightColor = lightColor;

	// SpotLight
	vec3 spotLightPos = vec3(0.0, 0.7, 0.0);
	vec3 spotLightNorm = vec3(0.0, -1.0, 0.0);
	float cutOff = cos(2.85);
	float outerCutOff = cos(2.9);
	
	vec3 spotLightDir = normalize(spotLightPos - vertexPos);
	float theta = dot(spotLightDir, normalize(spotLightNorm));
	float epsilon = cutOff - outerCutOff;
	float intensity = clamp((outerCutOff - theta) / epsilon, 0.0, 1.0);    

	// Diffuse
	vec3 normal = normalize(normalWorld);
	vec3 lightDir = normalize(lightPos - vertexPos);
	float brightness = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = brightness * mLightColor;

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(viewPos - vertexPos);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = spec * mLightColor;

	if(theta < cutOff)
	{
		// Inside spot light
		// Make it brighter and smooth out the edges
		vec3 spotLightDiffuse = (diffuse + 0.5) * intensity;
		vec3 spotLightSpecular = specular * intensity;
		phong = (ambient + diffuse + specular + spotLightDiffuse + spotLightSpecular) * objectColor;
	}
	else
	{
		phong = (ambient + diffuse + specular) * objectColor;
	}

	outputColor = vec4(phong, 1.0);
}
