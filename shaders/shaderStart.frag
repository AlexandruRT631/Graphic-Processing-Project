#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fPosit;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 viewDirN;
vec3 normalEye;

in vec4 fragPosLightSpace;
uniform sampler2D shadowMap;

float constant = 1.0f;
float linear = 0.5f;
float quadratic = 5.0f;

struct PointLight {
	vec3 position;
	vec3 color;
};
#define NR_POINT_LIGHTS 9
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 computePointLight(PointLight pointLight) {
	vec3 lightDir = normalize(pointLight.position - fPosit.xyz);

	float diff = max(dot(normalEye, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normalEye);
	float spec = pow(max(dot(viewDirN, reflectDir), 0.0), shininess);
	
	// compute distance to light
	float dist = length(pointLight.position - fPosit.xyz);
	// compute attenuation
	float att = 1.0f/(constant + linear * dist + quadratic * dist * dist);

	vec3 ambientPl = ambient * texture(diffuseTexture, fTexCoords).rgb * pointLight.color * att;
	vec3 diffusePl = diffuse * diff * texture(diffuseTexture, fTexCoords).rgb * pointLight.color * att;
	vec3 specularPl = specular * spec * texture(specularTexture, fTexCoords).rgb * pointLight.color * att;

	return ambientPl + diffusePl + specularPl;
}

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir - fPosEye.xyz);
	
	//compute view direction 
	viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow() 
{
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if (normalizedCoords.z > 1.0f)
	{
		return 0.0f;
	}
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
	float bias = 0.001f;
	//float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	float shadow = 0.0f;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (float x = -1.0f; x <= 1.0f; x += 0.25f) {
		for (float y = -1.0f; y <= 1.0f; y += 0.25f) {
			float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
		}
	}
	shadow /= 81.0f;
    return shadow;
}

float computeFog()
{
	float fogDensity = 0.01f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);
	
	vec3 baseColor = vec3(0.3f, 0.3f, 0.3f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
	
    float shadow = computeShadow();

    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
	// vec3 color = min((ambient + diffuse) + specular, 1.0f);
	
	for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		color += computePointLight(pointLights[i]);
	}
    
    // fColor = vec4(color, 1.0f);
	fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
