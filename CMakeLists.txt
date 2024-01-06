set(GLFW_INCLUDE ${CMAKE_SOURCE_DIR}/3rdParty/glfw/include)
set(GLM_INCLUDE ${CMAKE_SOURCE_DIR}/3rdParty/glm)
set(VULKAN_DIR $ENV{VULKAN_SDK})
string(REPLACE "\\" "/" VULKAN_DIR ${VULKAN_DIR})
set(VULKAN_INCLUDE ${VULKAN_DIR}/include)
set(VULKAN_LIB_DIR ${VULKAN_DIR}/Lib)

add_subdirectory(./3rdParty)
add_subdirectory(./src)