add_library(
    util
    mesh.hpp mesh.cpp
    texture.hpp texture.cpp 
    common.hpp common.cpp 
    bound.hpp bound.cpp 
    material.hpp material.cpp 
    shader.hpp shader.cpp 
    particle.hpp particle.cpp
    scene.hpp scene.cpp
    camera.hpp camera.cpp
)
target_compile_features(util PRIVATE cxx_std_17)
target_link_libraries(util PUBLIC glm glew_s glfw stb)
target_include_directories(util PUBLIC ${CMAKE_SOURCE_DIR}/third_party/stb/include)
target_include_directories(util PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glew/include)
# target_include_directories(util PUBLIC ${CMAKE_SOURCE_DIR}/third_party/freetype/include)
