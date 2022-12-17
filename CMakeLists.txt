cmake_minimum_required(VERSION 3.10)
project(skygfx-shadertoy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
	set(CMAKE_MACOSX_BUNDLE YES)
endif()

set(LIBS_FOLDER "libs")

file(GLOB ALL_SRC 
	src/*.cpp
	src/*.h
	lib/skygfx/examples/utils/*.cpp
	lib/skygfx/examples/utils/*.h
	lib/skygfx/examples/lib/imgui/backends/imgui_impl_glfw.cpp
	${RESOURCES}
)

add_executable(${PROJECT_NAME} ${ALL_SRC})

# emscripten

if(EMSCRIPTEN)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index")
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output)
	set(CMAKE_EXECUTABLE_SUFFIX .html)

	set(LINKER_FLAGS "${LINKER_FLAGS} -s NO_DISABLE_EXCEPTION_CATCHING=1")
	set(LINKER_FLAGS "${LINKER_FLAGS} -s DISABLE_EXCEPTION_THROWING=0")
	set(LINKER_FLAGS "${LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH")
	set(LINKER_FLAGS "${LINKER_FLAGS} -s USE_WEBGL2")
	set(LINKER_FLAGS "${LINKER_FLAGS} -s USE_GLFW=3")
	set(LINKER_FLAGS "${LINKER_FLAGS} --shell-file ${CMAKE_SOURCE_DIR}/emscripten/shell.html")

	set(COMPILER_FLAGS "${COMPILER_FLAGS} -s NO_DISABLE_EXCEPTION_CATCHING")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -s DISABLE_EXCEPTION_THROWING=0")

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
endif()

# skygfx

add_subdirectory(lib/skygfx)
set_property(TARGET skygfx PROPERTY FOLDER ${LIBS_FOLDER})
target_link_libraries(${PROJECT_NAME} skygfx)

# glfw

if (NOT EMSCRIPTEN)
	set(GLFW_INSTALL OFF)
	set(GLFW_BUILD_DOCS OFF)
	add_subdirectory(lib/skygfx/examples/lib/glfw)
	set_property(TARGET glfw PROPERTY FOLDER ${LIBS_FOLDER}/glfw)
	set_property(TARGET update_mappings PROPERTY FOLDER ${LIBS_FOLDER}/glfw)
	target_link_libraries(${PROJECT_NAME} glfw)
endif()

# imgui

file(GLOB IMGUI_SRC 
	lib/skygfx/examples/lib/imgui/*.cpp 
	lib/skygfx/examples/lib/imgui/*.h
)
target_include_directories(${PROJECT_NAME} PUBLIC
	lib/skygfx/examples/lib/imgui
	lib/skygfx/examples/lib/imgui/backends
)
add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} imgui)
set_property(TARGET imgui PROPERTY FOLDER ${LIBS_FOLDER})
