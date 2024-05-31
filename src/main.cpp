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
    std::vector <std::pair <int,int>> beatmap;
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
        camera.position = glm::vec3(-0.6f,5.f, 8.f);
        camera.pitch = glm::radians(-30.f);
        camera.yaw = -PI/2;
        /*lights.push_back({
            Camera{-0.581, PI, glm::vec3(8.11,6.91,-5.97)},
            glm::vec3(100),
            CONE_LIGHT
        });*/
        /*lights.push_back({
            Camera{-0.584, 0.731, glm::vec3(-2.81,1.744,-2.521)},
            glm::vec3(30),
            CONE_LIGHT,
        });*/
        lights.push_back({
            Camera{-0.584, 0.731, glm::vec3(-2.81,1.744,-2.521)},
            glm::vec3(1),
            DIRECTIONAL_LIGHT,
        });
        lights.push_back({
            Camera{-0.584, float(0.731 + PI / 2), glm::vec3(2.81,1.744,-2.521)},
            glm::vec3(20),
            POINT_LIGHT,
        });
        window = window_init(width, height, "wheel chair player");
        glew_init();
        model = glm::mat4(1.f);
        init_control(window);
        Control::camera = &camera;
        scene = std::make_unique<Scene>();
        printf("Application initiated.\n");
    }
    void load_beatmap(const char* path) {
        FILE *f = fopen(path, "r");
        int t, p;
        while(~fscanf(f, "%d%d", &t,&p)){ 
            beatmap.emplace_back(t, p);
        }
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
        std::sort(beatmap.begin(), beatmap.end());
        puts("init draw");
        scene->init_draw();
        scene->model()["robot"] = {glm::translate(glm::mat4(1.f), glm::vec3(0.7f, -1.f, 0.7f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.01))};
        scene->activate_shadow();
        puts("Enter main loop");
        auto last = glfwGetTime();
        int frame_count = 0;
        while (!glfwWindowShouldClose(window)) {
            scene->model()["plant"] = {glm::translate(glm::mat4(1.f), glm::vec3(debug_x, debug_y, debug_z))};
            for(auto &[x,y]: scene -> meshes) if(x == "ground") {
                for(auto m: y->mtl->materials) {
                    m->roughness = roughness;
                    m->metallic= metallic;
                }
            }
            auto now = glfwGetTime();
            control_update_frame(now);
            /*scene->model()["wheel"] = {};
            for(auto [t, x]: beatmap) {
                int _t = t - now * 1000;
                if(_t >= -300 && _t <= 1e4) {
                    scene->model()["wheel"].push_back(glm::translate(glm::mat4(1.f), glm::vec3((x - 2) * 2.f, -1.f, -_t / 30.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.04)));
                }
            }*/
            // printf("Frame: %d\n", ++frame_count);

            // printf("%f %f\n", pitch, yaw);
            auto vp = projection() * Control::camera->view();
            /*for(auto i: {glm::vec3(0,0,0), glm::vec3(0,0,1), glm::vec3(0,1,1), glm::vec3(1,1,1)}) {
                auto p = vp * glm::vec4(i, 1);
                auto q = glm::vec3(p.xyz) / p.w;
                printf("%f %f %f\n", q.x,q.y,q.z);
            }
            return;*/
            ;
           // lights[0].vp();
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
            scene->update_light(lights);
            // light, light_intense);
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
        printf("loading %s %s\n", name.c_str(), argv[i]);
        app.load(name, argv[i]);
    }
    // app.load_beatmap("2.beatmap");
    app.main_loop();
}
