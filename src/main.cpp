#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "util.cpp"
#include "control.cpp"
#include <optional>
#include "loader/loader.hpp"
#include "loader/bound.hpp"
#include "loader/particle.hpp"
#include "loader/scene.hpp"

static const int width = 1200, height = 800;

/*

TODO:
+ map_Bump
+ shadow map

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
        light.position = glm::vec3(1.65, 1.24, -2.1);
        light.intense = glm::vec3(10, 10, 10);
        window = window_init(width, height, "Venus??");
        glew_init();
        model = glm::mat4(1.f);
        init_control(window);
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
        printf("Particle system generated.\n");*/
    }
    glm::mat4 projection() {
        return glm::perspective(45.f, 1.f * width / height, .1f, 100.f);
    }
    /*glm::mat4 view() {
        return glm::rotate(glm::mat4(1.f), -pitch, glm::vec3(1, 0, 0)) *
               glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0, 1, 0)) *
               glm::translate(glm::mat4(1.f), -camera);
    }*/
    void main_loop() {
        puts("init draw");
        scene -> init_draw();
        scene -> model()["plant"] = glm::translate(glm::mat4(1.f), glm::vec3(0, -1, 0));
        puts("Enter main loop");
        auto last = glfwGetTime();
        int frame_count = 0;
        while (!glfwWindowShouldClose(window)) {
            auto now = glfwGetTime();
            control_update_frame(now);
            // printf("Frame: %d\n", ++frame_count);

            glfwPollEvents();
            glfwSwapBuffers(window);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClearColor(0., 0., 0., 1.);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            // printf("%f %f\n", pitch, yaw);
            auto vp = projection() * get_view_matrix();
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
            scene->render(vp, Control::camera, light);

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
