#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Texture.hpp"

Texture::Texture(const std::string_view aFilename, const bool aFlip, TextureScale aScaling, TextureBorder aBorder) noexcept
: mHandle(0), mpData(nullptr), mPath(aFilename), mWidth(0), mHeight(0), mChannels(0) {
	if(this->mPath.empty()) {
		return;
	}

	GLint glTextureScaleValue = 0;
	GLint glTextureScaleValue2 = 0;
	switch(aScaling) {
		case(TextureScale::LINEAR):
			glTextureScaleValue = GL_LINEAR;
			glTextureScaleValue2 = GL_LINEAR_MIPMAP_LINEAR;
			break;
		case(TextureScale::NEAREST_NEIGHBOR):
		default:
			glTextureScaleValue = GL_NEAREST;
			glTextureScaleValue2 = GL_NEAREST;
			break;
	}

	GLint glTextureBorderValue = 0;
	switch(aBorder) {
		case(TextureBorder::FILL_OUT_OF_RANGE):
			glTextureBorderValue = GL_CLAMP_TO_BORDER;
			break;
		case(TextureBorder::REPEAT):
		default:
			glTextureBorderValue = GL_REPEAT;
			break;
	}

	glGenTextures(1, &this->mHandle);
	glBindTexture(GL_TEXTURE_2D, this->mHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glTextureBorderValue);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glTextureBorderValue);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glTextureScaleValue2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glTextureScaleValue);

	stbi_set_flip_vertically_on_load(aFlip);
	this->mpData = stbi_load(mPath.data(), &this->mWidth, &this->mHeight, &this->mChannels, 4);
	if(!this->mpData) {
		std::cerr << "STBI failed to load image " << aFilename << "!\n";
		return;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->mWidth, this->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->mpData);
	glGenerateMipmap(GL_TEXTURE_2D);
}
Texture::Texture(
	const uint64_t aWidth, const uint64_t aHeight,
	const uint64_t aInternalFormat, const uint64_t aFormat, const uint64_t aDataType,
	const bool aNoMipmaps, TextureScale aScaling, TextureBorder aBorder) noexcept
	: mPath(""), mHandle(0), mpData(nullptr), mWidth(aWidth), mHeight(aHeight), mChannels(0) {
		GLint glTextureScaleValue = 0;
		GLint glTextureScaleValue2 = 0;
		switch(aScaling) {
			case(TextureScale::LINEAR):
				glTextureScaleValue = GL_LINEAR;
				if(aNoMipmaps) {
					glTextureScaleValue2 = GL_LINEAR;
				}
				else {
					glTextureScaleValue2 = GL_LINEAR_MIPMAP_LINEAR;
				}
				break;
			case(TextureScale::NEAREST_NEIGHBOR):
			default:
				glTextureScaleValue = GL_NEAREST;
				glTextureScaleValue2 = GL_NEAREST;
				break;
		}

		GLint glTextureBorderValue = 0;
		switch(aBorder) {
			case(TextureBorder::FILL_OUT_OF_RANGE):
				glTextureBorderValue = GL_CLAMP_TO_BORDER;
				break;
			case(TextureBorder::REPEAT):
			default:
				glTextureBorderValue = GL_REPEAT;
				break;
		}

		glGenTextures(1, &this->mHandle);
		glBindTexture(GL_TEXTURE_2D, this->mHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, aInternalFormat, this->mWidth, this->mHeight, 0, aFormat, aDataType, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glTextureBorderValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glTextureBorderValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glTextureScaleValue2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glTextureScaleValue);
	}

Texture::Texture(void* aData, const size_t aPixelAmount, const bool aFlip, TextureScale aScaling, TextureBorder aBorder) noexcept
	: mPath(""), mHandle(0), mpData(nullptr), mChannels(4) {
		GLint glTextureScaleValue = 0;
		GLint glTextureScaleValue2 = 0;
		switch(aScaling) {
			case(TextureScale::LINEAR):
				glTextureScaleValue = GL_LINEAR;
				glTextureScaleValue2 = GL_LINEAR_MIPMAP_LINEAR;
				break;
			case(TextureScale::NEAREST_NEIGHBOR):
			default:
				glTextureScaleValue = GL_NEAREST;
				glTextureScaleValue2 = GL_NEAREST;
				break;
		}

		GLint glTextureBorderValue = 0;
		switch(aBorder) {
			case(TextureBorder::FILL_OUT_OF_RANGE):
				glTextureBorderValue = GL_CLAMP_TO_BORDER;
				break;
			case(TextureBorder::REPEAT):
			default:
				glTextureBorderValue = GL_REPEAT;
				break;
		}

		glGenTextures(1, &this->mHandle);
		glBindTexture(GL_TEXTURE_2D, this->mHandle);

		stbi_set_flip_vertically_on_load(aFlip);
		this->mpData = stbi_load_from_memory((stbi_uc*)aData, aPixelAmount, &this->mWidth, &this->mHeight, &this->mChannels, 4);
		if(!this->mpData) {
			std::cerr << "STBI failed to load image from memory!\n";
			return;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, this->mWidth, this->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->mpData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glTextureBorderValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glTextureBorderValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glTextureScaleValue2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glTextureScaleValue);

		glGenerateMipmap(GL_TEXTURE_2D);
	}

Texture::Texture(Texture&& aOther) noexcept
	: mPath(std::move(aOther.mPath)), mWidth(aOther.mWidth), mHeight(aOther.mHeight), mChannels(aOther.mChannels) {
		glDeleteTextures(1, &this->mHandle);
		this->mHandle = aOther.mHandle;
		//no free in constructor!
		this->mpData = aOther.mpData;

		aOther.mHandle = 0;
		aOther.mpData = nullptr;
	}
Texture& Texture::operator=(Texture&& aOther) noexcept {
		glDeleteTextures(1, &this->mHandle);
		this->mHandle = aOther.mHandle;
		stbi_image_free(this->mpData);
		this->mpData = aOther.mpData;

		this->mPath = std::move(aOther.mPath);
		this->mHandle = aOther.mHandle;
		this->mpData = aOther.mpData;
		this->mWidth = aOther.mWidth;
		this->mHeight = aOther.mHeight;
		this->mChannels = aOther.mChannels;

		aOther.mHandle = 0;
		aOther.mpData = nullptr;
		return *this;
	}

void Texture::setOutOfBoundsColor(const float aR, const float aG, const float aB, const float aA) noexcept {
	float clampColor[4] = { aR, aG, aB, aA };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);
}

void Texture::bind(const uint64_t aId) noexcept {
	if(this->mHandle == 0) return;
	glActiveTexture(GL_TEXTURE0 + aId);
	glBindTexture(GL_TEXTURE_2D, this->mHandle);
}
void Texture::unbind() noexcept {
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::getHandle() const noexcept {
	return this->mHandle;
}
std::string_view Texture::getPath() const noexcept {
	return this->mPath;
}
GLubyte* Texture::getData() const noexcept {
	return this->mpData;
}
int32_t Texture::getWidth() const noexcept {
	return this->mWidth;
}
int32_t Texture::getHeight() const noexcept {
	return this->mHeight;
}
int32_t Texture::getChannels() const noexcept {
	return this->mChannels;
}

uint64_t Texture::getAmountOfSlots() noexcept {
	GLint amount;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &amount);
	return amount;
}

Texture::~Texture() noexcept {
	glDeleteTextures(1, &this->mHandle);
	stbi_image_free(this->mpData);
}
