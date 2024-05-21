#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "init.cpp"
#include "control.cpp"
#include <optional>
#include "util/mesh.hpp"
#include "util/bound.hpp"
// #include "util/particle.hpp"
#include "util/scene.hpp"

static const int width = 1000, height = 1000;

/*

TODO:
+ map_Bump
+ shadow map
+ Normal mapping
  To implement a correct mapping that applies no matter how the object is oriented
  I need to put a tangent and bitangent vector for each vertex
  which is not supported now.

  In this program, it is specialize for the ground is simple and can be manually transformed.


*/

class Application {
    GLFWwindow *window;
    std::unique_ptr <Scene> scene;
    // std::unique_ptr <Mesh> mesh, ground;
    // std::unique_ptr <ParticleSystem> ps;
    glm::mat4 model;
public:
    Application(): scene(nullptr) { }
    ~Application() {
        scene = nullptr;
        glfwDestroyWindow(window);
        printf("Window destroyed\n");
        glfwTerminate();
        printf("Terminated..");
    }
    void init() {
        camera.position = glm::vec3(0.f,1.5f, 3.f);
        camera.pitch = glm::radians(-35.f);
        camera.yaw = -PI/2;
        light.pitch = -PI/3;
        light.yaw = PI * 0.6;
        light.position = glm::vec3(1.65, 3, -2.1);
        light_intense = glm::vec3(10, 10, 10);
        window = window_init(width, height, "Ground and indoor planet");
        glew_init();
        model = glm::mat4(1.f);
        init_control(window);
        Control::camera = &camera;
        scene = std::make_unique<Scene>();
        printf("Application initiated.\n");
    }
    void load(std::string name, const Path &path) {
        try{
            scene -> load_mesh(name, path);
        } catch(const char *e) {
            printf("%s\n", e);
        }
        /*
        printf("Mesh loaded from %s.\n", path.u8string().c_str());
        //ground = std::make_unique <Mesh> (glm::vec3(0,-1, 2000), glm::vec3(-2000,-1,-2000),glm::vec3(2000, -1, -2000), glm::vec3(0,1,0), glm::vec3(1,1,1));
        // ground -> init_draw();
        try{
            ps = std::make_unique <ParticleSystem> (2e-3, glm::vec3(0, 0, 0), 1.8, 2);
            ps -> generate(1e5);
        } catch(const char *e) {
            printf("%s\n", e);
        }
        printf("Particle system generated.\n");
        */
    }
    glm::mat4 projection() {
        return glm::perspective(glm::radians(45.f), 1.f * width / height, .1f, 100.f);
    }
    /*glm::mat4 view() {
        return glm::rotate(glm::mat4(1.f), -pitch, glm::vec3(1, 0, 0)) *
               glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0, 1, 0)) *
               glm::translate(glm::mat4(1.f), -camera);
    }*/
    void main_loop() {
        puts("init draw");
        scene->init_draw();
        scene->model()["plant"] = glm::translate(glm::mat4(1.f), glm::vec3(0, -1, 0));
        scene->activate_shadow();
        puts("Enter main loop");
        auto last = glfwGetTime();
        int frame_count = 0;
        while (!glfwWindowShouldClose(window)) {
            auto now = glfwGetTime();
            control_update_frame(now);
            // printf("Frame: %d\n", ++frame_count);

            // printf("%f %f\n", pitch, yaw);
            auto vp = projection() * camera.view();
            /* {
                glm::vec4 p = mvp * glm::vec4(0.5,0.5,-1,1);
                auto q = p.xyz() / p.w;
                printf("%f %f %f\n", q.x,q.y,q.z);
            }
            {
                auto p = mvp * glm::vec4(0.5,0.5,-2,1);
                auto q = p.xyz() / p.w;
                printf("%f %f %f\n", q.x,q.y,q.z);
            }
            break; */
            // mesh->draw(vp * glm::rotate(glm::mat4(1.f), float(now / 50 * rot_speed), glm::vec3(0, 1, 0)), Control::camera, light);
            /*ground->draw(vp, Control::camera, light);
            mesh->draw(vp, Control::camera, light);*/
            scene->update_light(light, light_intense);
            scene->render(window, vp, camera.position);

            // ps->set_particle_size(2e-3 * particle_size);
            // ps->draw(particle_number, vp, Control::camera, now / 100 * rot_speed, light);
            render_ui();
            // break;
        }
    }
};

int main(int argc, char **argv) {
    /*if(argc <= 1) {
        printf("Usage: program [Path to .obj]\n");
        return 0;
    }*/
    Application app;
    app.init();
    std::string name = "";
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-name") == 0) {
            name = argv[i + 1];
            i++;
            continue;
        }
        app.load(name, argv[i]);
    }
    app.main_loop();
}
