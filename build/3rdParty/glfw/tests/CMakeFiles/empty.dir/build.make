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
CMAKE_SOURCE_DIR = C:\Users\ZhenKou\OneDrive\project\vulkan_learn

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build

# Include any dependencies generated for this target.
include 3rdParty/glfw/tests/CMakeFiles/empty.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include 3rdParty/glfw/tests/CMakeFiles/empty.dir/compiler_depend.make

# Include the progress variables for this target.
include 3rdParty/glfw/tests/CMakeFiles/empty.dir/progress.make

# Include the compile flags for this target's objects.
include 3rdParty/glfw/tests/CMakeFiles/empty.dir/flags.make

3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/flags.make
3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/includes_C.rsp
3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj: C:/Users/ZhenKou/OneDrive/project/vulkan_learn/3rdParty/glfw/tests/empty.c
3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object 3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT 3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj -MF CMakeFiles\empty.dir\empty.c.obj.d -o CMakeFiles\empty.dir\empty.c.obj -c C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\tests\empty.c

3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/empty.dir/empty.c.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\tests\empty.c > CMakeFiles\empty.dir\empty.c.i

3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/empty.dir/empty.c.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\tests\empty.c -o CMakeFiles\empty.dir\empty.c.s

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/flags.make
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/includes_C.rsp
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj: C:/Users/ZhenKou/OneDrive/project/vulkan_learn/3rdParty/glfw/deps/tinycthread.c
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj -MF CMakeFiles\empty.dir\__\deps\tinycthread.c.obj.d -o CMakeFiles\empty.dir\__\deps\tinycthread.c.obj -c C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\tinycthread.c

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/empty.dir/__/deps/tinycthread.c.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\tinycthread.c > CMakeFiles\empty.dir\__\deps\tinycthread.c.i

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/empty.dir/__/deps/tinycthread.c.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\tinycthread.c -o CMakeFiles\empty.dir\__\deps\tinycthread.c.s

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/flags.make
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/includes_C.rsp
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj: C:/Users/ZhenKou/OneDrive/project/vulkan_learn/3rdParty/glfw/deps/glad_gl.c
3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj: 3rdParty/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj -MF CMakeFiles\empty.dir\__\deps\glad_gl.c.obj.d -o CMakeFiles\empty.dir\__\deps\glad_gl.c.obj -c C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\glad_gl.c

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/empty.dir/__/deps/glad_gl.c.i"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\glad_gl.c > CMakeFiles\empty.dir\__\deps\glad_gl.c.i

3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/empty.dir/__/deps/glad_gl.c.s"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\deps\glad_gl.c -o CMakeFiles\empty.dir\__\deps\glad_gl.c.s

# Object files for target empty
empty_OBJECTS = \
"CMakeFiles/empty.dir/empty.c.obj" \
"CMakeFiles/empty.dir/__/deps/tinycthread.c.obj" \
"CMakeFiles/empty.dir/__/deps/glad_gl.c.obj"

# External object files for target empty
empty_EXTERNAL_OBJECTS =

3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/empty.c.obj
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.obj
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.obj
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/build.make
3rdParty/glfw/tests/empty.exe: C:/Users/ZhenKou/OneDrive/project/vulkan_learn/lib/libglfw3.a
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/linkLibs.rsp
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/objects1.rsp
3rdParty/glfw/tests/empty.exe: 3rdParty/glfw/tests/CMakeFiles/empty.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable empty.exe"
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\empty.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
3rdParty/glfw/tests/CMakeFiles/empty.dir/build: 3rdParty/glfw/tests/empty.exe
.PHONY : 3rdParty/glfw/tests/CMakeFiles/empty.dir/build

3rdParty/glfw/tests/CMakeFiles/empty.dir/clean:
	cd /d C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests && $(CMAKE_COMMAND) -P CMakeFiles\empty.dir\cmake_clean.cmake
.PHONY : 3rdParty/glfw/tests/CMakeFiles/empty.dir/clean

3rdParty/glfw/tests/CMakeFiles/empty.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\ZhenKou\OneDrive\project\vulkan_learn C:\Users\ZhenKou\OneDrive\project\vulkan_learn\3rdParty\glfw\tests C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests C:\Users\ZhenKou\OneDrive\project\vulkan_learn\build\3rdParty\glfw\tests\CMakeFiles\empty.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : 3rdParty/glfw/tests/CMakeFiles/empty.dir/depend

