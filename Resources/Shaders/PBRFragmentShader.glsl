#version 450 core // Minimal GL version support expected from the GPU
const float PI = 3.14159265358979323846;
const float INV_PI = 0.31830988618379067153776752674503;

struct LightSource {
	int type;
	vec3 center;
	vec3 info; //direction if directional light  ### attenuation params if point light
	vec3 color;
	float intensity;
};

struct Material {
	bool hasAlbedoTex;
	bool hasRoughnessTex;
	bool hasMetallicTex;
	bool hasAmbiantOcclusionTex;
	vec3 albedo;
	float roughness;
	float metallicness;
	sampler2D albedoTex;
	sampler2D roughnessTex;
	sampler2D metallicTex;
	sampler2D ambiantOcclusionTex;
};

uniform LightSource lights[50];
uniform Material material;
uniform mat4 viewMat;
uniform int numOfLights;

in vec3 fNormal; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec2 fTexCoord; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)

out vec4 colorResponse; // Shader output: the color response attached to this fragment

float sqr (float x) { return x*x; }

float GGX (float NdotH, float roughness) {
	if (roughness >= 1.0f) 
		return INV_PI;
	float alpha = sqr (roughness);
	float tmp = alpha / max(1e-8f,(NdotH*NdotH*(sqr (alpha)-1.0f)+1.0f));
	return sqr (tmp) * INV_PI;
}

vec3 SchlickSGFresnel (float VdotH, vec3 F0) {
	float sphg = exp ((-5.55473f*VdotH - 6.98316f) * VdotH);
	return F0 + (vec3(1.0f) - F0) * sphg;
}

float smithG_GGX (float NdotV, float alphaG) {
	return 2.0f/(1.0f + sqrt (1.0f + sqr (alphaG) * (1.0f - sqr (NdotV) / sqr(NdotV))));
}

float G1 (float D, float k) {
	return 1.0f / (D * (1.0f-k) + k);
}

float geometry (float NdotL, float NdotV, float roughness) {
	float k = roughness * roughness * 0.5f;
	return G1(NdotL,k) * G1(NdotV,k);
}

vec3 BRDF (vec3 L, vec3 V, vec3 N,  vec3 albedo, float roughness, float metallic)  {
	vec3 diffuseColor = albedo * (1.0f - metallic);
	vec3 specularColor = mix(vec3(0.08f), albedo, metallic);

	float NdotL = max (0.0f, dot (N, L));
	float NdotV = max (0.0f, dot (N, V));

	if (NdotL <= 0.0f)
		return vec3 (0.0f); 

	vec3 H = normalize (L + V);
	float NdotH = max (0.0f, dot (N, H));
	float VdotH = max (0.0f, dot (V, H));

	float D = GGX (NdotH, roughness);
	vec3  F = SchlickSGFresnel (VdotH, specularColor);
	float G = geometry (NdotL, NdotV, roughness);

	vec3 fd = diffuseColor * (vec3(1.0f)-specularColor) / PI;
	vec3 fs = F * D * G / (4.0f);

	return (fd + fs);
}

vec3 lightRadiance(LightSource l, vec3 position){
	return l.color * l.intensity * PI;
}

vec3 materialReflectance(Material m, vec3 wi, vec3 wo, vec2 fTextCoord, vec3 n) {
	
	vec2 texCoord = fTexCoord; 
	vec3 albedo;
	float roughness;
	float metallicness;
	float ambiantOcclusion;
	
	if (material.hasAlbedoTex)
		albedo = texture(material.albedoTex, texCoord).rgb;
	else
		albedo = material.albedo;

	if (material.hasRoughnessTex)
		roughness = texture(material.roughnessTex, texCoord).r;
	else
		roughness = material.roughness;

	if (material.hasMetallicTex)
		metallicness = texture(material.metallicTex, texCoord).r;
	else
		metallicness = material.metallicness;

	if (material.hasAmbiantOcclusionTex)
		ambiantOcclusion = texture(material.ambiantOcclusionTex, texCoord).r;
	else
		ambiantOcclusion = 1.0f;

	return ambiantOcclusion * BRDF(wi, wo, n, albedo, roughness, metallicness);
}

void main () {
	vec3 wo = - normalize(fPosition);	
	vec3 n = normalize(fNormal);
	
	vec3 res = vec3(0, 0, 0);
	for (int i = 0; i < numOfLights; i++) {
		LightSource l = lights[i];
		vec3 wi;
    		float attenuation;
   		 if (l.type == 0){
       		 	vec3 lp = vec3(viewMat * vec4(l.center,1.0)) - fPosition; 
        		float d = length(lp);
       			wi = normalize(lp);
        		attenuation = 1 / (l.info.x + l.info.y * d + l.info.z * d * d);
        		attenuation = 1 / (l.info.x + l.info.y * d + l.info.z * d * d);

    		}else{
       			wi = -normalize(l.info);
        		attenuation = 1;
    		}

		float wiDotN = max(0.f, dot(wi, n));
		
		if (wiDotN <= 0.f)
			continue;
		
		res += attenuation * lightRadiance(l, fPosition) * materialReflectance(material, wi, wo, fTexCoord, n) * wiDotN;
	}
	colorResponse = vec4(res, 1.0f);
}