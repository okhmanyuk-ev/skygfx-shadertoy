#include <skygfx/skygfx.h>
#include "../lib/skygfx/examples/utils/utils.h"
#include "../lib/skygfx/examples/utils/imgui_helper.h"
#include <imgui.h>

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

	auto imgui = ImguiHelper(window);

	loop_func = [&]{
		imgui.newFrame();
		ImGui::ShowDemoWindow();

		ImGui::Begin("Test");
		ImGui::Text("%d", skygfx::GetWidth());
		ImGui::Text("%d", skygfx::GetHeight());
		ImGui::Separator();
		
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(window, &w, &h);
		glfwGetFramebufferSize(window, &display_w, &display_h);

		ImGui::Text("%d", w);
		ImGui::Text("%d", h);
		ImGui::Separator();

		ImGui::Text("%d", display_w);
		ImGui::Text("%d", display_h);

		ImGui::End();

		skygfx::Clear();
		imgui.draw();
		skygfx::Present();
		glfwPollEvents();
	};

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(&loop, 0, 1);
#else
	while (!glfwWindowShouldClose(window)) { loop(); }
#endif

	skygfx::Finalize();

	glfwTerminate();

	return 0;
}
