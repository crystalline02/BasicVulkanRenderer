# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build

# Include any dependencies generated for this target.
include src/CMakeFiles/VulkanRenderer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/VulkanRenderer.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/VulkanRenderer.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/VulkanRenderer.dir/flags.make

src/CMakeFiles/VulkanRenderer.dir/entry_point.obj: src/CMakeFiles/VulkanRenderer.dir/flags.make
src/CMakeFiles/VulkanRenderer.dir/entry_point.obj: src/CMakeFiles/VulkanRenderer.dir/includes_CXX.rsp
src/CMakeFiles/VulkanRenderer.dir/entry_point.obj: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/src/entry_point.cpp
src/CMakeFiles/VulkanRenderer.dir/entry_point.obj: src/CMakeFiles/VulkanRenderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/VulkanRenderer.dir/entry_point.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/VulkanRenderer.dir/entry_point.obj -MF CMakeFiles\VulkanRenderer.dir\entry_point.obj.d -o CMakeFiles\VulkanRenderer.dir\entry_point.obj -c C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\entry_point.cpp

src/CMakeFiles/VulkanRenderer.dir/entry_point.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanRenderer.dir/entry_point.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\entry_point.cpp > CMakeFiles\VulkanRenderer.dir\entry_point.i

src/CMakeFiles/VulkanRenderer.dir/entry_point.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanRenderer.dir/entry_point.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\entry_point.cpp -o CMakeFiles\VulkanRenderer.dir\entry_point.s

src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj: src/CMakeFiles/VulkanRenderer.dir/flags.make
src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj: src/CMakeFiles/VulkanRenderer.dir/includes_CXX.rsp
src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/src/vulkan_app.cpp
src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj: src/CMakeFiles/VulkanRenderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj -MF CMakeFiles\VulkanRenderer.dir\vulkan_app.obj.d -o CMakeFiles\VulkanRenderer.dir\vulkan_app.obj -c C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_app.cpp

src/CMakeFiles/VulkanRenderer.dir/vulkan_app.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanRenderer.dir/vulkan_app.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_app.cpp > CMakeFiles\VulkanRenderer.dir\vulkan_app.i

src/CMakeFiles/VulkanRenderer.dir/vulkan_app.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanRenderer.dir/vulkan_app.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_app.cpp -o CMakeFiles\VulkanRenderer.dir\vulkan_app.s

src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj: src/CMakeFiles/VulkanRenderer.dir/flags.make
src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj: src/CMakeFiles/VulkanRenderer.dir/includes_CXX.rsp
src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/src/vulkan_fn.cpp
src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj: src/CMakeFiles/VulkanRenderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj -MF CMakeFiles\VulkanRenderer.dir\vulkan_fn.obj.d -o CMakeFiles\VulkanRenderer.dir\vulkan_fn.obj -c C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_fn.cpp

src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanRenderer.dir/vulkan_fn.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_fn.cpp > CMakeFiles\VulkanRenderer.dir\vulkan_fn.i

src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanRenderer.dir/vulkan_fn.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\vulkan_fn.cpp -o CMakeFiles\VulkanRenderer.dir\vulkan_fn.s

src/CMakeFiles/VulkanRenderer.dir/resources.obj: src/CMakeFiles/VulkanRenderer.dir/flags.make
src/CMakeFiles/VulkanRenderer.dir/resources.obj: src/CMakeFiles/VulkanRenderer.dir/includes_CXX.rsp
src/CMakeFiles/VulkanRenderer.dir/resources.obj: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/src/resources.cpp
src/CMakeFiles/VulkanRenderer.dir/resources.obj: src/CMakeFiles/VulkanRenderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/VulkanRenderer.dir/resources.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/VulkanRenderer.dir/resources.obj -MF CMakeFiles\VulkanRenderer.dir\resources.obj.d -o CMakeFiles\VulkanRenderer.dir\resources.obj -c C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\resources.cpp

src/CMakeFiles/VulkanRenderer.dir/resources.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanRenderer.dir/resources.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\resources.cpp > CMakeFiles\VulkanRenderer.dir\resources.i

src/CMakeFiles/VulkanRenderer.dir/resources.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanRenderer.dir/resources.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\resources.cpp -o CMakeFiles\VulkanRenderer.dir\resources.s

src/CMakeFiles/VulkanRenderer.dir/model/model.obj: src/CMakeFiles/VulkanRenderer.dir/flags.make
src/CMakeFiles/VulkanRenderer.dir/model/model.obj: src/CMakeFiles/VulkanRenderer.dir/includes_CXX.rsp
src/CMakeFiles/VulkanRenderer.dir/model/model.obj: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/src/model/model.cpp
src/CMakeFiles/VulkanRenderer.dir/model/model.obj: src/CMakeFiles/VulkanRenderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/CMakeFiles/VulkanRenderer.dir/model/model.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/VulkanRenderer.dir/model/model.obj -MF CMakeFiles\VulkanRenderer.dir\model\model.obj.d -o CMakeFiles\VulkanRenderer.dir\model\model.obj -c C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\model\model.cpp

src/CMakeFiles/VulkanRenderer.dir/model/model.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanRenderer.dir/model/model.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\model\model.cpp > CMakeFiles\VulkanRenderer.dir\model\model.i

src/CMakeFiles/VulkanRenderer.dir/model/model.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanRenderer.dir/model/model.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src\model\model.cpp -o CMakeFiles\VulkanRenderer.dir\model\model.s

# Object files for target VulkanRenderer
VulkanRenderer_OBJECTS = \
"CMakeFiles/VulkanRenderer.dir/entry_point.obj" \
"CMakeFiles/VulkanRenderer.dir/vulkan_app.obj" \
"CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj" \
"CMakeFiles/VulkanRenderer.dir/resources.obj" \
"CMakeFiles/VulkanRenderer.dir/model/model.obj"

# External object files for target VulkanRenderer
VulkanRenderer_EXTERNAL_OBJECTS =

C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/entry_point.obj
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/vulkan_app.obj
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/vulkan_fn.obj
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/resources.obj
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/model/model.obj
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/build.make
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/lib/libglfw3.a
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: D:/VulkanSDK/1.3.268.0/Lib/vulkan-1.lib
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/linkLibs.rsp
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/objects1.rsp
C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe: src/CMakeFiles/VulkanRenderer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\bin\VulkanRenderer.exe"
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\VulkanRenderer.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/VulkanRenderer.dir/build: C:/Users/ZhenKou/OneDrive/project/MyVulkanRenderer/bin/VulkanRenderer.exe
.PHONY : src/CMakeFiles/VulkanRenderer.dir/build

src/CMakeFiles/VulkanRenderer.dir/clean:
	cd /d C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src && $(CMAKE_COMMAND) -P CMakeFiles\VulkanRenderer.dir\cmake_clean.cmake
.PHONY : src/CMakeFiles/VulkanRenderer.dir/clean

src/CMakeFiles/VulkanRenderer.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\src C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src C:\Users\ZhenKou\OneDrive\project\MyVulkanRenderer\build\src\CMakeFiles\VulkanRenderer.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/VulkanRenderer.dir/depend
