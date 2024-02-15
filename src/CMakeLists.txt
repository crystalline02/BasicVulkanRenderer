project(VulkanRenderer LANGUAGES CXX)
set(SOURCES
    entry_point.cpp
    vulkan_app.cpp
    vulkan_fn.cpp
    model.cpp
    resources.cpp
    )
add_executable(VulkanRenderer ${SOURCES})
target_include_directories(VulkanRenderer PRIVATE
    ${GLFW_INCLUDE}
    ${VULKAN_INCLUDE}
    ${GLM_INCLUDE}
    ${STBIMAGE_INCLUDE}
    ${TINYOBJLOADER_INCLUDE}
    )
target_link_libraries(VulkanRenderer PRIVATE
    glfw
    ${VULKAN_LIB_DIR}/vulkan-1.lib)
set_target_properties(VulkanRenderer PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
target_compile_features(VulkanRenderer PRIVATE cxx_std_20)