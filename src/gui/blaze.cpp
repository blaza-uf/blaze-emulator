#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL_syswm.h>
#include <blaze/color.hpp>
#include <map>
#include <string>
#include <sstream>
#include <blaze/Bus.hpp>
#include <SDL_ttf.h>

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

#ifdef _WIN32
	#include <windows.h>
	#include <shobjidl.h>
#endif // _WIN32

namespace Blaze {
	static constexpr int defaultWindowWidth         = 800;
	static constexpr int defaultWindowHeight        = 600;
	static constexpr const char* defaultWindowTitle = "Blaze";
	static constexpr Color defaultWindowColor { 0, 0, 0 };

#ifdef _WIN32
	enum MenuID: UINT_PTR {
		FileExit = 1,
		FileOpen = 2,
		FileClose = 3,
		EditOptions = 4,
		ViewShowDebugger = 5,
		HelpHelp = 6,
	};
#endif // _WIN32
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

#ifdef _WIN32
static bool openROMDialog(std::string& outPath) {
	HRESULT hr;
	IFileDialog* fileDialog = nullptr;
	IShellItem* item = nullptr;
	LPWSTR filePath = nullptr;
	int requiredChars = 0;
	int writtenChars = 0;

	hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr)) {
		return false;
	}

	hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileDialog));
	if (!SUCCEEDED(hr)) {
		CoUninitialize();
		return false;
	}

	hr = fileDialog->Show(nullptr);
	if (!SUCCEEDED(hr)) {
		fileDialog->Release();
		CoUninitialize();
		return false;
	}

	hr = fileDialog->GetResult(&item);
	if (!SUCCEEDED(hr)) {
		fileDialog->Release();
		CoUninitialize();
		return false;
	}

	hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	if (!SUCCEEDED(hr)) {
		item->Release();
		fileDialog->Release();
		CoUninitialize();
		return false;
	}

	requiredChars = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
	if (requiredChars == 0) {
		CoTaskMemFree(filePath);
		item->Release();
		fileDialog->Release();
		CoUninitialize();
		return false;
	}

	outPath.resize(requiredChars);

	writtenChars = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, outPath.data(), requiredChars, nullptr, nullptr);
	if (writtenChars == 0) {
		CoTaskMemFree(filePath);
		item->Release();
		fileDialog->Release();
		CoUninitialize();
		return false;
	}

	outPath.resize(writtenChars);

	CoTaskMemFree(filePath);
	item->Release();
	fileDialog->Release();
	CoUninitialize();
	return true;
};
#endif

static int createText(const std::string& text, const SDL_Color& color, TTF_Font* font, SDL_Renderer* renderer, SDL_Texture*& outTexture, int& outWidth, int& outHeight) {
	SDL_Surface* tmpSurface = TTF_RenderText_Solid(font, text.c_str(), color);
	if (!tmpSurface) {
		return -1;
	}

	outTexture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	if (!outTexture) {
		SDL_FreeSurface(tmpSurface);
		return -1;
	}

	outWidth = tmpSurface->w;
	outHeight = tmpSurface->h;

	SDL_FreeSurface(tmpSurface);

	return 0;
};

int main(int argc, char** argv) {
	SDL_Window* mainWindow;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Event event;
	std::map<int, bool> keyboard;
	bool running = true;
	SDL_SysWMinfo mainWindowInfo;
	Blaze::Bus bus;
	TTF_Font* font = nullptr;

#ifdef _WIN32
	HWND win32MainWindow = nullptr;
	HMENU mainMenu = nullptr;
	HMENU fileMenu = nullptr;
	HMENU editMenu = nullptr;
	HMENU viewMenu = nullptr;
	HMENU helpMenu = nullptr;
#endif // _WIN32

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	if (TTF_Init() < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL_ttf: %s", TTF_GetError());
		return 1;
	}

#ifdef _WIN32
	// TODO: fall back to other fonts
	font = TTF_OpenFont("C:\\Windows\\Fonts\\FiraCode-Regular.ttf", 16);
#else
	#warning TODO
#endif
	if (!font) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s", TTF_GetError());
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

#ifdef _WIN32
	win32MainWindow = mainWindowInfo.info.win.window;

	// set up the menus
	{
		mainMenu = CreateMenu();
		fileMenu = CreateMenu();
		editMenu = CreateMenu();
		viewMenu = CreateMenu();
		helpMenu = CreateMenu();

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "File");

		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileOpen, "Open ROM");
		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileClose, "Close ROM");
		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileExit, "Exit");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)editMenu, "Edit");

		AppendMenu(editMenu, MF_STRING, Blaze::MenuID::EditOptions, "Options");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)viewMenu, "View");

		AppendMenu(viewMenu, MF_STRING, Blaze::MenuID::ViewShowDebugger, "Show Debugger");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)helpMenu, "Help");

		AppendMenu(helpMenu, MF_STRING, Blaze::MenuID::HelpHelp, "Help");

		SetMenu(win32MainWindow, mainMenu);
	}

	// enable Win32 events in the SDL event loop
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif // _WIN32

	std::string debugBuffer;
	bus.cpu.putCharacterHook = [&](char character) {
		debugBuffer.push_back(character);
	};

	// main event loop
	while (running) {
		// process all events for this frame
		while (SDL_PollEvent(&event)) {
			int snesKey;

			switch (event.type) {
				case SDL_QUIT:
					// exit if window closed
					running = false;
					break;

				case SDL_KEYDOWN:
					snesKey = mapSDLToSNES(event.key.keysym.sym);
					// update emulator state
					break;

				case SDL_KEYUP:
					snesKey = mapSDLToSNES(event.key.keysym.sym);
					// update emulator state
					break;

#ifdef _WIN32
			case SDL_SYSWMEVENT:
				if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
					switch (static_cast<Blaze::MenuID>(LOWORD(event.syswm.msg->msg.win.wParam))) {
						case Blaze::MenuID::FileOpen: {
							std::string path;
							std::stringstream output;

							if (openROMDialog(path)) {
								output << "Got ROM: " << path;
							} else {
								output << "Failed to open ROM selection dialog";
							}

							OutputDebugStringA(output.str().c_str());
						} break;

						case Blaze::MenuID::FileClose: {
							// TODO
						} break;

						case Blaze::MenuID::FileExit: {
							running = false;
						} break;

						case Blaze::MenuID::EditOptions: {
							// TODO
						} break;

						case Blaze::MenuID::ViewShowDebugger: {
							// TODO
						} break;

						case Blaze::MenuID::HelpHelp: {
							// TODO
						} break;

						default: {
							// what to do here?
						} break;
					}
				}
				break;
#endif // _WIN32

			default:
				break;
			}
		}

		if (!running) {
			break;
		}

		// clear the window
		SDL_SetRenderDrawColor(renderer, Blaze::defaultWindowColor.r, Blaze::defaultWindowColor.g, Blaze::defaultWindowColor.b, Blaze::defaultWindowColor.a);
		SDL_RenderClear(renderer);

		// execute a single instruction
		bus.cpu.execute();

		// render the debug buffer
		if (!debugBuffer.empty()) {
			SDL_Rect rect = {
				0, 0,
			};
			SDL_Color color = {
				// white
				255, 255, 255,
			};
			SDL_Texture* texture = nullptr;

			if (createText(debugBuffer, color, font, renderer, texture, rect.w, rect.h) < 0) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create text texture: %s", SDL_GetError());
				abort();
			}
			SDL_RenderCopy(renderer, texture, nullptr, &rect);
			SDL_DestroyTexture(texture);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(mainWindow);

	TTF_CloseFont(font);

	TTF_Quit();
	SDL_Quit();

	return 0;
};
