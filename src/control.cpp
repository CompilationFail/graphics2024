#include "util/camera.hpp"
#include "util/common.hpp"
#include "util/shader.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <string>

float roughness = 0.3f;
float metallic = 0.6f;

float debug_x, debug_y = -1.f, debug_z;
/*float particle_size = 1.5;
float rot_speed = 1;
int particle_number = 4e4; */

Camera camera;
std::vector <LightInfo> lights;

Camera *cameras[10]; 
int camera_cnt, current_camera;

namespace Control {


bool enable_mouse_control = 1;
const float stride = 1;
float speed = 1;
float mouse_sensitivity = 1;
bool ui_window = 0;

Camera *camera;
int key_WASD[4];

double l_xpos, l_ypos;
double last_time;
int first_callback = true;
const double angle_stride = 0.002;
int mouse_state;
float fps = 0;



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
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        camera = cameras[current_camera = (current_camera + 1) % camera_cnt];
    }
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

            if(camera) {
                camera -> pitch -= (float)dy;
                camera -> yaw += (float)dx;
            }
        }
        l_xpos = xpos, l_ypos = ypos;
    }
}


void update(double now) {
    if(!camera) return;
    for (int i = 0; i < 4; ++i)
    {
        if (key_WASD[i])
            camera -> position += camera -> dir4(i) * stride * float(now - last_time) * speed;
    }
    last_time = now;
}


void ui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ui_window) {
        ImGui::Begin("SSDO_test", &ui_window);
        ImGui::Text("FPS: %f", fps);
        ImGui::Text("pitch: %.03f, yaw: %.03f", camera->pitch, camera->yaw);
        ImGui::Text("camera position:(%.03f,%.03f,%.03f)", camera->position.x, camera->position.y, camera->position.z);
        /*auto d = dir(), r = right(), u = up();
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
        ImGui::Text("Lights Info");
        for(int i = 0; i < (int) lights.size(); ++i) {
            auto &l = lights[i];
            /*ImGui::SliderFloat((std::to_string(i) + ": pos.x:").c_str(), &l.camera.position.x, -100, 100);
            ImGui::SliderFloat((std::to_string(i) + ": pos.y:").c_str(), &l.camera.position.y, -100, 100);
            ImGui::SliderFloat((std::to_string(i) + ": pos.z:").c_str(), &l.camera.position.z, -100, 100);*/
            ImGui::SliderFloat((std::to_string(i) + ": intense.x:").c_str(), &l.intense.x, -10, 10);
            ImGui::SliderFloat((std::to_string(i) + ": intense.y:").c_str(), &l.intense.y, -10, 10);
            ImGui::SliderFloat((std::to_string(i) + ": intense.z:").c_str(), &l.intense.z, -10, 10);
        } 
        ImGui::Text("Ground Material");
        ImGui::SliderFloat("roughness:", &roughness, 0, 1);
        ImGui::SliderFloat("metallic:", &metallic, 0, 1);
        ImGui::Text("Debug parameters");
        ImGui::SliderFloat("x:", &debug_x, -100, 100);
        ImGui::SliderFloat("y:", &debug_y, -100, 100);
        ImGui::SliderFloat("z:", &debug_z, -100, 100);
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
}

void init_control(GLFWwindow *window) {
    camera_cnt = (int)lights.size() + 1;
    cameras[0] = Control::camera = &camera;
    for(int i = 1; i < camera_cnt; ++i) cameras[i] = &lights[i-1].camera;
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
        Control::fps = float(fps_counter / (now - fps_last));
        fps_last = (float)now;
        fps_counter = 0;
    }
    Control::update(now);
}
void render_ui() {
    Control::ui();
}
