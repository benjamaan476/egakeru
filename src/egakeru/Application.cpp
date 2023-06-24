#include "Application.h"
#include "Log/Log.h"

#include "Instrumentor.h"

Application::Application(std::string_view name, uint32_t width, uint32_t height)
{
	Log::Init();
	LOG_INFO("Hello");
	PROFILE_BEGIN_SESSION("Startup", "GameEngine-Startup.json");
	_instance = this;
	initWindow(name, width, height);

	egkr::egakeru::create();
	image = egkr::Texture2D::createFromFile("chess.png");

	chessSprite = egkr::egakeru::createSprite(image);
	initGui();
	PROFILE_END_SESSION();
}

void Application::run()
{
	mainLoop();
	cleanup();
}

void Application::initWindow(std::string_view name, uint32_t width, uint32_t height)
{
	PROFILE_FUNCTION()
	WindowProperties windowProperties{ name.data(), width, height};

	auto callback = std::bind(&Application::dragDropCallback, this, std::placeholders::_1, std::placeholders::_2);

	auto cbk = [](int pathCount, const char** paths)
	{
		if (pathCount)
		{
			std::string str(paths[0]);
			LOG_TRACE("File Dropped: {0}", str);
		}
	};

	_window = Window::create(windowProperties, callback);
}

void Application::initGui()
{
	_gui = Gui::create();
}

void Application::mainLoop()
{
	bool show_demo_window = false;
	float4 clear_color{ 0.45f, 0.55f, 0.60f, 1.00f };
	int counter = 0;

	while (!_window->isRunning())
	{
		_window->OnUpdate();
		
		{
			_gui->begin();

			_gui->demo(show_demo_window);
			
			boardProperties.onRender(_gui.get());

			Gui::Window wind(_gui.get(), "Hello, World");

			if(wind.button("Show debug window"))
			{
				show_demo_window = !show_demo_window;
			}


			wind.text("This is some useful text.");               // Display some text (you can use a format strings too)
			wind.checkbox("Demo Window", show_demo_window);      // Edit bools storing our window open/close state

			//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//wind.rgbaColour("clear color", clearColor); // Edit 3 floats representing a color

			if (wind.button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;

			auto count = std::format("counter = {}", counter);
			wind.text(count, true);

			auto& textureSampler = egkr::egakeru::getTextureSampler();

			if (dropped)
			{
				dropped = false;
				if (wind.dragDropSource("dasf", "Image drop", _droppedPayload.c_str(), Gui::DragDropFlags::Extern))
				{
					wind.endDropSource();

				}
			}
			wind.image("Image", image, textureSampler);

			std::string payload;
			auto dest = wind.dragDropDestination("Image drop", payload);


			if (dest)
			{
				//Renderer::createImage(payload);
				image = egkr::Texture2D::createFromFile(payload);
				wind.endDropDestination();
			}

			Gui::Window sprite(_gui.get(), "Sprite Editor");
			sprite.var("Sprite Size", chessSprite->size, 0.f, 1000.f, 10.f);
			sprite.var("Sprite Position", chessSprite->position, 0.f, 1000.f, 10.f);
			sprite.var("Sprite Rotation", chessSprite->rotation, 0.f, 360.f, 1.f);
			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		egkr::egakeru::drawFrame(boardProperties);
	}

	egkr::state.device.waitIdle();
}

void Application::destroyUi()
{
	Gui::release();
}

void Application::cleanup()
{
	destroyUi();
	chessSprite->destory();
	image.destroy();
	egkr::egakeru::cleanup();

}

void Application::dragDropCallback(int pathCount, const char** paths)
{
	if(pathCount)
	{
		dropped = true;
		_droppedPayload = std::string{ paths[0] };

		std::string str(paths[0]);
		LOG_TRACE("File Dropped: {0}", str);
	}
}
