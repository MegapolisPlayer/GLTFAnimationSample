#include "Shader.hpp"

std::string readFile(std::fstream& aStream, const std::string_view aFilepath) noexcept {
	std::string result;
	result.reserve(FILE_READ_BLOCK_SIZE);
	std::string buffer(FILE_READ_BLOCK_SIZE, '\0');
	aStream.open(aFilepath.data(), std::ios::in | std::ios::binary);
	if(!aStream.is_open()) {
		std::cerr << "Failed to open file " << aFilepath << "! \n";
		return "";
	}

	while(aStream.read(&buffer[0], FILE_READ_BLOCK_SIZE)) {
		result.append(buffer);
		memset(&buffer[0], 0, FILE_READ_BLOCK_SIZE); //do not decrease size; string are contiguous
	};

	result.append(buffer);
	aStream.close();
	return result;
}

std::ostream& operator<<(std::ostream& aStream, const glm::vec2& aVector) noexcept {
	aStream << '[' << aVector.x << ';' << aVector.y << ']';
	return aStream;
}
std::ostream& operator<< (std::ostream& aStream, const glm::vec3& aVector) noexcept {
	aStream << '[' << aVector.x << ';' << aVector.y << ';' << aVector.z << ']';
	return aStream;
}
std::ostream& operator<<(std::ostream& aStream, const glm::vec4& aVector) noexcept {
	aStream << '[' << aVector.x << ';' << aVector.y << ';' << aVector.z << ';' << aVector.w << ']';
	return aStream;
}

glm::mat4 convertToGLM(const fastgltf::math::fmat4x4& aFrom) noexcept {
	glm::mat4 result;

	//no conversion!!!
	result[0][0] = aFrom[0][0]; result[0][1] = aFrom[0][1]; result[0][2] = aFrom[0][2]; result[0][3] = aFrom[0][3];
	result[1][0] = aFrom[1][0]; result[1][1] = aFrom[1][1]; result[1][2] = aFrom[1][2]; result[1][3] = aFrom[1][3];
	result[2][0] = aFrom[2][0]; result[2][1] = aFrom[2][1]; result[2][2] = aFrom[2][2]; result[2][3] = aFrom[2][3];
	result[3][0] = aFrom[3][0]; result[3][1] = aFrom[3][1]; result[3][2] = aFrom[3][2]; result[3][3] = aFrom[3][3];

	return result;
}
glm::quat convertToGLM(const fastgltf::math::quat<float>& aFrom) noexcept {
	return glm::quat(aFrom.w(), aFrom.x(), aFrom.y(), aFrom.z());
}
glm::vec3 convertToGLM(const fastgltf::math::vec<float, 3> aFrom) noexcept {
	return glm::vec3(aFrom.x(), aFrom.y(), aFrom.z());
}
glm::vec4 convertToGLM(const fastgltf::math::vec<float, 4> aFrom) noexcept {
	return glm::vec4(aFrom.x(), aFrom.y(), aFrom.z(), aFrom.w());
}

Shader::Shader(const std::string_view aVertexSource, const std::string_view aFragmentSource) noexcept {
	std::fstream fileLoader;

	this->mHandle = glCreateProgram();
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);

	{
		std::string source = readFile(fileLoader, aVertexSource);
		const char* sourcePtr =	source.c_str();
		GLint sourceSize = source.size();
		glShaderSource(vertex, 1, &sourcePtr, &sourceSize);
		glCompileShader(vertex);
	}
	{
		std::string source = readFile(fileLoader, aFragmentSource);
		const char* sourcePtr =	source.c_str();
		GLint sourceSize = source.size();
		glShaderSource(fragment, 1, &sourcePtr, &sourceSize);
		glCompileShader(fragment);
	}

	glAttachShader(this->mHandle, vertex);
	glAttachShader(this->mHandle, fragment);

	glLinkProgram(this->mHandle);
	glValidateProgram(this->mHandle);

	bool failed = false;

	GLint success = 0;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		failed = true;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &success);
		std::string message;
		message.resize(success);
		glGetShaderInfoLog(vertex, GL_INFO_LOG_LENGTH, NULL, &message[0]);
		std::cout << aVertexSource << " - Vertex shader compilation failed: " << message << '\n';
	}
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		failed = true;
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &success);
		std::string message;
		message.resize(success);
		glGetShaderInfoLog(fragment, GL_INFO_LOG_LENGTH, NULL, &message[0]);
		std::cout << aFragmentSource << " - Fragment shader compilation failed: " << message << '\n';
	}
	glGetProgramiv(this->mHandle, GL_LINK_STATUS, &success);
	if(success == GL_FALSE) {
		failed = true;
		glGetProgramiv(this->mHandle, GL_INFO_LOG_LENGTH, &success);
		std::string message;
		message.resize(success);
		glGetProgramInfoLog(this->mHandle, GL_INFO_LOG_LENGTH, NULL, &message[0]);
		std::cout << "Shader link failed: " << message << '\n';
	}

	//first print all messages, then exit
	if(failed) { std::exit(EXIT_FAILURE); }

	glDetachShader(this->mHandle, vertex);
	glDetachShader(this->mHandle, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	glUseProgram(this->mHandle);
}

Shader::Shader(Shader&& aOther) noexcept {
	this->mHandle = aOther.mHandle;
	aOther.mHandle = 0;
}
Shader& Shader::operator=(Shader&& aOther) noexcept {
	this->mHandle = aOther.mHandle;
	aOther.mHandle = 0;
	return *this;
}

void Shader::bind() noexcept {
	glUseProgram(this->mHandle);
}
void Shader::unbind() noexcept {
	glUseProgram(0);
}
GLuint Shader::getHandle() const noexcept {
	return this->mHandle;
}
Shader::~Shader() {
	glDeleteProgram(this->mHandle);
}
