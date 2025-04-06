#ifndef GLTF_MESH
#define GLTF_MESH
#include "Texture.hpp"

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
	GLfloat materialId;

	glm::vec4 boneIds;
	glm::vec4 boneWeights;
};

struct alignas(16) Material {
	glm::vec4 color = glm::vec4(1.0f);
	GLfloat textureAmount = 1.0f; //1.0 texture only, 0.0 color only
	GLint textureSlot = 0;
	GLfloat textureOpacity = 1.0f;
};

class Mesh {
public:
	Mesh(std::vector<Vertex>& aVerts, std::vector<GLuint>& aInds, const glm::mat4& aTransform) noexcept;
	void draw(const glm::mat4& aProjectionView) noexcept;
	~Mesh() noexcept;
private:
	GLuint mVAO, mVBO, mIBO;
	uint64_t mVertices, mIndices;
	glm::mat4 mTransform;
};

#endif
