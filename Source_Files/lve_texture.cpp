#include "lve_texture.hpp"

namespace lve {
	LveTexture::LveTexture(LveDevice& device, std::vector<std::string> fPath) : lveDevice{ device }, filePath{ fPath } {
		initTextureSampler(filePath.size());
		fillfromBmpFile(filePath);
		createTransferCommandPool();
		initTextureBuffer();
	}
	LveTexture::~LveTexture() {

		for (auto& texPtr : texFile) {
            vkFreeMemory(lveDevice.device(), texPtr.vdm, (VkAllocationCallbacks *) nullptr);
            vkFreeMemory(lveDevice.device(), texPtr.nvdm, (VkAllocationCallbacks *) nullptr);
            vkDestroyImageView(lveDevice.device(), texPtr.texImageView, (VkAllocationCallbacks *) nullptr);
            vkDestroyImage(lveDevice.device(), texPtr.texImage, (VkAllocationCallbacks *) nullptr);
            vkDestroySampler(lveDevice.device(), texPtr.texSampler, (VkAllocationCallbacks *) nullptr);
        }
        for (auto& texPtr : texDelImage) {
            vkDestroyImage(lveDevice.device(), texPtr, (VkAllocationCallbacks *) nullptr);
        }
        vkFreeCommandBuffers(lveDevice.device(), TransferPool, 1, &TextureCommandBuffer);
        vkDestroyCommandPool(lveDevice.device(),TransferPool,nullptr);
    }

	void LveTexture::createTransferCommandPool() {
		VkCommandPoolCreateInfo				vcpci;
		vcpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vcpci.pNext = nullptr;
		vcpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vcpci.queueFamilyIndex = lveDevice.findPhysicalQueueFamilies().TransferFamily;

		if(vkCreateCommandPool(lveDevice.device(), &vcpci, (VkAllocationCallbacks*) nullptr, &TransferPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool!");

		VkCommandBufferAllocateInfo	vcbai;
		vcbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vcbai.pNext = nullptr;
		vcbai.commandPool = TransferPool;
		vcbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vcbai.commandBufferCount = 1;

		if(vkAllocateCommandBuffers(lveDevice.device(), &vcbai, &TextureCommandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command pool!");
	}



	void LveTexture::initTextureSampler(size_t size) {
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = 0;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.minLod = 0.;
		samplerCreateInfo.maxLod = 0.;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		texFile.resize(size);

		for (auto& texPtr : texFile)
		{
			if (vkCreateSampler(lveDevice.device(), &samplerCreateInfo, nullptr, &texPtr.texSampler) != VK_SUCCESS)
				throw std::runtime_error("Failed to create Texture Sampler!");
		}

		FileHeader.resize(size);
		InfoHeader.resize(size);
	}

	void LveTexture::fillfromBmpFile(std::vector<std::string> filePath) {
		for (int i = 0; i < filePath.size(); i++)
		{
			const int birgb = { 0 };
			FILE* fp;

			fopen_s(&fp, filePath[i].c_str(), "rb");

			FileHeader[i].bfType = ReadShort(fp);

			if (FileHeader[i].bfType != 0x4d42)
			{
				assert("Error in File Type: Not BMP");
			}

			FileHeader[i].bfSize = ReadInt(fp);
			FileHeader[i].bfReserved1 = ReadShort(fp);
			FileHeader[i].bfReserved2 = ReadShort(fp);
			FileHeader[i].bfOffBits = ReadInt(fp);

			InfoHeader[i].biSize = ReadInt(fp);
			InfoHeader[i].biWidth = ReadInt(fp);
			InfoHeader[i].biHeight = ReadInt(fp);

			uint32_t texWidth = InfoHeader[i].biWidth;
			uint32_t texHeight = InfoHeader[i].biHeight;

			InfoHeader[i].biPlanes = ReadShort(fp);
			InfoHeader[i].biBitCount = ReadShort(fp);
			InfoHeader[i].biCompression = ReadInt(fp);
			InfoHeader[i].biSizeImage = ReadInt(fp);
			InfoHeader[i].biXPelsPerMeter = ReadInt(fp);
			InfoHeader[i].biYPelsPerMeter = ReadInt(fp);
			InfoHeader[i].biClrUsed = ReadInt(fp);
			InfoHeader[i].biClrImportant = ReadInt(fp);

			texture = new unsigned char[4 * texWidth * texHeight];

			int numExtra = 4 * (((3 * InfoHeader[i].biWidth) + 3) / 4) - 3 * InfoHeader[i].biWidth;

			if (InfoHeader[i].biCompression != birgb)
			{
				assert("Error in compression");
			}

			rewind(fp);
			fseek(fp, 14 + 40, SEEK_SET);
			
			if (InfoHeader[i].biBitCount == 24)
			{
				unsigned char* tp = texture;
				for (unsigned int t = 0; t < texHeight; t++)
				{
					for (unsigned int s = 0; s < texWidth; s++, tp += 4)
					{
						*(tp + 3) = 255;			// a
						*(tp + 2) = fgetc(fp);		// b
						*(tp + 1) = fgetc(fp);		// g
						*(tp + 0) = fgetc(fp);		// r
					}

					for (int e = 0; e < numExtra; e++)
					{
						fgetc(fp);
					}
				}
			}
			
			fclose(fp);
			texFile[i].pixels = texture;
			texFile[i].width = texWidth;
			texFile[i].height = texHeight;
		}

	}

	void LveTexture::initTextureBuffer() {

		for (int i = 0; i < filePath.size(); i++)
		{
			//Staging Image

			uint32_t texWidth = texFile[i].width;
			uint32_t texHeight = texFile[i].height;
			unsigned char* texture = texFile[i].pixels;
			VkDeviceSize textureSize = texWidth * texHeight * 4;

			VkImage  stagingImage;
			VkImage  textureImage;

			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.pNext = nullptr;
			imageInfo.flags = 0;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			//imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
			imageInfo.extent.width = texWidth;
			imageInfo.extent.height = texHeight;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = (const uint32_t*)nullptr;

			lveDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingImage,texFile[i].vdm);

			VkImageSubresource	vis;
			vis.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			vis.mipLevel = 0;
			vis.arrayLayer = 0;

			VkSubresourceLayout	vsl;
			vkGetImageSubresourceLayout(lveDevice.device(), stagingImage, &vis, &vsl);

			void* gpuMemory;
			vkMapMemory(lveDevice.device(), texFile[i].vdm, 0, VK_WHOLE_SIZE, 0, &gpuMemory);
			// 0 and 0 = offset and memory map flags

			if (vsl.rowPitch == 4 * texWidth)
			{
				memcpy(gpuMemory, (void*)texture, (size_t)textureSize);
			}
			else
			{
				unsigned char* gpuBytes = (unsigned char*)gpuMemory;
				for (unsigned int y = 0; y < texHeight; y++)
				{
					memcpy(&gpuBytes[y * vsl.rowPitch], &texture[4 * y * texWidth], (size_t)(4 * texWidth));
				}
			}

			vkUnmapMemory(lveDevice.device(), texFile[i].vdm);

			//Actual Image
			VkImageCreateInfo stagingImageInfo;
			stagingImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			stagingImageInfo.pNext = nullptr;
			stagingImageInfo.flags = 0;
			stagingImageInfo.imageType = VK_IMAGE_TYPE_2D;
			stagingImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			stagingImageInfo.extent.width = texWidth;
			stagingImageInfo.extent.height = texHeight;
			stagingImageInfo.extent.depth = 1;
			stagingImageInfo.mipLevels = 1;
			stagingImageInfo.arrayLayers = 1;
			stagingImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			stagingImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			stagingImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			stagingImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			stagingImageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			stagingImageInfo.queueFamilyIndexCount = 0;
			stagingImageInfo.pQueueFamilyIndices = (const uint32_t*)nullptr;

			lveDevice.createImageWithInfo(stagingImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, texFile[i].nvdm);

			VkCommandBufferBeginInfo vcbbi;
			vcbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			vcbbi.pNext = nullptr;
			vcbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vcbbi.pInheritanceInfo = (VkCommandBufferInheritanceInfo*)nullptr;

			if(vkBeginCommandBuffer(TextureCommandBuffer, &vcbbi)!=VK_SUCCESS)
				throw std::runtime_error("Error in Command Buffer: Texture");

			VkImageSubresourceRange	visr;
			visr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			visr.baseMipLevel = 0;
			visr.levelCount = 1;
			visr.baseArrayLayer = 0;
			visr.layerCount = 1;

			VkImageMemoryBarrier vimb;
			vimb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vimb.pNext = nullptr;
			vimb.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			vimb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			vimb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.image = stagingImage;
			vimb.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			vimb.dstAccessMask = 0;
			vimb.subresourceRange = visr;

			vkCmdPipelineBarrier(TextureCommandBuffer,
				VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
				(VkMemoryBarrier*)nullptr,0, (VkBufferMemoryBarrier*)nullptr,1, &vimb);

			visr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			visr.baseMipLevel = 0;
			visr.levelCount = 1;
			visr.baseArrayLayer = 0;
			visr.layerCount = 1;


			vimb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vimb.pNext = nullptr;
			vimb.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			vimb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			vimb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.image = textureImage;
			vimb.srcAccessMask = 0;
			vimb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vimb.subresourceRange = visr;

			vkCmdPipelineBarrier(TextureCommandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, (VkMemoryBarrier*)nullptr,
				0, (VkBufferMemoryBarrier*)nullptr,
				1, &vimb);

			// now do the final image transfer:

			VkImageSubresourceLayers visl;
			visl.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			visl.baseArrayLayer = 0;
			visl.mipLevel = 0;
			visl.layerCount = 1;

			VkOffset3D vo3;
			vo3.x = 0;
			vo3.y = 0;
			vo3.z = 0;

			VkExtent3D ve3;
			ve3.width = texWidth;
			ve3.height = texHeight;
			ve3.depth = 1;

			VkImageCopy vic;
			vic.srcSubresource = visl;
			vic.srcOffset = vo3;
			vic.dstSubresource = visl;
			vic.dstOffset = vo3;
			vic.extent = ve3;

			vkCmdCopyImage(TextureCommandBuffer,
				stagingImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &vic);


			visr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			visr.baseMipLevel = 0;
			visr.levelCount = 1;
			visr.baseArrayLayer = 0;
			visr.layerCount = 1;


			vimb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vimb.pNext = nullptr;
			vimb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			vimb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vimb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vimb.image = textureImage;
			//vimb.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			vimb.srcAccessMask = 0;
			//vimb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			vimb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			vimb.subresourceRange = visr;

			vkCmdPipelineBarrier(TextureCommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0,
				0, (VkMemoryBarrier*)nullptr,
				0, (VkBufferMemoryBarrier*)nullptr,
				1, &vimb);

			if(vkEndCommandBuffer(TextureCommandBuffer)!=VK_SUCCESS)
				throw std::runtime_error("Failed to submit command buffer: Texture");

			VkSubmitInfo vsi;
			vsi.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			vsi.pNext = nullptr;
			vsi.commandBufferCount = 1;
			vsi.pCommandBuffers = &TextureCommandBuffer;
			vsi.waitSemaphoreCount = 0;
			vsi.pWaitSemaphores = (VkSemaphore*)nullptr;
			vsi.signalSemaphoreCount = 0;
			vsi.pSignalSemaphores = (VkSemaphore*)nullptr;
			vsi.pWaitDstStageMask = (VkPipelineStageFlags*)nullptr;

			vkQueueSubmit(lveDevice.graphicsQueue_, 1, &vsi, VK_NULL_HANDLE);
			

			vkQueueWaitIdle(lveDevice.graphicsQueue_);
			


			// create an image view for the texture image:


			visr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			visr.baseMipLevel = 0;
			visr.levelCount = 1;
			visr.baseArrayLayer = 0;
			visr.layerCount = 1;

			VkImageViewCreateInfo			vivci;
			vivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			vivci.pNext = nullptr;
			vivci.flags = 0;
			vivci.image = textureImage;
			vivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vivci.format = VK_FORMAT_R8G8B8A8_SRGB;
			vivci.components.r = VK_COMPONENT_SWIZZLE_R;
			vivci.components.g = VK_COMPONENT_SWIZZLE_G;
			vivci.components.b = VK_COMPONENT_SWIZZLE_B;
			vivci.components.a = VK_COMPONENT_SWIZZLE_A;
			vivci.subresourceRange = visr;

			vkCreateImageView(lveDevice.device(), &vivci, (VkAllocationCallbacks *) nullptr, &texFile[i].texImageView);

			vkDestroyImage(lveDevice.device(), stagingImage, (VkAllocationCallbacks*) nullptr);

            texDelImage.push_back(textureImage);
		}

	}
}