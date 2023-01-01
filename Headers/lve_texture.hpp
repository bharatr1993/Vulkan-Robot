#pragma once

#include <cstdio>
#include <vulkan.h>
#include "Structs.h"
#include <stdexcept>
#include <string>
#include "lve_device.hpp"

namespace lve {
	struct bmfh
	{
		short bfType;
		int bfSize;
		short bfReserved1;
		short bfReserved2;
		int bfOffBits;
	};

	struct bmih
	{
		int biSize;
		int biWidth;
		int biHeight;
		short biPlanes;
		short biBitCount;
		int biCompression;
		int biSizeImage;
		int biXPelsPerMeter;
		int biYPelsPerMeter;
		int biClrUsed;
		int biClrImportant;
	};

	class LveTexture {
	public:
		LveTexture(LveDevice& device, std::vector<std::string> fPath);
		~LveTexture();

		std::vector<TextureFile> texFile{};
	private:
		LveDevice& lveDevice;
		std::vector<std::string> filePath;
		void initTextureSampler(size_t size);
		void initTextureBuffer();
		void fillfromBmpFile(std::vector<std::string> filePath);
		void createTransferCommandPool();
		std::vector<bmfh> FileHeader;
		std::vector<bmih> InfoHeader;
		VkCommandBuffer	TextureCommandBuffer;
		VkCommandPool TransferPool;
		unsigned char* texture;
        std::vector<VkImage> texDelImage;

		int ReadInt(FILE* fp)
		{
			unsigned char b3, b2, b1, b0;
			b0 = fgetc(fp);
			b1 = fgetc(fp);
			b2 = fgetc(fp);
			b3 = fgetc(fp);
			return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
		}

		short ReadShort(FILE* fp)
		{
			unsigned char b1, b0;
			b0 = fgetc(fp);
			b1 = fgetc(fp);
			return (b1 << 8) | b0;
		}
		
	};
}
