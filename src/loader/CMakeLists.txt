add_library(
    loader loader.hpp loader.cpp
    texture.hpp texture.cpp 
    common.hpp common.cpp 
    bound.hpp bound.cpp 
    material.hpp material.cpp 
    shader.hpp shader.cpp 
    particle.hpp particle.cpp
    # text.hpp text.cpp
)
target_compile_features(loader PRIVATE cxx_std_17)
target_link_libraries(loader PUBLIC glm glew_s glfw tinygltf)
target_include_directories(loader PUBLIC ${CMAKE_SOURCE_DIR}/third_party/stb/include)
target_include_directories(loader PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glew/include)
# target_include_directories(loader PUBLIC ${CMAKE_SOURCE_DIR}/third_party/freetype/include)
