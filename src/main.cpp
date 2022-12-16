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
