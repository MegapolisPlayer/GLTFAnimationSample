#ifndef EUROTRAM_TEXTURE
#define EUROTRAM_TEXTURE
#include "Shader.hpp"

enum class TextureBorder : uint8_t {
	REPEAT = 0,
	FILL_OUT_OF_RANGE
};

enum class TextureScale : uint8_t {
	LINEAR = 0,
	NEAREST_NEIGHBOR
};

class Texture {
public:
	//channels = bits per pixel
	Texture(const std::string_view aFilename, const bool aFlip = true, TextureScale aScaling = TextureScale::LINEAR, TextureBorder aBorder = TextureBorder::REPEAT) noexcept;
	Texture(const uint64_t aWidth, const uint64_t aHeight, const uint64_t aInternalFormat, const uint64_t aFormat, const uint64_t aDataType = GL_FLOAT, const bool aNoMipmaps = false, TextureScale aScaling = TextureScale::LINEAR, TextureBorder aBorder = TextureBorder::REPEAT) noexcept;

	//embedded textures are a pain
	Texture(void* aData, const size_t aPixelAmount, const bool aFlip = true, TextureScale aScaling = TextureScale::LINEAR, TextureBorder aBorder = TextureBorder::REPEAT) noexcept;

	Texture(Texture&& aOther) noexcept;
	Texture& operator=(Texture&& aOther) noexcept;
	Texture(Texture& aOther) noexcept = delete;
	Texture& operator=(Texture& aOther) noexcept = delete;

	void setOutOfBoundsColor(const float aR, const float aG, const float aB, const float aA = 1.0f) noexcept;

	void bind(const uint64_t aId) noexcept;
	void unbind() noexcept;

	GLuint getHandle() const noexcept;
	std::string_view getPath() const noexcept;

	GLubyte* getData() const noexcept;
	int32_t getWidth() const noexcept;
	int32_t getHeight() const noexcept;
	int32_t getChannels() const noexcept;

	static uint64_t getAmountOfSlots() noexcept;

	~Texture() noexcept;
private:
	std::string_view mPath; //so no duplicates
	GLuint mHandle;
	GLubyte* mpData;
	int32_t mWidth, mHeight, mChannels;
};

#endif
