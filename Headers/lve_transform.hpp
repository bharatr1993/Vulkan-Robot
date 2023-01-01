#pragma once
#include "glm/glm.hpp"

namespace lve
{

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};
        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };
}

