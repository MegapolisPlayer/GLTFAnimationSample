#ifndef EUROTRAM_SHADER
#define EUROTRAM_SHADER

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <stbi/stb_image.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/base64.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <chrono>
#include <csignal> //can be helpful to raise interrupts when debugging
#include <clocale>
#include <thread>
#include <list>
#include <filesystem>
#include <numeric>
#include <random>
#include <functional>

using namespace std::chrono_literals;

#define FILE_READ_BLOCK_SIZE 8192

std::string readFile(std::fstream& aStream, const std::string_view aFilepath) noexcept;

std::ostream& operator<<(std::ostream& aStream, const glm::vec2& aVector) noexcept;
std::ostream& operator<<(std::ostream& aStream, const glm::vec3& aVector) noexcept;
std::ostream& operator<<(std::ostream& aStream, const glm::vec4& aVector) noexcept;

std::ostream& operator<<(std::ostream& aStream, const glm::mat4& aMatrix) noexcept;

glm::mat4 convertToGLM(const fastgltf::math::fmat4x4& aFrom) noexcept;
glm::quat convertToGLM(const fastgltf::math::quat<float>& aFrom) noexcept;
glm::vec3 convertToGLM(const fastgltf::math::vec<float, 3> aFrom) noexcept;
glm::vec4 convertToGLM(const fastgltf::math::vec<float, 4> aFrom) noexcept;

class Shader {
public:
	Shader(const std::string_view aVertexSource, const std::string_view aFragmentSource) noexcept;
	Shader(Shader&& aOther) noexcept;
	Shader& operator=(Shader&& aOther) noexcept;
	Shader(Shader& aOther) noexcept = delete;
	Shader& operator=(Shader& aOther) noexcept = delete;

	void bind() noexcept;
	void unbind() noexcept;

	GLuint getHandle() const noexcept;

	~Shader();
private:
	GLuint mHandle;
};

#endif
