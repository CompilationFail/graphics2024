#include "loader/common.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

float debug_x, debug_y, debug_z;

namespace Control {

bool enable_mouse_control = 1;
const float stride = 1e-4;
float speed = 1;
float mouse_sensitivity = 1;
bool ui_window = 0;

glm::vec3 direction(0, 0, -1), upVec(0, 1, 0);
glm::vec3 camera(0.f, 0.f, 2.f);
int key_WASD[4];

double l_xpos, l_ypos;
double last_time;
int first_callback = true;
const double angle_stride = 0.002;
int mouse_state;

glm::vec3 dir4(int d) {
    if(d == 0) return direction;
    if(d == 1) return glm::cross(upVec, direction);
    if(d == 2) return -direction;
    if(d == 3) return glm::cross(direction, upVec);
    assert(0);
}
glm::vec3 rotate(glm::vec3 d, glm::vec3 axis, float angle) {
    return glm::rotate(glm::mat4(1.f), angle, axis) * glm::vec4(d, 1);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT) mouse_state = action;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_W) key_WASD[0] = action;
    if (key == GLFW_KEY_A) key_WASD[1] = action;
    if (key == GLFW_KEY_S) key_WASD[2] = action;
    if (key == GLFW_KEY_D) key_WASD[3] = action;
    if (key == GLFW_KEY_U && action == GLFW_PRESS) ui_window ^= 1;
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        enable_mouse_control ^= 1;
        if (enable_mouse_control) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}



static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if(first_callback) {
        first_callback = false;
        l_xpos = xpos, l_ypos = ypos;
    } else {
        if(enable_mouse_control) {
            double dx = (xpos - l_xpos) * angle_stride * mouse_sensitivity;
            double dy = (ypos - l_ypos) * angle_stride * mouse_sensitivity;
            // printf("%f %f\n", dx, dy);
            direction = rotate(direction, upVec, -dx);

            glm::vec3 x = glm::cross(upVec, -direction);
            // y = upVec
            // z = -direction
            direction = rotate(direction, x, -dy);
            upVec = rotate(upVec, x, -dy);
            printf("%.10f\n", glm::dot(direction, upVec));
        }
        l_xpos = xpos, l_ypos = ypos;
    }
}


void update(double now) {
    for (int i = 0; i < 4; ++i)
    {
        if (key_WASD[i])
            camera += dir4(i) * stride * float(now - last_time) * speed;
    }
}

void ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ui_window) {
        ImGui::Begin("Test", &ui_window);
        ImGui::Text("direction: (%.03f, %.03f, %.03f), [%.03f]", direction.x, direction.y, direction.z, glm::l2Norm(direction));
        ImGui::Text("upVec: (%.03f, %.03f, %.03f), [%.03f]", upVec.x, upVec.y, upVec.z, glm::l2Norm(upVec));
        glm::vec3 x = glm::cross(upVec, -direction);
        ImGui::Text("x: (%.03f, %.03f, %.03f)", x.x, x.y, x.z);
        ImGui::Text("camera: (%.03f, %.03f, %.03f)", camera.x, camera.y, camera.z);
        ImGui::Text("Mouse control: %s", enable_mouse_control ? "enabled" : "disabled");
        ImGui::SliderFloat("Speed", &speed, 0.f, 10.f);
        ImGui::SliderFloat("Mouse Sensitivity", &mouse_sensitivity, 0.f, 10.f);
        ImGui::Text("Debug parameters");
        ImGui::SliderFloat("x:", &debug_x, -100, 100);
        ImGui::SliderFloat("y:", &debug_y, -100, 100);
        ImGui::SliderFloat("z:", &debug_z, -100, 100);
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void test() {
    {
        double dx = 0.3;
        double dy = 0.3;
        // printf("%f %f\n", dx, dy);
        direction = rotate(direction, upVec, -dx);

        glm::vec3 x = glm::cross(upVec, -direction);
        // y = upVec
        // z = -direction
        direction = rotate(direction, x, -dy);
        upVec = rotate(upVec, x, -dy);
        printf("%.10f\n", glm::dot(direction, upVec));
        printf("%.3f %.3f %.3f\n", direction.x, direction.x, direction. z);
        printf("%.3f %.3f %.3f\n", upVec.x, upVec.x, upVec. z);
    }
    {
        double dx = -0.3;
        double dy = -0.3;
        // printf("%f %f\n", dx, dy);

        glm::vec3 x = glm::cross(upVec, -direction);
        // y = upVec
        // z = -direction
        direction = rotate(direction, x, -dy);
        upVec = rotate(upVec, x, -dy);
        
        direction = rotate(direction, upVec, -dx);

        printf("%.10f\n", glm::dot(direction, upVec));
        printf("%.3f %.3f %.3f\n", direction.x, direction.x, direction. z);
        printf("%.3f %.3f %.3f\n", upVec.x, upVec.x, upVec. z);
    }
}

}

void init_control(GLFWwindow *window) {
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, Control::key_callback);
    glfwSetMouseButtonCallback(window, Control::mouse_button_callback);
    glfwSetCursorPosCallback(window, Control::cursor_position_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    //  Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    Control::test();
}


void control_update_frame(double now) {
    Control::update(now);
}
void render_ui() {
    Control::ui();
}

glm::mat4 get_view_matrix() {
    /*
        TODO: Do we need a lock here?
    */
    return glm::lookAt(Control::camera, Control::camera + Control::direction, Control::upVec);
}
