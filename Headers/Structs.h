#pragma once

#include <vector>
#include "glm.hpp"
#include "vulkan.h"

struct Vertex {
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec4 color;
	glm::vec4 uv;
};

struct particlebuf{
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 radius;
    glm::vec2 offset;
};

struct miscparticleinfo{
    glm::vec2 totalcount;
    glm::vec2 fElapsedTime;
    glm::vec2 mousePos;
};

struct Transform {
    glm::mat4 transform;
	glm::mat4 cameraMatrix;
	glm::mat4 projectionMatrix;
    glm::mat4 bckgTransform;
    glm::mat4 legLeftTransform;
    glm::mat4 legRightTransform;
    glm::mat4 cabinTransform;
    glm::vec2 scale;
};

struct colorTF {
    glm::vec4 color;
};

struct TextureFile
{
	uint32_t			width;
	uint32_t			height;
    unsigned char*      pixels;
	VkImage				texImage;
	VkImageView			texImageView;
	VkSampler			texSampler;
	VkDeviceMemory			vdm;
	VkDeviceMemory			nvdm;
};

enum class playerControls {
    moveUp,
    moveDown,
    None
};