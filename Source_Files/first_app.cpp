#include "first_app.hpp"

// std
#include <array>
#include <stdexcept>

namespace lve {

    int moveLeft = GLFW_KEY_LEFT;
    int moveRight = GLFW_KEY_RIGHT;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;

    playerControls currentMove = playerControls::None;

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

        if (key == moveForward && action == GLFW_PRESS)
            currentMove =playerControls::moveUp;
        else if(key == moveForward && action == GLFW_RELEASE && currentMove==playerControls::moveUp)
            currentMove =playerControls::None;
        else if(key == moveBackward && action == GLFW_PRESS)
            currentMove = playerControls::moveDown;
        else if(key == moveBackward && action == GLFW_RELEASE && currentMove==playerControls::moveDown)
            currentMove = playerControls::None;

    }


FirstApp::FirstApp() {


    lveRenderer = std::make_unique<LveRenderer>(lveDevice, lveWindow, lveSwapChain, vertPath, fragPath, compPath,bmpPath, vertices,objPath);
    glfwSetKeyCallback(lveWindow.window,keyCallback);
    lveRenderer->transform.projectionMatrix = glm::perspective( glm::radians(60.),(double) lveWindow.getAspectRatio(), 0.1, 500. );
    lveRenderer->transform.projectionMatrix *= glm::lookAt(  glm::vec3(0.0f,0.0f,3.0f),  glm::vec3(0.0f,0.0f,0.0f),  glm::vec3(0.0f,1.0f,0.0f) );
    lveRenderer->lveComputePipeline->prepareMiscBuffer(100,fElapsedTime);

    //lveRenderer->createComputeCommandBuffer();//Just for Testing!!!Remove
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  while (!lveWindow.shouldClose()) {
       std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
       renderScene();
       lveRenderer->lveComputePipeline->updateComputeDescriptor();
       lveRenderer->createCommandBuffers(pipelineNames);
       lveRenderer->createComputeCommandBuffer();
       lveRenderer->drawFrame();
       glfwPollEvents();
       std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
       std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
       fElapsedTime=time_span.count();
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::renderScene() {

        double xpos, ypos;
        glfwGetCursorPos(lveWindow.window, &xpos, &ypos);

        xpos-=400;
        ypos-=300;

        xpos=xpos/400.0f;
        ypos=ypos/300.0f;

        xpos = clamp(xpos,1,-1);
        ypos = clamp(ypos,1,-1);

        lveRenderer->lveComputePipeline->miscInfo.mousePos=glm::vec2(xpos,ypos);


        lveRenderer->lveComputePipeline->commitMiscBuffer(fElapsedTime);

        static float timer=0, angle=0, rposx=0,rposz=0;

        if(xpos>0.75)
            angle+=fElapsedTime;
        else if(xpos<-0.75f)
            angle-=fElapsedTime;

        bMat.translation = {0.0f,-0.75f+1.25f,-2.5f};
        bMat.rotation = {0.0f,glm::pi<float>(),glm::pi<float>()};
        bMat.scale={0.45f,0.45f,0.45f};

        tfMat.translation = {0.0f,1.0f+1.25f,-2.5f};
        tfMat.rotation = {0.0f,glm::pi<float>(),glm::pi<float>()};
        tfMat.scale={0.5f,0.5f,0.5f};

        cameraMat.translation = {0.0f,0.0f,0.0f};
        cameraMat.rotation = {0.05f,glm::pi<float>()+angle, 0.0f};
        cameraMat.scale={1.0f,1.0f,1.0f};

        cabinMat.translation = {rposx,1.0f,rposz};
        cabinMat.rotation = {glm::pi<float>()/2.0f, 0.0f, 0.0f};
        cabinMat.scale={5.0f,5.0f,5.0f};
/*
        glm::mat4 rotx = glm::rotate(glm::mat4(1.0f),glm::pi<float>()/2.0f,glm::vec3(1.0f,0.0f,0.0f));

        glm::mat4 rotz = glm::rotate(glm::mat4(1.0f),glm::pi<float>()/2.0f,glm::vec3(1.0f,0.0f,0.0f));

        glm::mat4 scale = glm::scale(glm::mat4(1.0f),glm::vec3(5.0f));

        glm::mat4 translate = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,1.0f,0.0f));
*/
        static float angleft=-glm::pi<float>()/6.0f, angright=glm::pi<float>()/6.0f;

        static bool fwdleft=true;



        if(fwdleft && angleft<(glm::pi<float>()/6.0f))
        {
            legLeftMat.rotation = {angleft,glm::pi<float>(),glm::pi<float>()};
            legRightMat.rotation = {angright,glm::pi<float>(),glm::pi<float>()};

            if(currentMove==playerControls::moveUp) {
                angleft += 1.5f * fElapsedTime;
                angright -= 1.5f * fElapsedTime;
            }
        }
        else if(!fwdleft && angleft>-(glm::pi<float>()/6.0f))
        {
            legLeftMat.rotation = {angleft,glm::pi<float>(),glm::pi<float>()};
            legRightMat.rotation = {angright,glm::pi<float>(),glm::pi<float>()};

            if(currentMove==playerControls::moveUp) {
                angleft -= 1.5f * fElapsedTime;
                angright += 1.5f * fElapsedTime;
            }
        }

        if(currentMove==playerControls::moveUp) {
            rposz-=3.0*cos(angle)*fElapsedTime;
            rposx+=3.0*sin(angle)*fElapsedTime;
        }

        if(angleft>(glm::pi<float>()/6.0f))
            fwdleft=false;
        else if(angleft<-(glm::pi<float>()/6.0f))
            fwdleft=true;

        legLeftMat.translation = {0.0f,-0.1f+1.25f,-2.5f};

        legLeftMat.scale={0.5f,0.5f,0.5f};

        legRightMat.translation = {0.0f,-0.1f+1.25f,-2.5f};

        legRightMat.scale={0.5f,0.5f,0.5f};

        lveRenderer->transform.transform = tfMat.mat4();
        lveRenderer->transform.bckgTransform=bMat.mat4();
        lveRenderer->transform.legLeftTransform = legLeftMat.mat4();
        lveRenderer->transform.legRightTransform = legRightMat.mat4();
        lveRenderer->transform.scale = glm::vec2(tfMat.scale.x, tfMat.scale.y);
        lveRenderer->transform.cameraMatrix = cameraMat.mat4();
        lveRenderer->transform.cabinTransform = cabinMat.mat4();

        for (auto& bufferPtr : lveRenderer->lveBufferPtrVec)
        {
            bufferPtr->writeToBuffer((void*)&lveRenderer->transform, sizeof(lveRenderer->transform));

        }

        lveRenderer->descriptorSet->updateTransformDescriptorSet();



}

}  // namespace lve