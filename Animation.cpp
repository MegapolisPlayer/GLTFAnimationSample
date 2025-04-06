#include "Animation.hpp"
#include "Model.hpp"

Animation::Animation() noexcept  {}

void Animation::setStateAtTime(Model& aModel, const float aTime) noexcept {
	//only calc and update local matrices of nodes
	//rest done in model class

	std::vector<TRSData> data;
	data.reserve(this->mSamplers.size());
	for(uint64_t i = 0; i < this->mSamplers.size(); i++) {
		aModel.mNodes[this->mSamplers[i].nodeIndex].localMatrix = glm::mat4(1.0f);
		data.push_back(this->getLocalSamplerTransform(i, aTime));
	}

	//local matrix reset in Model class
	for(uint64_t i = 0; i < data.size(); i++) {
		auto& node = aModel.mNodes[this->mSamplers[i].nodeIndex];

		switch(data[i].type) {
			case(fastgltf::AnimationPath::Translation):
				node.localMatrix = glm::translate(glm::mat4(1.0f), data[i].t);
				break;
			case(fastgltf::AnimationPath::Rotation):
				node.localMatrix *= glm::mat4_cast(data[i].r);
				break;
			case(fastgltf::AnimationPath::Scale):
				node.localMatrix = glm::scale(node.localMatrix, data[i].s);
				break;
			default:
				std::cerr << "Applying weight animation is not supported! (" << (uint16_t)data[i].type << ")\n";
				break;
		}
	}
}

Animation::~Animation() noexcept {}

float Animation::lerp(float aLast, float aNext, float aCurrent) noexcept {
	if(aCurrent <= aLast) return 0.0; //should not happen
	if(aCurrent >= aNext) return 1.0;
	return ((float)aCurrent - (float)aLast)/((float)aNext - (float)aLast); //should be in range 0-1
}
uint64_t Animation::getIndex(const SamplerData& aSampler, const float aTime) noexcept {
	for(uint64_t i = 0; i < aSampler.time.size()-1; i++) {
		if(aTime < aSampler.time[i+1]) return i;
	}
	return aSampler.time.back();
}
glm::vec3 Animation::interpolatePosition(const SamplerData& aSampler, const float aTime) noexcept {
	uint64_t start = getIndex(aSampler, aTime);
	uint64_t end = start+1;
	float weight = lerp(aSampler.time[start], aSampler.time[end], aTime);
	glm::vec3 position = glm::vec3(glm::mix(aSampler.value[start], aSampler.value[end], weight));
	return position;
}
glm::quat Animation::interpolateRotation(const SamplerData& aSampler, const float aTime) noexcept {
	uint64_t start = getIndex(aSampler, aTime);
	uint64_t end = start+1;
	float weight = lerp(aSampler.time[start], aSampler.time[end], aTime);
	glm::quat qstart = glm::quat(aSampler.value[start].w, aSampler.value[start].x, aSampler.value[start].y, aSampler.value[start].z);
	glm::quat qend = glm::quat(aSampler.value[end].w, aSampler.value[end].x, aSampler.value[end].y, aSampler.value[end].z);
	glm::quat rotation = glm::normalize(glm::slerp(qstart, qend, weight));
	return rotation;
}
glm::vec3 Animation::interpolateScale(const SamplerData& aSampler, const float aTime) noexcept {
	uint64_t start = getIndex(aSampler, aTime);
	uint64_t end = start+1;
	float weight = lerp(aSampler.time[start], aSampler.time[end], aTime);
	glm::vec3 scale = glm::vec3(glm::mix(aSampler.value[start], aSampler.value[end], weight));
	return scale;
}

TRSData Animation::getLocalSamplerTransform(const uint64_t aSamplerId, const float aTime) noexcept {
	auto& sampler = this->mSamplers[aSamplerId];
	TRSData result;
	result.type = sampler.type;

	//TRS enforced by sampler order

	switch(sampler.type) {
		case(fastgltf::AnimationPath::Translation):
			result.t = interpolatePosition(sampler, aTime);
			break;
		case(fastgltf::AnimationPath::Rotation):
			result.r = interpolateRotation(sampler, aTime);
			break;
		case(fastgltf::AnimationPath::Scale):
			result.s = interpolateScale(sampler, aTime);
			break;
		default:
			std::cerr << "Weight animation is not supported!\n";
			break;
	}

	return result;
}
