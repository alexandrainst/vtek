
// Taken from tonemapping example
vec3 calcPointLight_NoAttenuation(
	vec3 lightPos, vec3 worldPos, vec4 colorIntensity, vec3 matCol,
	vec3 norm, vec3 viewDir)
{
	// ambient
	vec3 ambient = colorIntensity.xyz * matCol;

	// diffuse
	vec3 lightDir = normalize(lightPos - worldPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = colorIntensity.xyz * diff * matCol;

	// specular
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
	vec3 specular = colorIntensity.xyz * spec * matCol;

	vec3 result = (ambient + diffuse + specular) * colorIntensity.w;
	return result;
}
