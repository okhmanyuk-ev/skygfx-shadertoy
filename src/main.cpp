#include <skygfx/skygfx.h>
#include "../lib/skygfx/examples/utils/utils.h"
#include "../lib/skygfx/examples/utils/imgui_helper.h"
#include <imgui.h>
#include <imgui_stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

int main(int argc, char *argv[])
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
layout(location = TEXCOORD_LOCATION) in vec2 aTexCoord;

layout(location = 0) out struct { vec4 Color; vec2 TexCoord; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	Out.TexCoord = aTexCoord;
#ifdef FLIP_TEXCOORD_Y
	Out.TexCoord.y = 1.0 - Out.TexCoord.y;
#endif
	gl_Position = vec4(aPosition, 1.0);
})";

const std::string fragment_shader_code = R"(
#version 450 core

layout(binding = 1) uniform Settings
{
	float time;
} settings;

layout(location = 0) out vec4 result;
layout(location = 0) in struct { vec4 Color; vec2 TexCoord; } In;

layout(binding = 0) uniform sampler2D sTexture;

void main()
{
	result = In.Color * texture(sTexture, In.TexCoord);
})";

using Vertex = skygfx::Vertex::PositionColorTexture;

const std::vector<Vertex> vertices = {
	{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }, // bottom right
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }, // bottom left
	{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } }, // top right
	{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }, // top left
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };

struct alignas(16) Settings
{
	float time = 0.0f;
} settings;

std::shared_ptr<skygfx::Shader> shader = nullptr;
std::shared_ptr<skygfx::Texture> texture = nullptr;

std::string status = "idle";

void make_shader(const std::string& vertex_code, const std::string& fragment_code)
{
	try
	{
		shader = std::make_shared<skygfx::Shader>(Vertex::Layout, vertex_code, fragment_code);
		status = "compiled";
	}
	catch (const std::runtime_error& e)
	{
		status = e.what();
	}
}

void drawTriangle()
{
	if (shader == nullptr)
	{
		make_shader(vertex_shader_code, fragment_shader_code);
	}

	if (texture == nullptr)
	{
		int tex_width = 0;
		int tex_height = 0;
		void* tex_memory = stbi_load("assets/bricks.png", &tex_width, &tex_height, nullptr, 4); // TODO: this image has 3 channels, we must can load that type of images

		texture = std::make_shared<skygfx::Texture>(tex_width, tex_height, 4/*TODO: no magic numbers should be*/, tex_memory, true);
	}

	settings.time = static_cast<float>(glfwGetTime());

	skygfx::SetTopology(skygfx::Topology::TriangleList);
	skygfx::SetSampler(skygfx::Sampler::Linear);
	skygfx::SetShader(*shader);
	skygfx::SetDynamicVertexBuffer(vertices);
	skygfx::SetDynamicIndexBuffer(indices);
	skygfx::SetTexture(0, *texture);
	skygfx::SetDynamicUniformBuffer(1, settings);

	skygfx::Clear(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
	skygfx::DrawIndexed(static_cast<uint32_t>(indices.size()));
}

void drawShaderEditor()
{
	ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);

	ImGui::Begin("Fragment shader");
	ImGui::Text("%s", status.c_str());

	static auto frag_source = fragment_shader_code;

	ImGui::InputTextMultiline("src", &frag_source, ImVec2(-1, -1), ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData* data) {
		make_shader(vertex_shader_code, data->Buf);
		return 0;
	});

	ImGui::End();
}
