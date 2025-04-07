#ifndef GLTF_MODELLOAD
#define GLTF_MODELLOAD
#include "Mesh.hpp"
#include "Animation.hpp"

struct Node {
	std::string name;
	glm::mat4 animationMatrix;
	glm::mat4 localMatrix;
	glm::mat4 transformMatrix;
	int64_t idOfSkin;
	int64_t meshId;

	std::vector<uint64_t> children;
	int64_t parent = -1;

	uint64_t amountOfJoints = 0;
	uint64_t jointsIdOffset = 0;

	Node() noexcept {};
	Node(const std::string& aName, const glm::mat4& aGlobal, const glm::mat4& aLocal, const int64_t aIdOfSkin) noexcept
	: name(aName), transformMatrix(aGlobal), localMatrix(aLocal), idOfSkin(aIdOfSkin) {};
	~Node() noexcept {};
};

struct Bone {
	std::string name;
	std::vector<uint64_t> joints;
	std::vector<glm::mat4> inverseBindMatrix;
};

class Model {
	friend class Animation;
public:
	Model(const std::filesystem::path& aPath) noexcept;

	void draw(const glm::mat4& aProjectionView) noexcept;
	void setStateAtTime(uint64_t aId, float aTime) noexcept;

	uint64_t getAnimationAmount() const noexcept;

	~Model() noexcept;
private:
	std::vector<uint64_t> mRootNodes;

	std::vector<Mesh> mMeshes;
	std::vector<Node> mNodes;
	std::vector<Bone> mBones;
	std::vector<Material> mMaterials;
	std::vector<Texture> mTextures;
	std::vector<Animation> mAnimations;

	GLuint mMaterialBuffer;
	GLuint mJointMatrixBuffer;
	size_t mJointsAmount;

	//workaround: joint ID bound to node, we want to store in array
	//get order of node, add offset
	void getNodeJointAmount();
	void getNodeJointOffset(uint64_t aId, uint64_t* aCurrentOffset); //call AFTER getting amount

	glm::mat4 getNodeMatrix(Node& node);
	void updateJoints(Node& node, std::vector<glm::mat4>& aJointMatrices);
};

#endif
