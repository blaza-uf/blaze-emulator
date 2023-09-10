#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <blaze/color.hpp>

namespace Blaze {
	static constexpr int defaultWindowWidth         = 800;
	static constexpr int defaultWindowHeight        = 600;
	static constexpr const char* defaultWindowTitle = "Blaze";
	static constexpr Color defaultWindowColor { 0, 0, 0 };
} // namespace Blaze

int main(int argc, char** argv) {
	SDL_Window* mainWindow;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Event event;

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

	// main event loop
	while (true) {
		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT) {
			// user wants to quit the application
			break;
		}

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
