#include "loader/common.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "loader/shader.hpp"

float debug_x, debug_y, debug_z;
/*float particle_size = 1.5;
float rot_speed = 1;
int particle_number = 4e4; */
LightSource light;

namespace Control {

const glm::vec3 worldUp(0,1,0);

bool enable_mouse_control = 1;
const float stride = 1;
float speed = 1;
float mouse_sensitivity = 1;
bool ui_window = 0;

float pitch, yaw = -PI/2;
glm::vec3 camera(0.f, 0.5f, 3.f);
int key_WASD[4];

double l_xpos, l_ypos;
double last_time;
int first_callback = true;
const double angle_stride = 0.002;
int mouse_state;
float fps = 0;

glm::vec3 dir() {
    glm::vec3 direction;
    direction.x = cos(yaw) * cos(pitch);
    direction.y = sin(pitch);
    direction.z = sin(yaw) * cos(pitch);
    direction = glm::normalize(direction);
    return direction;
}
glm::vec3 right() {
    return glm::normalize(glm::cross(dir(), worldUp));
}
glm::vec3 up() {
    return glm::cross(right(), dir());
}

glm::vec3 dir4(int d) {
    if(d == 0) return dir();
    if(d == 2) return -dir();
    if(d == 1) return -right();
    if(d == 3) return right();
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

            pitch -= dy;
            yaw += dx;
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
    last_time = now;
}


void ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ui_window) {
        ImGui::Begin("Venus", &ui_window);
        ImGui::Text("FPS: %f", fps);
        /*ImGui::Text("pitch: %.03f, yaw: %.03f", pitch, yaw);
        auto d = dir(), r = right(), u = up();
        ImGui::Text("direction: (%f, %f, %f)", d.x, d.y, d.z);
        ImGui::Text("right: (%f, %f, %f)", r.x, r.y, r.z);
        ImGui::Text("up: (%f, %f, %f)", u.x, u.y, u.z);
        ImGui::Text("camera: (%.03f, %.03f, %.03f)", camera.x, camera.y, camera.z);*/
        ImGui::Text("Mouse control: %s", enable_mouse_control ? "enabled" : "disabled");
        ImGui::SliderFloat("Speed", &speed, 0.f, 10.f);
        ImGui::SliderFloat("Mouse Sensitivity", &mouse_sensitivity, 0.f, 10.f);
        /*ImGui::Text("Particle System");
        ImGui::SliderFloat("Particle size:", &particle_size, 0.1, 10);
        ImGui::SliderFloat("Rotate speed", &rot_speed, 0.1, 5);
        ImGui::SliderInt("Particle number", &particle_number, 1000, 1e5);*/
        ImGui::Text("Light");
        ImGui::SliderFloat("l_pos.x:", &light.position.x, -100, 100);
        ImGui::SliderFloat("l_pos.y:", &light.position.y, -100, 100);
        ImGui::SliderFloat("l_pos.z:", &light.position.z, -100, 100);
        ImGui::SliderFloat("intense.x:", &light.intense.x, 0, 200);
        ImGui::SliderFloat("intense.y:", &light.intense.y, 0, 200);
        ImGui::SliderFloat("intense.z:", &light.intense.z, 0, 200);
        /*ImGui::Text("Debug parameters");
        ImGui::SliderFloat("x:", &debug_x, -100, 100);
        ImGui::SliderFloat("y:", &debug_y, -100, 100);
        ImGui::SliderFloat("z:", &debug_z, -100, 100);*/
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
}

void init_control(GLFWwindow *window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
}


void control_update_frame(double now) {
    static int fps_counter = 0;
    static float fps_last = 0;
    fps_counter++;
    if(now - fps_last > 0.1) {
        Control::fps = fps_counter / (now - fps_last);
        fps_last = now;
        fps_counter = 0;
    }
    Control::update(now);
}
void render_ui() {
    Control::ui();
}

glm::mat4 get_view_matrix() {
    /*
        TODO: Do we need a lock here?
    */
    return glm::lookAt(Control::camera, Control::camera + Control::dir(), Control::up());
}