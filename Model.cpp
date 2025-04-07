#include "Model.hpp"

Model::Model(const std::filesystem::path& aPath) noexcept {
	constexpr auto extensions =
	fastgltf::Extensions::KHR_materials_ior |
	fastgltf::Extensions::KHR_materials_specular |
	fastgltf::Extensions::KHR_materials_emissive_strength |
	fastgltf::Extensions::KHR_materials_sheen
	;

	auto gdb = fastgltf::GltfDataBuffer::FromPath(aPath);
	if(!gdb) {
		std::cerr << "Error: GLTF2 model load failed!\n";
		return;
	}
	fastgltf::Parser parse(extensions);

	constexpr auto importOptions =
	fastgltf::Options::DontRequireValidAssetMember |
	fastgltf::Options::LoadExternalBuffers |
	fastgltf::Options::LoadExternalImages |
	fastgltf::Options::GenerateMeshIndices |
	fastgltf::Options::DecomposeNodeMatrices
	;

	//we need to pass directory
	auto loadState = parse.loadGltfBinary(gdb.get(), aPath.parent_path(), importOptions);
	fastgltf::Asset* model = loadState.get_if();
	if(!model) {
		std::cerr << "Error: GLTF2 model data load failed! " <<
		fastgltf::getErrorMessage(loadState.error()) <<
		"(parent path: " <<aPath.parent_path() << ", current: " << aPath << ")\n";
		return;
	}

	//material
	uint64_t textureId = 0;
	for(fastgltf::Material& m : model->materials) {
		std::cout << "Material: " << m.name.c_str() << '\n';

		this->mMaterials.emplace_back();
		this->mMaterials.back().color = glm::vec4(1, 0, 1, 1);
		this->mMaterials.back().color = convertToGLM(m.pbrData.baseColorFactor);
		this->mMaterials.back().textureAmount = 0.0f;
		this->mMaterials.back().textureSlot = 0;
		this->mMaterials.back().textureOpacity = 1.0f;

		auto& texInfo = m.pbrData.baseColorTexture;
		if(texInfo.has_value()) {
			this->mMaterials.back().textureAmount = 1.0f;
			auto& texture = model->textures[texInfo->textureIndex];
			if(texture.imageIndex.has_value()) {
				auto& image = model->images[texture.imageIndex.value()];
				this->mMaterials.back().textureSlot = textureId;
				textureId++;
				if(textureId >= 32) {
					std::cerr << "Error: more than 32 material textures per model are not supported... yet.\n";
					return;
				}

				//for each variant possibility - call correct function

				fastgltf::sources::URI* uriData = std::get_if<fastgltf::sources::URI>(&image.data);
				fastgltf::sources::Array* arrayData = std::get_if<fastgltf::sources::Array>(&image.data);
				fastgltf::sources::BufferView* bufData = std::get_if<fastgltf::sources::BufferView>(&image.data);

				if(arrayData) {
					this->mTextures.push_back(Texture(arrayData->bytes.data(), arrayData->bytes.size()));
				}
				else if(uriData) {
					std::cerr << "Error: texture type not supported!\n";
				}
				else if(bufData) {
					//every texture loaded here...

					auto& bufferView = model->bufferViews[bufData->bufferViewIndex];
					auto& buffer = model->buffers[bufferView.bufferIndex];

					std::visit(fastgltf::visitor {
						[&](auto& arg) {},
							   [&](fastgltf::sources::Array& aData) {
								   this->mTextures.push_back(Texture(aData.bytes.data() + bufferView.byteOffset, bufferView.byteLength));
							   }
					}, buffer.data);
				}
				else {
					std::cerr << "Error: texture type undefined!\n";
				}
			}
		}
	}

	std::vector<size_t> meshNodeAccess;
	meshNodeAccess.resize(model->meshes.size());

	//nodes
	for(fastgltf::Node& node : model->nodes) {
		fastgltf::TRS* nt = std::get_if<fastgltf::TRS>(&node.transform);
		glm::mat4 nodeTransform = glm::scale(glm::mat4(1.0), convertToGLM(nt->scale));
		nodeTransform *= glm::mat4_cast(convertToGLM(nt->rotation));
		nodeTransform = glm::translate(nodeTransform, convertToGLM(nt->translation));

		if(node.skinIndex.has_value()) {
			this->mNodes.emplace_back(node.name.c_str(), convertToGLM(fastgltf::getTransformMatrix(node)), nodeTransform, node.skinIndex.value());
		}
		else {
			this->mNodes.emplace_back(node.name.c_str(), convertToGLM(fastgltf::getTransformMatrix(node)), nodeTransform, -1);
		}

		if(node.meshIndex.has_value()) {
			this->mNodes.back().meshId = node.meshIndex.value();
			meshNodeAccess[this->mNodes.back().meshId] = this->mNodes.size()-1;
		}

		std::cout << node.name.c_str() << ' ' << this->mNodes.size()-1 << ')';
		for(auto& c : node.children) {
			std::cout << ' ' << model->nodes[c].name.c_str();
			this->mNodes.back().children.push_back(c);
		}
		std::cout << ' ' << this->mNodes.back().transformMatrix * glm::vec4(1.0) << '\n';
	};

	//set node parent attribute
	for(uint64_t i = 0; i < this->mNodes.size(); i++) {
		for(int64_t c : this->mNodes[i].children) {
			this->mNodes[c].parent = i; //parent of our children is us
			std::cout << this->mNodes[c].name << "->" << this->mNodes[i].name << '\n';
		}
	}

	//process bones
	for(fastgltf::Skin& s : model->skins) {
		this->mBones.push_back({});
		Bone& writeSkin = this->mBones.back();
		writeSkin.name = s.name.c_str();

		for(size_t j : s.joints) {
			writeSkin.joints.push_back(j); //joints are nodes!
		}

		if(s.inverseBindMatrices.has_value()) {
			fastgltf::Accessor& ibmAccess =  model->accessors[s.inverseBindMatrices.value()];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fmat4x4>(*model, ibmAccess, [&](fastgltf::math::fmat4x4 aV, GLuint aId) {
				writeSkin.inverseBindMatrix.push_back(convertToGLM(aV));
			});
		}
	}

	//root nodes (children of non-node root)
	std::cout << "Root nodes ";
	for(size_t nid : model->scenes[model->defaultScene.value()].nodeIndices) {
		this->mRootNodes.push_back(nid);
		std::cout << nid << " ";
	}
	std::cout << std::endl;

	this->getNodeJointAmount();
	//get offsets for joint matrices (ids bound to node)
	uint64_t curOff = 0;
	for(size_t nid : this->mRootNodes) {
		this->getNodeJointOffset(nid, &curOff);
	}

	std::cout << "Joint offsets ";
	for(Node& n : this->mNodes) std::cout << n.jointsIdOffset << " ";
	std::cout << std::endl;

	std::cout << "Joint amounts ";
	for(Node& n : this->mNodes) std::cout << n.amountOfJoints << " ";
	std::cout << std::endl;

	uint64_t jointMatricesAmount = 0;
	for(Node& n : this->mNodes) jointMatricesAmount += n.amountOfJoints;

	std::cout << "Total joint amount: " << jointMatricesAmount << '\n';
	glGenBuffers(1, &this->mJointMatrixBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->mJointMatrixBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, jointMatricesAmount*sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
	this->mJointsAmount = jointMatricesAmount;

	//meshes
	uint64_t meshNodeAccessorId = 0;
	for(fastgltf::Mesh& m : model->meshes) {
		//aliases
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		for(fastgltf::Primitive& p : m.primitives) {
			size_t initialId = vertices.size();
			uint64_t matIndex = 32;
			if(p.materialIndex.has_value()) {
				matIndex = p.materialIndex.value();
			}

			//indices
			{
				fastgltf::Accessor& indicesAccess = model->accessors[p.indicesAccessor.value()];
				indices.reserve(indices.size() + indicesAccess.count);
				fastgltf::iterateAccessor<GLuint>(*model, indicesAccess, [&](GLuint aId) {
					indices.push_back(aId + initialId);
				});
			}

			//position + material index
			{
				fastgltf::Accessor& verticesAccess = model->accessors[p.findAttribute("POSITION")->accessorIndex];
				vertices.resize(vertices.size() + verticesAccess.count);
				fastgltf::iterateAccessorWithIndex<glm::vec3>(*model, verticesAccess, [&](glm::vec3 aV, GLuint aId) {
					assert(initialId+aId < vertices.size());
					vertices[initialId+aId].position = aV;
					vertices[initialId+aId].materialId = matIndex;
				});
			}

			//normals
			{
				if(p.findAttribute("NORMAL") != p.attributes.end()) {
					fastgltf::Accessor& normalAccess = model->accessors[p.findAttribute("NORMAL")->accessorIndex];
					fastgltf::iterateAccessorWithIndex<glm::vec3>(*model, normalAccess, [&](glm::vec3 aV, GLuint aId) {
						assert(initialId+aId < vertices.size());
						vertices[initialId+aId].normal = aV;
					});
				}
			}

			//UVs
			{
				if(p.findAttribute("TEXCOORD_0") != p.attributes.end()) {
					fastgltf::Accessor& uvAccess = model->accessors[p.findAttribute("TEXCOORD_0")->accessorIndex];
					fastgltf::iterateAccessorWithIndex<glm::vec2>(*model, uvAccess, [&](glm::vec2 aV, GLuint aId) {
						assert(initialId+aId < vertices.size());
						vertices[initialId+aId].texCoords.x = aV.x;
						vertices[initialId+aId].texCoords.y = 1.0 - aV.y; //flipping UVs Y simpler than flipping every image!
					});
				}
			}

			//joints
			{
				if(p.findAttribute("JOINTS_0") != p.attributes.end()) {
					fastgltf::Accessor& jointsAccess = model->accessors[p.findAttribute("JOINTS_0")->accessorIndex];
					if(p.findAttribute("JOINTS_0") != p.attributes.end()) {
						fastgltf::iterateAccessorWithIndex<fastgltf::math::u8vec4>(*model, jointsAccess, [&](fastgltf::math::u8vec4 aV, GLuint aId) {
							vertices[initialId+aId].boneIds[0] = aV.x()+this->mNodes[meshNodeAccess[meshNodeAccessorId]].jointsIdOffset;
							vertices[initialId+aId].boneIds[1] = aV.y()+this->mNodes[meshNodeAccess[meshNodeAccessorId]].jointsIdOffset;
							vertices[initialId+aId].boneIds[2] = aV.z()+this->mNodes[meshNodeAccess[meshNodeAccessorId]].jointsIdOffset;
							vertices[initialId+aId].boneIds[3] = aV.w()+this->mNodes[meshNodeAccess[meshNodeAccessorId]].jointsIdOffset;
						});
					}
				}
			}

			//weights
			{
				if(p.findAttribute("WEIGHTS_0") != p.attributes.end()) {
					fastgltf::Accessor& jointsAccess = model->accessors[p.findAttribute("WEIGHTS_0")->accessorIndex];
					if(p.findAttribute("WEIGHTS_0") != p.attributes.end()) {
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(*model, jointsAccess, [&](fastgltf::math::fvec4 aV, GLuint aId) {
							vertices[initialId+aId].boneWeights[0] = aV.x();
							vertices[initialId+aId].boneWeights[1] = aV.y();
							vertices[initialId+aId].boneWeights[2] = aV.z();
							vertices[initialId+aId].boneWeights[3] = aV.w();
						});
					}
				}
			}
		}

		//mesh names are non-descriptive usually, use node names (1 node can only have 1 mesh and vice versa)
		std::cout << "Mesh name: " << this->mNodes[meshNodeAccess[meshNodeAccessorId]].name << '\n';
		this->mMeshes.emplace_back(vertices, indices, this->mNodes[meshNodeAccess[meshNodeAccessorId]].transformMatrix);
		meshNodeAccessorId++;
	}

	glGenBuffers(1, &this->mMaterialBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->mMaterialBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->mMaterials.size()*sizeof(Material), this->mMaterials.data(), GL_STATIC_DRAW);

	//process animations
	for(fastgltf::Animation& a : model->animations) {
		this->mAnimations.push_back({});
		auto& anim = this->mAnimations.back();
		anim.mName = a.name.c_str();
		std::cout << "Found animation: " << a.name.c_str() << '\n';

		for(fastgltf::AnimationSampler& s : a.samplers) {
			anim.mSamplers.push_back({});

			fastgltf::Accessor& samplerInputAccess = model->accessors[s.inputAccessor]; //time
			fastgltf::Accessor& samplerOutputAccess = model->accessors[s.outputAccessor]; //value

			//input - keyframe times

			fastgltf::iterateAccessor<GLfloat>(*model, samplerInputAccess, [&](float aV) {
				anim.mSamplers.back().time.push_back(aV);
			});

			//output - property (vec3 for transform, scale - vec4 for rotation quaternion)
			if(samplerOutputAccess.type == fastgltf::AccessorType::Vec3) {
				fastgltf::iterateAccessor<glm::vec3>(*model, samplerOutputAccess, [&](glm::vec3 aV) {
					anim.mSamplers.back().value.push_back(glm::vec4(aV, 1.0));
				});
			}
			else if(samplerOutputAccess.type == fastgltf::AccessorType::Vec4) {
				fastgltf::iterateAccessor<glm::vec4>(*model, samplerOutputAccess, [&](glm::vec4 aV) {
					anim.mSamplers.back().value.push_back(aV);
				});
			}
			else {
				std::cerr << "Wrong type!\n";
			} //should never happen, GLTF stores "keyframes" only as vec3 or vec4
		}
		for(fastgltf::AnimationChannel& c : a.channels) {
			anim.mSamplers[c.samplerIndex].type = c.path;
			if(c.nodeIndex.has_value()) {
				anim.mSamplers[c.samplerIndex].nodeIndex = c.nodeIndex.value();
			}
			else {
				anim.mSamplers[c.samplerIndex].nodeIndex = -1;
			}
		}
	}
}

//id is bound to node -> every node will have offset
void Model::getNodeJointAmount() {
	for(uint64_t nodeId = 0; nodeId < this->mNodes.size(); nodeId++) {
		if (this->mNodes[nodeId].idOfSkin > -1) {
			Bone      skin             = this->mBones[this->mNodes[nodeId].idOfSkin];
			size_t    numJoints        = (uint32_t)skin.joints.size();

			this->mNodes[nodeId].amountOfJoints = numJoints;
		}
	}
}

//same structure as updateJoints
void Model::getNodeJointOffset(uint64_t aId, uint64_t* aCurrentOffset) {
	this->mNodes[aId].jointsIdOffset = *aCurrentOffset;
	*aCurrentOffset += this->mNodes[aId].amountOfJoints;

	for(auto& child : this->mNodes[aId].children) this->getNodeJointOffset(child, aCurrentOffset);
}

//https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
//https://www.khronos.org/files/gltf20-reference-guide.pdf
//https://github.com/SaschaWillems/Vulkan/blob/master/examples/gltfskinning/README.md
//https://github.com/SaschaWillems/Vulkan/blob/master/examples/gltfskinning/gltfskinning.cpp

glm::mat4 Model::getNodeMatrix(Node& node) {
	glm::mat4 nodeMatrix = node.localMatrix;
	int64_t currentParent = node.parent;
	while (currentParent != -1) {
		nodeMatrix = this->mNodes[currentParent].localMatrix * nodeMatrix;
		currentParent = this->mNodes[currentParent].parent;
	}
	return nodeMatrix;
}

void Model::updateJoints(Node& node, std::vector<glm::mat4>& aJointMatrices) {
	if (node.idOfSkin > -1) {
		glm::mat4              inverseTransform = glm::inverse(node.transformMatrix);
		Bone                   skin             = this->mBones[node.idOfSkin];
		size_t                 numJoints        = (uint32_t)skin.joints.size();
		std::vector<glm::mat4> jointMatrices(numJoints);
		for (size_t i = 0; i < numJoints; i++) {
			//do NOT set transform matrix anew
			jointMatrices[i] = getNodeMatrix(this->mNodes[skin.joints[i]]) * skin.inverseBindMatrix[i];
			jointMatrices[i] = inverseTransform * jointMatrices[i];
		}

		aJointMatrices.insert(aJointMatrices.end(), jointMatrices.begin(), jointMatrices.end());
	}

	for(auto& child : node.children) updateJoints(this->mNodes[child], aJointMatrices);
}

void Model::draw(const glm::mat4& aProjectionView) noexcept {
	for(uint64_t i = 0; i < this->mTextures.size(); i++)
		this->mTextures[i].bind(i);

	std::vector<glm::mat4> jointMatrices;
	for(size_t id : this->mRootNodes) {
		this->updateJoints(this->mNodes[id], jointMatrices);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->mJointMatrixBuffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, jointMatrices.size()*sizeof(glm::mat4), jointMatrices.data());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 51, this->mJointMatrixBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 50, this->mMaterialBuffer);

	for(Mesh& m : this->mMeshes) m.draw(aProjectionView);
}

//BUG when changing animations doesnt work!

void Model::setStateAtTime(uint64_t aId, float aTime) noexcept {
	 this->mAnimations[aId].setStateAtTime(*this, aTime);
}
uint64_t Model::getAnimationAmount() const noexcept {
	return this->mAnimations.size();
}
Model::~Model() noexcept {}
