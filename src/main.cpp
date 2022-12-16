#include <skygfx/skygfx.h>
#include "../lib/skygfx/examples/utils/utils.h"
#include "../lib/skygfx/examples/utils/imgui_helper.h"
#include <imgui.h>

#ifdef EMSCRIPTEN
#include <emscripten/html5.h>

std::function<void(int, int)> resize_func;
EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent *e, void *userData)
{
	int w = (int) e->windowInnerWidth;
	int h = (int) e->windowInnerHeight;
	//w *= emscripten_get_device_pixel_ratio();
	//h *= emscripten_get_device_pixel_ratio();
	resize_func(w, h);
	return 0;
}
#endif

std::function<void()> loop_func;
void loop() { loop_func(); }

void drawTriangle();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto [window, native_window, width, height] = utils::SpawnWindow(800, 600, "skygfx-shadertoy");

	skygfx::Initialize(native_window, width, height);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		skygfx::Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	});

#ifdef EMSCRIPTEN
	resize_func = [&](int w, int h) {
		glfwSetWindowSize(window, w, h);
	};

	double canvas_w, canvas_h;
	emscripten_get_element_css_size("#canvas", &canvas_w, &canvas_h);
	glfwSetWindowSize(window, (int)canvas_w, (int)canvas_h);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, false, ResizeCallback);
#endif

	auto imgui = ImguiHelper(window);

	loop_func = [&]{
		imgui.newFrame();
		ImGui::ShowDemoWindow();
		skygfx::Clear();
		drawTriangle();
		imgui.draw();
		skygfx::Present();
		glfwPollEvents();
	};

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(loop, 0, 1);
#else
	while (!glfwWindowShouldClose(window)) { loop(); }
#endif

	skygfx::Finalize();

	glfwTerminate();

	return 0;
}

const std::string vertex_shader_code = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;

layout(location = 0) out struct { vec4 Color; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	gl_Position = vec4(aPosition, 1.0);
})";

const std::string fragment_shader_code = R"(
#version 450 core

layout(location = 0) out vec4 result;
layout(location = 0) in struct { vec4 Color; } In;

void main()
{
	result = In.Color;
})";

using Vertex = skygfx::Vertex::PositionColor;

const std::vector<Vertex> vertices = {
	{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
};

const std::vector<uint32_t> indices = { 0, 1, 2 };

void drawTriangle()
{
	static auto shader = skygfx::Shader(Vertex::Layout, vertex_shader_code, fragment_shader_code);

	skygfx::SetTopology(skygfx::Topology::TriangleList);
	skygfx::SetShader(shader);
	skygfx::SetDynamicIndexBuffer(indices);
	skygfx::SetDynamicVertexBuffer(vertices);
	skygfx::Clear(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
	skygfx::DrawIndexed(static_cast<uint32_t>(indices.size()));
}
