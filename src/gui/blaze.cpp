#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL_syswm.h>
#include <Windows.h>
#include <blaze/color.hpp>
#include <map>

namespace Blaze {
	static constexpr int defaultWindowWidth         = 800;
	static constexpr int defaultWindowHeight        = 600;
	static constexpr const char* defaultWindowTitle = "Blaze";
	static constexpr Color defaultWindowColor { 0, 0, 0 };

	static constexpr UINT_PTR ID_FILE_EXIT = 1;
} // namespace Blaze

int main(int argc, char** argv) {
	SDL_Window* mainWindow;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Event event;
	std::map<int, bool> keyboard;
	bool running = true;
	SDL_SysWMinfo mainWindowInfo;
	HWND win32MainWindow = nullptr;
	HMENU mainMenu = nullptr;
	HMENU fileMenu = nullptr;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	if (SDL_CreateWindowAndRenderer(Blaze::defaultWindowWidth, Blaze::defaultWindowHeight, SDL_WINDOW_RESIZABLE, &mainWindow, &renderer) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main window and renderer: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_SetWindowTitle(mainWindow, Blaze::defaultWindowTitle);

	SDL_VERSION(&mainWindowInfo.version);
	if (!SDL_GetWindowWMInfo(mainWindow, &mainWindowInfo)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get window handle: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
		return 1;
	}

	win32MainWindow = mainWindowInfo.info.win.window;

	// set up the menus
	{
		mainMenu = CreateMenu();
		fileMenu = CreateMenu();

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "File");

		AppendMenu(fileMenu, MF_STRING, Blaze::ID_FILE_EXIT, "Exit");

		SetMenu(win32MainWindow, mainMenu);
	}

	// enable Win32 events in the SDL event loop
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	// main event loop
	while (running) {
		SDL_PollEvent(&event);

		switch (event.type) {
			case SDL_QUIT:
				// exit if window close
				running = false;
				break;

			case SDL_KEYDOWN:
				keyboard[event.key.keysym.sym] = false;
				break;

			case SDL_KEYUP:
				keyboard[event.key.keysym.sym] = true;
				break;

			case SDL_SYSWMEVENT:
				if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
					switch (LOWORD(event.syswm.msg->msg.win.wParam)) {
						case Blaze::ID_FILE_EXIT: {
							running = false;
						} break;
					}
				}
				break;

			default:
				break;
		}

		if (!running) {
			break;
		}

		// TODO: handle input

		// clear the window
		SDL_SetRenderDrawColor(renderer, Blaze::defaultWindowColor.r, Blaze::defaultWindowColor.g, Blaze::defaultWindowColor.b, Blaze::defaultWindowColor.a);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(mainWindow);

	SDL_Quit();

	return 0;
};
