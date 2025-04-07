#version 450 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in float MaterialId;
layout(location = 4) in vec4 BoneIds;
layout(location = 5) in vec4 BoneWeights;

layout(location = 15) uniform mat4 uMatrix;

out vec2 pTexCoord;
flat out float pMaterialId;

layout(std430, binding = 51) readonly buffer sJoints {
	mat4 uJoints[];
};

void main() {
	//copied from https://www.khronos.org/files/gltf20-reference-guide.pdf page 6 bottom right
	//unused bones will have weight 0

	mat4 skinMatrix =
		BoneWeights.x * uJoints[int(BoneIds.x)] +
		BoneWeights.y * uJoints[int(BoneIds.y)] +
		BoneWeights.z * uJoints[int(BoneIds.z)] +
		BoneWeights.w * uJoints[int(BoneIds.w)];

	gl_Position = uMatrix * skinMatrix * vec4(Position, 1.0);
	pTexCoord = TexCoord;
	pMaterialId = MaterialId;
}
