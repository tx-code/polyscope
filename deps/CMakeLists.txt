if("${POLYSCOPE_BACKEND_OPENGL3_GLFW}")
  # # Glad
  find_package(Glad REQUIRED)

  # # GLFW
  find_package(glfw3 REQUIRED)
endif()

# # Imgui
find_package(imgui REQUIRED)

# # MarchingCube
if(NOT TARGET MarchingCube::MarchingCube)
  add_subdirectory(MarchingCubeCpp)
endif()

# # stb
find_package(stb REQUIRED)