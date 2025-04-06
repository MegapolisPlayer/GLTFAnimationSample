#ifndef GLTF_ANIMATION
#define GLTF_ANIMATION
#include "Mesh.hpp"

struct TRSData {
	glm::vec3 t, s;
	glm::quat r;
	fastgltf::AnimationPath type;
};

struct SamplerData {
	fastgltf::AnimationPath type = (fastgltf::AnimationPath)0;
	std::vector<glm::vec4> value;
	std::vector<GLfloat> time;
	int64_t nodeIndex;

	SamplerData() noexcept : nodeIndex(-1) {}
	SamplerData(const fastgltf::AnimationPath aType, std::vector<glm::vec4>& aValue, std::vector<GLfloat>& aTime, const int64_t aNodeIndex) noexcept
	:type(aType), value(aValue), time(aTime), nodeIndex(aNodeIndex) {}
	~SamplerData() {}
};

class Model;

class Animation {
	friend class Model;
public:
	Animation() noexcept;

	void setStateAtTime(Model& aModel, const float aTime) noexcept;

	~Animation() noexcept;
private:
	std::string mName;

	std::vector<SamplerData> mSamplers;

	float lerp(float aLast, float aNext, float aCurrent) noexcept;
	uint64_t getIndex(const SamplerData& aSampler, const float aTime) noexcept;
	glm::vec3 interpolatePosition(const SamplerData& aSampler, const float aTime) noexcept;
	glm::quat interpolateRotation(const SamplerData& aSampler, const float aTime) noexcept;
	glm::vec3 interpolateScale(const SamplerData& aSampler, const float aTime) noexcept;

	TRSData getLocalSamplerTransform(const uint64_t aSamplerId, const float aTime) noexcept;
};

#endif
