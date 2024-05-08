add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE glfw glew_s glm loader imgui)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/third_party/glew/include)
# target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/third_party/freetype/include)

add_subdirectory(loader)