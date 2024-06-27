#include "scene.hpp"
#include <stack>

Scene::Scene()
    : shadow(0), depth_buffer(0), denoiser(nullptr), mixer(nullptr) {}
Scene::~Scene() {
    depth_shader = nullptr;
    denoiser = nullptr;
    mixer = nullptr;
}
std::map<std::string, std::vector<glm::mat4>> &Scene::model() {
    return _model;
}
void Scene::init_draw(int _width, int _height) {
    for(auto &[name, mesh]: meshes) mesh -> init_draw();
    width = _width, height = _height;

    static const float vertices[] = {
        -1.f, -1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f,  1.f, 0.f,
        1.f, -1.f, 0.f,
        1.f, 1.f, 0.f,
        -1.f,  1.f, 0.f
    };  
    glGenBuffers(1, &rec_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rec_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &rec_vao);  
    glBindVertexArray(rec_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  


    glGenFramebuffers(1, &buffer);
    glGenFramebuffers(1, &buffer2);
    glGenFramebuffers(1, &buffer3);
    glGenFramebuffers(1, &buffer4);
    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width,
                 height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glGenTextures(1, &depth2);
    glBindTexture(GL_TEXTURE_2D, depth2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width,
                 height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &normal);
    glBindTexture(GL_TEXTURE_2D, normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glGenTextures(1, &ssdo);
    glBindTexture(GL_TEXTURE_2D, ssdo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glGenTextures(1, &out_a);
    glBindTexture(GL_TEXTURE_2D, out_a);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glGenTextures(1, &out_b);
    glBindTexture(GL_TEXTURE_2D, out_b);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16_SNORM, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);
    GLuint buffers[2] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
    glReadBuffer(GL_NONE);
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
    
    glBindFramebuffer(GL_FRAMEBUFFER, buffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssdo, 0);
    glDrawBuffers(1, buffers);
    glReadBuffer(GL_NONE);
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
        
    glBindFramebuffer(GL_FRAMEBUFFER, buffer3);
    CheckGLError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_a, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_NONE);
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
    
    glBindFramebuffer(GL_FRAMEBUFFER, buffer4);
    CheckGLError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_b, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_NONE);
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
        

    try {
        denoiser = std::make_unique <Denoiser>();
        mixer = std::make_unique <Mixer>();
    } catch (std::string msg) {
        warn(2, "[ERROR] Fail to load shader program: %s", msg.c_str());
        exit(1);
    }
    first = 1;

}

void Scene::activate_shadow() {
    shadow = 1;
    depth_shader = std::make_unique <DepthShader> ();
    
    glGenFramebuffers(1, &depth_buffer);  
}
void Scene::update_light(std::vector <LightInfo> info) {
    light_info = info;
}
void Scene::render(GLFWwindow *window, glm::mat4 vp, glm::vec3 camera, float time, float denoise_alpha, float movement) {
    glfwGetFramebufferSize(window, &width, &height);
    CheckGLError();
    glfwPollEvents();
    CheckGLError();
 
    if (shadow) {
        render_depth_buffer();
    }
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
        CheckGLError();

        // glfwGetFramebufferSize(window, &width, &height);
        // CheckGLError();
        glViewport(0, 0, width, height);
        CheckGLError();
        glClearColor(0., 0., 0., 1.);
        CheckGLError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError();
        glEnable(GL_DEPTH_TEST);
        CheckGLError();
        glDepthFunc(GL_LESS);
        CheckGLError();
        for(auto &[name, mesh]: meshes) {
            if(!_model.count(name)) {
                mesh->draw(glm::mat4(1.f), vp, camera, light_info, depth_map, 0);
            } else {
                for(auto model: _model[name]) {
                    mesh->draw(model, vp, camera, light_info, depth_map, 0);
                }
            }
            // mesh->draw(_model.count(name) ? _model[name] : glm::mat4(1.f), vp, camera, light_info, depth_map);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };
    {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer2);
        CheckGLError();

        // glfwGetFramebufferSize(window, &width, &height);
        // CheckGLError();
        glViewport(0, 0, width, height);
        CheckGLError();
        glClearColor(0., 0., 0., 1.);
        CheckGLError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError();
        glEnable(GL_DEPTH_TEST);
        CheckGLError();
        glDepthFunc(GL_LESS);
        CheckGLError();
        for(auto &[name, mesh]: meshes) {
            if(!_model.count(name)) {
                mesh->draw(glm::mat4(1.f), vp, camera, light_info, depth_map, 1, depth, normal, color, time);
            } else {
                for(auto model: _model[name]) {
                    mesh->draw(model, vp, camera, light_info, depth_map, 1, depth, normal, color, time);
                }
            }
            // mesh->draw(_model.count(name) ? _model[name] : glm::mat4(1.f), vp, camera, light_info, depth_map);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    };
    {

        glBindFramebuffer(GL_FRAMEBUFFER, buffer3);
        CheckGLError();
        glDisable(GL_DEPTH_TEST);
    
        glViewport(0, 0, width, height);
        CheckGLError();
        glClearColor(0.0, 0.0, 0.0, 1.);
        CheckGLError();
        glClear(GL_COLOR_BUFFER_BIT);
        CheckGLError();

        denoiser -> use();
        CheckGLError();
        float alpha = std::min(1.f, denoise_alpha * (1 + movement * 1000));
        denoiser -> set(ssdo, first ? 0 : out_b, alpha);
        CheckGLError();
        first = 0;

        glBindBuffer(GL_ARRAY_BUFFER, rec_vbo);
        glBindVertexArray(rec_vao);
        CheckGLError();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        CheckGLError();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CheckGLError();
    };
    {
        glfwSwapBuffers(window);
        CheckGLError();
        glDisable(GL_DEPTH_TEST);

        glBindBuffer(GL_ARRAY_BUFFER, rec_vbo);
        CheckGLError();
        glBindVertexArray(rec_vao);
        CheckGLError();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  
        
        glViewport(0, 0, width, height);
        CheckGLError();
        glClearColor(0.0, 0.0, 0.0, 1.);
        CheckGLError();
        glClear(GL_COLOR_BUFFER_BIT);

        mixer -> use();
        // mixer -> set(color, ssdo);
        float alpha = 1 / (1 + movement * 1000);
        mixer -> set(color, out_a, alpha);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CheckGLError();
    };
    
    std::swap(out_a, out_b);
    std::swap(buffer3, buffer4);
}

void Scene::render_depth_buffer() {
    while(light_info.size() > depth_map.size()) {
        depth_map.push_back(0);
        auto &i = depth_map.back();
        glGenTextures(1, &i);  
        glBindTexture(GL_TEXTURE_2D, i);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                    depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
    }

    for(int i = 0; i < (int)light_info.size(); ++i) {
        auto &light = light_info[i];
        glViewport(0, 0, depth_map_width, depth_map_height);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        CheckGLError();

        glClear(GL_DEPTH_BUFFER_BIT);
        CheckGLError();
        glEnable(GL_DEPTH_TEST);
        CheckGLError();
        glDepthFunc(GL_LESS);
        CheckGLError();
        depth_shader -> use();
        auto vp = light.vp();
        for(auto &[name, mesh]: meshes) {
            if(_model.count(name)) {
                for(auto model: _model[name]) {
                    depth_shader ->set_transform(vp * model);
                    mesh->draw_depth();
                }
            } else {
                depth_shader -> set_transform(vp);
                mesh->draw_depth();
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CheckGLError();
    }
}


struct MeshInfo {
    std::string name;
    Path path;
    glm::vec3 scale, translate;
    MeshInfo() : name(""), path(""), scale(1), translate(0)
    {
    }
};

void Scene::load(Path path) {
    int type = 0;
    printf("Scene: Load from %s\n", path.u8string().c_str());
    clock_t begin_time = clock();
    auto filename = path.filename().u8string();
    FILE *f = fopen(path.u8string().c_str(), "r");
    if(f == nullptr) 
        throw "fail to open file";
    if(filename.size() < 6 || filename.substr(filename.size() - 6, 6) != ".scene") 
        warn(2, "%s: not a .scene file", filename.c_str());
    static char buf[BUFFLEN];
    std::stack <std::pair <void *, int> > stk; 
    auto fmte = [&](char *str = nullptr) {
        while(!stk.empty()) {
            if(stk.top().second <= 1) {
                delete stk.top().first;
            }
            stk.pop();
        }
        throw std::string("Format error") + str;
    };
    while(std::fgets(buf, BUFFLEN, f) != nullptr) {
        size_t len = strlen(buf);
        while(len && isspace(buf[len - 1])) buf[--len] = 0;
        if(len == BUFFLEN) warn(3, "%s: Line length exceeded bufflen", filename.c_str());
        char *pos = strstr(buf, "#");
        if(pos != nullptr) *pos = '\0';
        pos = nspace(buf), len = strlen(pos);
        if(len <= 1) continue;
        if(str_equal(pos, "mesh")) {
            stk.push(std::make_pair(new MeshInfo, 0));
        } else if(str_equal(pos, "name")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 5);
            if(stk.top().second == 0) ((MeshInfo*)stk.top().first) -> name = pos;
        } else if(str_equal(pos, "path")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 5);
            if(stk.top().second == 0) ((MeshInfo*)stk.top().first) -> path = pos;
        } else if(str_equal(pos, "translate")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 9);
            if(stk.top().second == 0) {
                readvec3(pos, &(((MeshInfo*)stk.top().first) -> translate), "mesh.translate");
            }
        } else if(str_equal(pos, "scale")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 5);
            if(stk.top().second == 0) {
                readvec3(pos, &(((MeshInfo*)stk.top().first) -> scale), "mesh.scale");
            }
        } else if(str_equal(pos, "light")) {
            stk.push(std::make_pair(new LightInfo, 1));
        } else if(str_equal(pos, "camera")) {
            if(stk.empty() || stk.top().second != 1) fmte();
            stk.push(std::make_pair(&(((LightInfo*)stk.top().first) -> camera), 2));
        } else if(str_equal(pos, "pos")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 3);
            if(stk.top().second == 2) {
                readvec3(pos, &(((Camera*)stk.top().first) -> position), "camera.pos");
            }
        } else if(str_equal(pos, "pitch")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 5);
            if(stk.top().second == 2) {
                readfloat(pos, &(((Camera*)stk.top().first) -> pitch), "camera.pitch");
            }
        } else if(str_equal(pos, "yaw")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 3);
            if(stk.top().second == 2) {
                readfloat(pos, &(((Camera*)stk.top().first) -> yaw), "camera.yaw");
            }
        } else if(str_equal(pos, "intense")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 7);
            if(stk.top().second == 1) {
                readvec3(pos, &(((LightInfo*)stk.top().first) -> intense), "light.intense");
            }
        } else if(str_equal(pos, "type")) {
            if(stk.empty()) fmte();
            pos = nspace(pos + 4);
            if(stk.top().second == 1) {
                int x = 0;
                readint(pos, &x,  "light.type");
                if(x < 0 || x > 2) fmte("light.type: 0 ~ 2");
                ((LightInfo*)stk.top().first) -> type = LightType(x);
            }
        } else if(str_equal(pos, "end")) {
            if(stk.empty()) fmte("unmatched end");
            switch(stk.top().second) {
                case 2: {
                    stk.pop();
                    break;
                }
                case 1: {
                    light_info.push_back(*(LightInfo*)(stk.top().first));
                    delete stk.top().first;
                    stk.pop();
                    break;
                }
                case 0: {
                    auto info = (MeshInfo*)stk.top().first;
                    load_mesh(info->name, info->path);
                    _model[info->name] = {glm::translate(glm::mat4(1.f), info->translate) * glm::scale(glm::mat4(1.f), info->scale)};
                    delete stk.top().first;
                    stk.pop();
                    break;
                }
            }
        }
    }
}