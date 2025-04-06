#include "Mesh.hpp"

Mesh::Mesh(std::vector<Vertex>& aVerts, std::vector<GLuint>& aInds, const glm::mat4& aTransform) noexcept {
	this->mTransform = aTransform;

	glGenVertexArrays(1, &this->mVAO);
	glBindVertexArray(this->mVAO);

	this->mVertices = aVerts.size();
	this->mIndices = aInds.size();

	glGenBuffers(1, &this->mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->mVBO);
	glBufferData(GL_ARRAY_BUFFER, aVerts.size()*sizeof(Vertex), aVerts.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, materialId));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, boneIds));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, boneWeights));
	glEnableVertexAttribArray(5);

	glGenBuffers(1, &this->mIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->mIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, aInds.size()*sizeof(GLuint), aInds.data(), GL_STATIC_DRAW);
}
void Mesh::draw(const glm::mat4& aProjectionView) noexcept {
	glUniformMatrix4fv(15, 1, GL_FALSE, glm::value_ptr(aProjectionView*this->mTransform));

	glBindVertexArray(this->mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->mVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->mIBO);
	glDrawElements(GL_TRIANGLES, this->mIndices, GL_UNSIGNED_INT, nullptr);
}
Mesh::~Mesh() noexcept {

}
