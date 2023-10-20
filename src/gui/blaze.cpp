#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <blaze/color.hpp>
#include <map>

// Define SNES key constants
#define SNES_KEY_UP      0
#define SNES_KEY_DOWN    1
#define SNES_KEY_LEFT    2
#define SNES_KEY_RIGHT   3
#define SNES_KEY_A       4
#define SNES_KEY_B       5
#define SNES_KEY_X       6
#define SNES_KEY_Y       7
#define SNES_KEY_START   8
#define SNES_KEY_SELECT  9
#define SNES_KEY_L       10
#define SNES_KEY_R       11


namespace Blaze {
	static constexpr int defaultWindowWidth         = 800;
	static constexpr int defaultWindowHeight        = 600;
	static constexpr const char* defaultWindowTitle = "Blaze";
	static constexpr Color defaultWindowColor { 0, 0, 0 };
} // namespace Blaze

// Function to map SDL keycodes to SNES keys
int mapSDLToSNES(SDL_Keycode sdlKey) {

    switch (sdlKey) {
        case SDLK_UP:
            return SNES_KEY_UP;
        case SDLK_DOWN:
            return SNES_KEY_DOWN;
        case SDLK_LEFT:
            return SNES_KEY_LEFT;
        case SDLK_RIGHT:
            return SNES_KEY_RIGHT;
        case SDLK_x:
            return SNES_KEY_A; // map x key to snes A
        case SDLK_z:
            return SNES_KEY_B; // map z key to snes B
        case SDLK_v:
            return SNES_KEY_X; // map v key to snes X
        case SDLK_c:
            return SNES_KEY_Y; // map c key to snes y
        case SDLK_RETURN: // could change start mapping
            return SNES_KEY_START;
        case SDLK_SPACE:  // could change select mapping
            return SNES_KEY_SELECT;
        case SDLK_a:
            return SNES_KEY_L;
        case SDLK_s:
            return SNES_KEY_R;
        default:
            return -1; // unmapped keys
    }
}

int main(int argc, char** argv) {
	SDL_Window* mainWindow;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Event event;
    std::map<int, bool> keyboard;

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
        int snesKey;

        switch (event.type) {
            case SDL_QUIT:
                // exit if window
                break;
            case SDL_KEYDOWN:
                snesKey = mapSDLToSNES(event.key.keysym.sym);
                // update emulator state
                break;
            case SDL_KEYUP:
                 snesKey = mapSDLToSNES(event.key.keysym.sym);
                // update emulator state
                break;
            default:
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
