#include <skygfx/skygfx.h>
#include "../lib/skygfx/examples/utils/utils.h"
#include "../lib/skygfx/examples/utils/imgui_helper.h"
#include <imgui.h>
#include <imgui_stdlib.h>

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
void drawShaderEditor();

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
		drawShaderEditor();
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

layout(location = 0) out struct
{
	vec3 position;
	vec4 color;
} Out;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.color = aColor;
	Out.position = aPosition;
	gl_Position = vec4(aPosition, 1.0);
})";

const std::string fragment_shader_code = R"(
#version 450 core

layout(location = 0) out vec4 result;
layout(location = 0) in struct
{
	vec3 position;
	vec4 color;
} In;

void main()
{
	result = In.color;
})";

using Vertex = skygfx::Vertex::PositionColor;

const std::vector<Vertex> vertices = {
	{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
};

const std::vector<uint32_t> indices = { 0, 1, 2 };

std::shared_ptr<skygfx::Shader> shader = nullptr;

void drawTriangle()
{
	if (shader == nullptr)
		shader = std::make_shared<skygfx::Shader>(Vertex::Layout, vertex_shader_code, fragment_shader_code);

	skygfx::SetTopology(skygfx::Topology::TriangleList);
	skygfx::SetShader(*shader);
	skygfx::SetDynamicIndexBuffer(indices);
	skygfx::SetDynamicVertexBuffer(vertices);
	skygfx::Clear(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
	skygfx::DrawIndexed(static_cast<uint32_t>(indices.size()));
}

std::string status = "Idle";

void compile(const std::string& new_fragment_shader_code)
{
	try
	{
		shader = std::make_shared<skygfx::Shader>(Vertex::Layout, vertex_shader_code, new_fragment_shader_code);
		status = "compiled";
	}
	catch (const std::runtime_error& e)
	{
		status = e.what();
	}
}

void drawShaderEditor()
{
	ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);

	ImGui::Begin("Fragment shader");
	ImGui::Text("%s", status.c_str());

	static auto frag_source = fragment_shader_code;

	ImGui::InputTextMultiline("src", &frag_source, ImVec2(-1, -1), ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData* data) {
		compile(data->Buf);
		return 0;
	});

	ImGui::End();
}
