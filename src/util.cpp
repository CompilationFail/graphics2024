#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <optional>
#include "loader/common.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}
 
GLFWwindow *window_init(int width = 800, int height = 600, const char *title = "no title", bool allow_transparent = false, int vsync = 0) {
    printf("initiating glfw window ...");
    glfwSetErrorCallback(error_callback);
    if(glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("fail to init glfw");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if(allow_transparent) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#ifdef __APPLE__ // for macos
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("failed to create window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync);
    printf("   done\n");
    return window;
}

void glew_init() {
    printf("initiating glew...");
    GLenum e = glewInit();
    if (e != GLEW_OK)
    {
        std::cerr << glewGetErrorString(e) << std::endl;
        throw std::runtime_error("failed to init glew");
    }
    printf("   done\n");
}

void imgui_init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
}

glm::vec3 dir4(float pitch, float yaw, int dir) {
    // dir 0 ~ 3, WASD 
    auto y = yaw;
    auto vec = glm::rotate(
        glm::rotate(glm::mat4(1.f), pitch, glm::vec3(1,0,0)), -yaw + dir * PI / 2, glm::vec3(0,1,0)) * glm::vec4(0,0,-1,1);
    return vec.xyz();
}
