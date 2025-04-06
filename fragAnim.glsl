#version 450 core

out vec4 oColor;

in vec2 pTexCoord;
flat in float pMaterialId;

layout(location = 16) uniform sampler2D uTextures[32];

struct Material {
	vec4 color;
	float textureAmount;
	int textureSlot;
	float textureOpacity;
	//16 byte aligned
};

layout(std430, binding = 50) readonly buffer sMaterials {
	Material mat[];
};

void main() {
	vec3 temp = mix(mat[int(pMaterialId)].color.rgb, texture(uTextures[mat[int(pMaterialId)].textureSlot], pTexCoord).rgb, mat[int(pMaterialId)].textureAmount);
	oColor = vec4(temp, 1.0);
}
