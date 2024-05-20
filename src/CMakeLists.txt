add_executable(main main.cpp)



target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE glfw glew_s glm util imgui ${OPENGL_gl_LIBRARY})

#if (APPLE)
#    target_link_libraries(main PRIVATE "-framework Cocoa")
#    target_link_libraries(main PRIVATE "-framework OpenGL")
#    target_link_libraries(main PRIVATE "-framework IOKit")
#endif (APPLE)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/third_party/glew/include)

# target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/third_party/freetype/include)

add_subdirectory(util)