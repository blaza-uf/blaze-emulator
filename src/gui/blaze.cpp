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
#include <blaze/util.hpp>
#include <thread>
#include <mutex>
#include <blaze/PPU.hpp>
#include <blaze/APU.hpp>
#include <blaze/debug.hpp>

#include <GL/gl.h>

#ifdef _WIN32
	#include <windows.h>
	#include <windowsx.h>
	#include <shobjidl.h>
#endif // _WIN32

namespace Blaze {
	static constexpr int defaultWindowWidth         = 800;
	static constexpr int defaultWindowHeight        = 600;
	static constexpr const char* defaultWindowTitle = "Blaze";
	static constexpr Color defaultWindowColor { 0, 0, 0 };
	static constexpr int snesWidth = 352;
	static constexpr int snesHeight = 240;
	
	enum snesKeyConstants {
		SNES_KEY_UP = 0,
		SNES_KEY_DOWN = 1,
		SNES_KEY_LEFT = 2,
		SNES_KEY_RIGHT = 3,
		SNES_KEY_A = 4,
		SNES_KEY_B = 5,
		SNES_KEY_X = 6,
		SNES_KEY_Y = 7,
		SNES_KEY_START = 8,
		SNES_KEY_SELECT = 9,
		SNES_KEY_L = 10,
		SNES_KEY_R = 11,
	};

#ifdef _WIN32
	enum MenuID: UINT_PTR {
		FileExit = 1,
		FileOpen = 2,
		FileClose = 3,
		EditOptions = 4,
		ViewShowDebugger = 5,
		HelpHelp = 6,
		EditContinuousExecution = 7,
		ViewShowDebugConsole = 8,

		DebuggerTextView = 100,
		DebuggerContinue = 101,
		DebuggerPause = 102,
		DebuggerNext = 103,
		DebuggerInto = 104,
		DebuggerRegisterView = 105,
		DebuggerBreakpointAddressInput = 106,

		DebugConsoleTextView = 200,
	};

	static constexpr LPCSTR debuggerWindowClassName = TEXT("Blaze Debugger Window Class");
	static constexpr int defaultDebuggerWindowWidth = 400;
	static constexpr int defaultDebuggerWindowHeight = 600;
	static constexpr int debuggerButtonAreaHeight = 26;
	static constexpr int debuggerButtonY = 3;
	static constexpr int debuggerButtonHeight = 20;
	static constexpr int debuggerButtonXMargin = 5;
	static constexpr int debuggerButtonYMargin = 3;
	static constexpr int debuggerRegisterViewHeight = 180;
	static constexpr int debuggerBreakpointAddressInputHeight = 20;

	static constexpr LPCSTR debugConsoleWindowClassName = TEXT("Blaze Debug Console Window Class");
	static constexpr int defaultDebugConsoleWindowWidth = 400;
	static constexpr int defaultDebugConsoleWindowHeight = 600;

	static WNDCLASS debuggerWindowClass = {};
	static HMENU editMenu = nullptr;
	static WNDCLASS debugConsoleWindowClass = {};

	#define NEWLINE "\r\n"
#else
	#define NEWLINE "\n"
#endif // _WIN32

	static bool continuousExecution = true;
	static Bus bus;
	static Address breakpoint = UINT32_MAX;
	static bool romLoaded = false;
} // namespace Blaze

static void updateBreakpoint(Blaze::Address address, bool shouldUpdateTextField = true);

static void normalizeNewlines(std::string& string) {
	for (size_t idx = string.find('\n'); idx != std::string::npos; idx = string.find('\n', idx)) {
		string.replace(idx, 1, NEWLINE);
		idx += sizeof(NEWLINE) - 1; // skip over the newly added string
	}
};

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
        case SDLK_RETURN: // could change 'start' mapping
            return SNES_KEY_START;
        case SDLK_SPACE:  // could change 'select mapping
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
static void setContinuousExecution(bool continuousExecution) {
	Blaze::continuousExecution = continuousExecution;
	MENUITEMINFO info = {};

	info.cbSize = sizeof(info);
	info.fMask = MIIM_STATE;
	info.fState = continuousExecution ? MFS_CHECKED : MFS_UNCHECKED;

	SetMenuItemInfo(Blaze::editMenu, Blaze::MenuID::EditContinuousExecution, FALSE, &info);
};

static std::string utf16ToUTF8(const std::wstring& contents) {
	std::string narrowContents;
	int requiredChars = 0;
	int writtenChars = 0;

	if (!contents.empty()) {
		requiredChars = WideCharToMultiByte(CP_UTF8, 0, contents.c_str(), contents.size(), nullptr, 0, nullptr, nullptr);
		if (requiredChars == 0) {
			throw std::runtime_error("Invalid UTF-8 string");
		}

		narrowContents.resize(requiredChars);

		writtenChars = WideCharToMultiByte(CP_UTF8, 0, contents.c_str(), contents.size(), narrowContents.data(), requiredChars, nullptr, nullptr);

		narrowContents.resize(writtenChars);

		// trim off null characters
		while (!narrowContents.empty() && narrowContents[narrowContents.size() - 1] == '\0') {
			narrowContents.resize(narrowContents.size() - 1);
		}
	}

	return narrowContents;
};

static std::wstring utf8ToUTF16(const std::string& contents) {
	std::wstring wideContents;
	int requiredChars = 0;
	int writtenChars = 0;

	if (!contents.empty()) {
		requiredChars = MultiByteToWideChar(CP_UTF8, 0, contents.c_str(), (int)contents.size(), nullptr, 0);
		if (requiredChars == 0) {
			throw std::runtime_error("Invalid UTF-8 string");
		}

		wideContents.resize(requiredChars);

		writtenChars = MultiByteToWideChar(CP_UTF8, 0, contents.c_str(), (int)contents.size(), wideContents.data(), requiredChars);

		wideContents.resize(writtenChars);

		// trim off null characters
		while (!wideContents.empty() && wideContents[wideContents.size() - 1] == '\0') {
			wideContents.resize(wideContents.size() - 1);
		}
	}

	return wideContents;
};

static bool openROMDialog(std::string& outPath) {
	HRESULT hr;
	IFileDialog* fileDialog = nullptr;
	IShellItem* item = nullptr;
	LPWSTR filePath = nullptr;

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

	try {
		outPath = utf16ToUTF8(filePath);
	} catch (...) {
		CoTaskMemFree(filePath);
		item->Release();
		fileDialog->Release();
		CoUninitialize();
		std::rethrow_exception(std::current_exception());
	}

	return true;
};

static LPCSTR fontFace = nullptr;

static HWND win32DebuggerTextWindow = nullptr;
static HWND win32DebuggerRegWindow = nullptr;
static HWND win32DebugConsoleTextWindow = nullptr;
static HWND win32BreakpointAddressInput = nullptr;

static void updateBreakpoint(Blaze::Address address, bool shouldUpdateTextField) {
	Blaze::breakpoint = address;

	if (!shouldUpdateTextField) {
		return;
	}

	if (Blaze::breakpoint == UINT32_MAX) {
		Edit_SetText(win32BreakpointAddressInput, TEXT(""));
	} else {
		auto contents = Blaze::valueToHexString(address, 6);

#if defined(UNICODE)
		Edit_SetText(win32BreakpointAddressInput, utf8ToUTF16(contents).c_str());
#else
		Edit_SetText(win32BreakpointAddressInput, contents.c_str());
#endif
	}
};

static void updateDisassembly() {
	std::string contents;
	std::string regContents;
	std::vector<Blaze::CPU::DisassembledInstruction> disassembledInstructions;
	Blaze::Address PC;

	if (win32DebuggerTextWindow == nullptr || win32DebuggerRegWindow == nullptr) {
		return;
	}

	if (!Blaze::romLoaded) {
		contents = "No ROM loaded";
		regContents = "No ROM loaded";
	} else if (Blaze::continuousExecution) {
		contents = "Can't display disassembly while CPU is running";
		regContents = "Can't display registers while CPU is running";
	} else {
		PC = Blaze::concat24(Blaze::bus.cpu.PBR, Blaze::bus.cpu.PC);

		disassembledInstructions = Blaze::CPU::disassemble(Blaze::bus, PC, 10, Blaze::bus.cpu.memoryAndAccumulatorAre8Bit(), Blaze::bus.cpu.indexRegistersAre8Bit(), Blaze::bus.cpu.usingEmulationMode(), Blaze::bus.cpu.getFlag(Blaze::CPU::flags::c));

		if (disassembledInstructions.empty()) {
			contents = "Failed to disassemble memory at " + Blaze::valueToHexString(PC, 6, "$");
		} else {
			contents = "   ADDR  | CODE\n ------- | ----\n";
			for (const auto& disassembledInstruction: disassembledInstructions) {
				contents += " " + Blaze::valueToHexString(disassembledInstruction.address, 6, "$") + " | ";
				contents += disassembledInstruction.code + "\n";
			}

			// remove the final newline
			contents.erase(contents.end() - 1);
		}

		regContents = Blaze::bus.cpu.usingEmulationMode() ? "emulation mode\n" : "native mode\n";
		regContents += "P = ";

	#define DISASSEMBLY_P_CHECK(_name) \
		if ((Blaze::bus.cpu.P & Blaze::CPU::flags::_name) != 0) { \
			regContents += #_name; \
		} else { \
			regContents += '-'; \
		}

		DISASSEMBLY_P_CHECK(n);
		DISASSEMBLY_P_CHECK(v);
		DISASSEMBLY_P_CHECK(m);
		DISASSEMBLY_P_CHECK(x);
		DISASSEMBLY_P_CHECK(d);
		DISASSEMBLY_P_CHECK(i);
		DISASSEMBLY_P_CHECK(z);
		DISASSEMBLY_P_CHECK(c);

	#undef DISASSEMBLY_P_CHECK

		regContents += '\n';

		regContents += "PBR = " + Blaze::valueToHexString(Blaze::bus.cpu.PBR, 2, "$") + "   DBR = " + Blaze::valueToHexString(Blaze::bus.cpu.DBR, 2, "$") + "\n";
		regContents += "DR  = " + Blaze::valueToHexString(Blaze::bus.cpu.DR, 4, "$") + " SP  = " + Blaze::valueToHexString(Blaze::bus.cpu.SP, 4, "$") + "\n";
		regContents += "PC  = " + Blaze::valueToHexString(Blaze::bus.cpu.PC, 4, "$") + "\n";
		regContents += '\n';

		regContents += "A   = " + Blaze::valueToHexString(Blaze::bus.cpu.A.forceLoadFull(), 4, "$") + "\n";
		regContents += "X   = " + Blaze::valueToHexString(Blaze::bus.cpu.X.forceLoadFull(), 4, "$") + " Y   = " + Blaze::valueToHexString(Blaze::bus.cpu.Y.forceLoadFull(), 4, "$") + "\n";
	}

#if defined(UNICODE)
	Edit_SetText(win32DebuggerTextWindow, utf8ToUTF16(contents).c_str());
#else
	Edit_SetText(win32DebuggerTextWindow, contents.c_str());
#endif

#if defined(UNICODE)
	Edit_SetText(win32DebuggerRegWindow, utf8ToUTF16(regContents).c_str());
#else
	Edit_SetText(win32DebuggerRegWindow, regContents.c_str());
#endif
};

static WNDPROC originalEditWindowProc = nullptr;

static LRESULT CALLBACK breakpointAddressInputWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_CHAR) {
		// only allow hex digits and delete and backspace
		if (!(
			(wParam >= '0' && wParam <= '9') ||
			(wParam >= 'a' && wParam <= 'f') ||
			(wParam >= 'A' && wParam <= 'F') || 
			wParam == VK_DELETE ||
			wParam == VK_BACK
		)) {
			return 0;
		}
	}

	return CallWindowProc(originalEditWindowProc, hwnd, uMsg, wParam, lParam);
};

static LRESULT CALLBACK debuggerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND continueButton;
	static HWND pauseButton;
	static HWND nextButton;
	static HWND intoButton;

	switch (uMsg) {
		case WM_CLOSE:
			ShowWindow(hwnd, SW_HIDE);
			return 0;

		case WM_CREATE: {
			auto hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
			HFONT hFont = nullptr;

			hFont = CreateFont(0, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFace);

			win32DebuggerTextWindow = CreateWindowEx(0, TEXT("Edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY, 0, Blaze::debuggerButtonAreaHeight, 0, 0, hwnd, (HMENU)Blaze::MenuID::DebuggerTextView, hInst, nullptr);
			if (!win32DebuggerTextWindow) {
				abort();
			}

			SetWindowFont(win32DebuggerTextWindow, hFont, FALSE);

			win32DebuggerRegWindow = CreateWindowEx(0, TEXT("Edit"), nullptr, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hwnd, (HMENU)Blaze::MenuID::DebuggerRegisterView, hInst, nullptr);
			if (!win32DebuggerRegWindow) {
				abort();
			}

			SetWindowFont(win32DebuggerRegWindow, hFont, FALSE);

			win32BreakpointAddressInput = CreateWindowEx(0, TEXT("Edit"), nullptr, WS_CHILD | WS_VISIBLE | ES_LEFT, 0, 0, 0, 0, hwnd, (HMENU)Blaze::MenuID::DebuggerBreakpointAddressInput, hInst, nullptr);
			if (!win32BreakpointAddressInput) {
				abort();
			}

			originalEditWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(win32BreakpointAddressInput, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(breakpointAddressInputWindowProc)));

			SetWindowFont(win32BreakpointAddressInput, hFont, FALSE);

			// limit the breakpoint address input to 6 characters (for 6 address digits)
			PostMessage(win32BreakpointAddressInput, EM_SETLIMITTEXT, 6, 0);

			PostMessage(win32BreakpointAddressInput, EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(TEXT("Breakpoint address")));

			continueButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Continue"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, Blaze::debuggerButtonY, 0, Blaze::debuggerButtonHeight, hwnd, (HMENU)Blaze::MenuID::DebuggerContinue, hInst, nullptr);
			pauseButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Pause"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, Blaze::debuggerButtonY, 0, Blaze::debuggerButtonHeight, hwnd, (HMENU)Blaze::MenuID::DebuggerPause, hInst, nullptr);
			nextButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Next"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, Blaze::debuggerButtonY, 0, Blaze::debuggerButtonHeight, hwnd, (HMENU)Blaze::MenuID::DebuggerNext, hInst, nullptr);
			intoButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Step Into"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, Blaze::debuggerButtonY, 0, Blaze::debuggerButtonHeight, hwnd, (HMENU)Blaze::MenuID::DebuggerInto, hInst, nullptr);

			updateDisassembly();

			return 0;
		}

		case WM_SIZE: {
			auto width = LOWORD(lParam);
			auto height = HIWORD(lParam);

			auto buttonWidth = (std::max<decltype(width)>(width, Blaze::debuggerButtonXMargin * 5) - (Blaze::debuggerButtonXMargin * 5)) / 4;

			MoveWindow(win32DebuggerTextWindow, 0, Blaze::debuggerButtonAreaHeight, width, ((height - Blaze::debuggerButtonAreaHeight) - Blaze::debuggerRegisterViewHeight) - Blaze::debuggerBreakpointAddressInputHeight, TRUE);
			MoveWindow(win32DebuggerRegWindow, 0, (height - Blaze::debuggerRegisterViewHeight) - Blaze::debuggerBreakpointAddressInputHeight, width, Blaze::debuggerRegisterViewHeight, TRUE);
			MoveWindow(win32BreakpointAddressInput, 0, height - Blaze::debuggerBreakpointAddressInputHeight, width, Blaze::debuggerBreakpointAddressInputHeight, TRUE);
			MoveWindow(continueButton, Blaze::debuggerButtonXMargin * 1 + buttonWidth * 0, Blaze::debuggerButtonY, buttonWidth, Blaze::debuggerButtonHeight, TRUE);
			MoveWindow(pauseButton, Blaze::debuggerButtonXMargin * 2 + buttonWidth * 1, Blaze::debuggerButtonY, buttonWidth, Blaze::debuggerButtonHeight, TRUE);
			MoveWindow(nextButton, Blaze::debuggerButtonXMargin * 3  + buttonWidth * 2, Blaze::debuggerButtonY, buttonWidth, Blaze::debuggerButtonHeight, TRUE);
			MoveWindow(intoButton, Blaze::debuggerButtonXMargin * 4  + buttonWidth * 3, Blaze::debuggerButtonY, buttonWidth, Blaze::debuggerButtonHeight, TRUE);
			return 0;
		}

		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case Blaze::DebuggerContinue:
					if (HIWORD(wParam) == BN_CLICKED) {
						setContinuousExecution(true);
						updateDisassembly();
					}
					break;
				case Blaze::DebuggerPause:
					if (HIWORD(wParam) == BN_CLICKED) {
						setContinuousExecution(false);
						updateDisassembly();
					}
					break;
				case Blaze::DebuggerNext:
					if (HIWORD(wParam) == BN_CLICKED && !Blaze::continuousExecution) {
						Blaze::Address PC = Blaze::concat24(Blaze::bus.cpu.PBR, Blaze::bus.cpu.PC);
						Blaze::CPU::Instruction instrInfo = Blaze::CPU::decodeInstruction(Blaze::bus.read8(PC), Blaze::bus.cpu.memoryAndAccumulatorAre8Bit(), Blaze::bus.cpu.indexRegistersAre8Bit());
						if (instrInfo.opcode == Blaze::CPU::Opcode::JSR || instrInfo.opcode == Blaze::CPU::Opcode::JSL) {
							// these are subroutine execution instructions; clicking "next" is not supposed to go into them (that's what "step into" is for)
							//
							// instead, let's set a breakpoint and continue execution
							updateBreakpoint(PC + instrInfo.size);
							setContinuousExecution(true);
							updateDisassembly();
						} else {
							Blaze::bus.cpu.execute();
							updateDisassembly();
						}
					}
					break;
				case Blaze::DebuggerInto:
					if (HIWORD(wParam) == BN_CLICKED && !Blaze::continuousExecution) {
						Blaze::bus.cpu.execute();
						updateDisassembly();
					}
					break;

				case Blaze::DebuggerBreakpointAddressInput:
					if (HIWORD(wParam) == EN_CHANGE) {
#if defined(UNICODE)
						std::wstring origContents;
#else
						std::string origContents;
#endif

						origContents.resize(Edit_GetTextLength(win32BreakpointAddressInput) + 1);

						Edit_GetText(win32BreakpointAddressInput, origContents.data(), origContents.size());

						// discard the null terminator
						origContents.resize(origContents.size() - 1);

#if defined(UNICODE)
						auto contents = utf16ToUTF8(origContents);
#else
						auto& contents = origContents;
#endif

						if (contents.empty()) {
							updateBreakpoint(UINT32_MAX, false);
						} else {
							updateBreakpoint(std::stoul(contents, nullptr, 16), false);
						}
					}
					break;
			}

			return 0;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);

			return 0;
		}

		case WM_KEYDOWN: {
			if (wParam == VK_F5) {
				// F5 -> continue
				PostMessage(hwnd, WM_COMMAND, MAKELONG(Blaze::DebuggerContinue, BN_CLICKED), 0);
				return 0;
			} else if (wParam == VK_F6) {
				// F6 -> pause
				PostMessage(hwnd, WM_COMMAND, MAKELONG(Blaze::DebuggerPause, BN_CLICKED), 0);
				return 0;
			} else if (wParam == VK_F11) {
				// F11 -> step into
				PostMessage(hwnd, WM_COMMAND, MAKELONG(Blaze::DebuggerInto, BN_CLICKED), 0);
				return 0;
			} else {
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
		}

		case WM_SYSKEYDOWN: {
			if (wParam == VK_F10) {
				// F10 -> next
				PostMessage(hwnd, WM_COMMAND, MAKELONG(Blaze::DebuggerNext, BN_CLICKED), 0);
				return 0;
			} else {
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
		}

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
};

static void updateConsole(const std::string& contents) {
#if defined(UNICODE)
	Edit_SetText(win32DebugConsoleTextWindow, utf8ToUTF16(contents).c_str());
#else
	Edit_SetText(win32DebugConsoleTextWindow, contents.c_str());
#endif

	auto lineCount = Edit_GetLineCount(win32DebugConsoleTextWindow);
	SendMessage(win32DebugConsoleTextWindow, EM_LINESCROLL, 0, lineCount);
};

static LRESULT CALLBACK debugConsoleWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			ShowWindow(hwnd, SW_HIDE);
			return 0;

		case WM_CREATE: {
			auto hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
			HFONT hFont = nullptr;

			hFont = CreateFont(0, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFace);

			win32DebugConsoleTextWindow = CreateWindowEx(0, TEXT("Edit"), nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 0, 0, hwnd, (HMENU)Blaze::MenuID::DebugConsoleTextView, hInst, nullptr);
			if (!win32DebugConsoleTextWindow) {
				abort();
			}

			SetWindowFont(win32DebugConsoleTextWindow, hFont, FALSE);

			updateConsole("");

			return 0;
		}

		case WM_SIZE: {
			auto width = LOWORD(lParam);
			auto height = HIWORD(lParam);

			MoveWindow(win32DebugConsoleTextWindow, 0, 0, width, height, TRUE);
			return 0;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);

			return 0;
		}

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
};
#else // !_WIN32
static void updateBreakpoint(Blaze::Address address, bool shouldUpdateTextField) {
	#warning TODO
};

static void updateDisassembly() {
	#warning TODO
};

static void updateConsole(const std::string& contents) {
	#warning TODO
};

static void setContinuousExecution(bool continuousExecution) {
	Blaze::continuousExecution = continuousExecution;
	#warning TODO
};
#endif

static bool running = true;

static void cpuThreadMain(SDL_Window* window) {
	Blaze::Bus& bus = Blaze::bus;
	SDL_GLContext ourGLContext = nullptr;

	ourGLContext = SDL_GL_CreateContext(window);

	while (running) {
		if (Blaze::concat24(bus.cpu.PBR, bus.cpu.PC) == Blaze::breakpoint) {
			updateBreakpoint(UINT32_MAX);
			setContinuousExecution(false);
			updateDisassembly();
		}

		if (Blaze::romLoaded && Blaze::continuousExecution) {
			// execute a single instruction
			bus.cpu.execute();
		}
	}
};

static std::string debugConsoleOutput;
static std::mutex debugConsoleMutex;

void Blaze::clear() {
	std::unique_lock lock(debugConsoleMutex);
	debugConsoleOutput = "";
	updateConsole(debugConsoleOutput);
};

void Blaze::print(const std::string& subsystem, const std::string& message) {
	std::unique_lock lock(debugConsoleMutex);
	std::string copy = message;

	normalizeNewlines(copy);

	debugConsoleOutput += copy;
	updateConsole(debugConsoleOutput);
};

void Blaze::printLine(const std::string& subsystem, const std::string& message) {
	Blaze::print(subsystem, message + '\n');
};

int main(int argc, char** argv) {
	SDL_Window* mainWindow = nullptr;
	SDL_Event event;
	SDL_SysWMinfo mainWindowInfo;
	bool& romLoaded = Blaze::romLoaded;
	Blaze::Bus& bus = Blaze::bus;
	bool holdingLeftControl = false;
	bool holdingRightControl = false;
	bool holdingLeftShift = false;
	bool holdingRightShift = false;
	SDL_GLContext theGLContext = nullptr;
	std::thread cpuThread;
	Blaze::PPU ppu;
	Blaze::APU apu;

	bus.ppu = &ppu;
	bus.apu = &apu;

#ifdef _WIN32
	HWND win32MainWindow = nullptr;
	HWND win32DebuggerWindow = nullptr;
	HWND win32DebugConsoleWindow = nullptr;
	HMENU mainMenu = nullptr;
	HMENU fileMenu = nullptr;
	HMENU& editMenu = Blaze::editMenu;
	HMENU viewMenu = nullptr;
	HMENU helpMenu = nullptr;
#endif // _WIN32

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

#ifdef _WIN32
	DWORD fontFileAttrs = INVALID_FILE_ATTRIBUTES;

	fontFileAttrs = GetFileAttributes(TEXT("C:\\Windows\\Fonts\\FiraCode-Regular.ttf"));
	fontFace = TEXT("Fira Code");
#else
	#warning TODO
#endif
#ifdef _WIN32
	if (fontFileAttrs == INVALID_FILE_ATTRIBUTES || (fontFileAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		// try another font
		fontFileAttrs = GetFileAttributes(TEXT("C:\\Windows\\Fonts\\consola.ttf"));
		fontFace = TEXT("Consolas");
		if (fontFileAttrs == INVALID_FILE_ATTRIBUTES || (fontFileAttrs & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font");
			return 1;
		}
	}
#else
		#warning TODO
#endif

	mainWindow = SDL_CreateWindow("Blaze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Blaze::defaultWindowWidth, Blaze::defaultWindowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (mainWindow == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main window and renderer: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	theGLContext = SDL_GL_CreateContext(mainWindow);
	if (theGLContext == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create GL context: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_SetWindowTitle(mainWindow, Blaze::defaultWindowTitle);

	SDL_VERSION(&mainWindowInfo.version);
	if (!SDL_GetWindowWMInfo(mainWindow, &mainWindowInfo)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get window handle: %s", SDL_GetError());
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

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)fileMenu, "&File");

		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileOpen, "&Open ROM\tCtrl+O");
		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileClose, "&Close ROM\tCtrl+Shift+O");
		AppendMenu(fileMenu, MF_STRING, Blaze::MenuID::FileExit, "Exit");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)editMenu, "&Edit");

		AppendMenu(editMenu, MF_STRING, Blaze::MenuID::EditOptions, "&Options");
		AppendMenu(editMenu, MF_STRING, Blaze::MenuID::EditContinuousExecution, "Continuous E&xecution\tCtrl+Shift+X");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)viewMenu, "&View");

		AppendMenu(viewMenu, MF_STRING, Blaze::MenuID::ViewShowDebugger, "Show &Debugger\tCtrl+D");
		AppendMenu(viewMenu, MF_STRING, Blaze::MenuID::ViewShowDebugConsole, "Show Debug &Console\tCtrl+Shift+C");

		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)helpMenu, "&Help");

		AppendMenu(helpMenu, MF_STRING, Blaze::MenuID::HelpHelp, "&Help");

		SetMenu(win32MainWindow, mainMenu);
	}

	setContinuousExecution(true);

	// enable Win32 events in the SDL event loop
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	// set up the debugger window

	Blaze::debuggerWindowClass.lpfnWndProc = debuggerWindowProc;
	Blaze::debuggerWindowClass.hInstance = mainWindowInfo.info.win.hinstance;
	Blaze::debuggerWindowClass.lpszClassName = Blaze::debuggerWindowClassName;

	RegisterClass(&Blaze::debuggerWindowClass);

	win32DebuggerWindow = CreateWindowEx(0, Blaze::debuggerWindowClassName, TEXT("Debugger Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Blaze::defaultDebuggerWindowWidth, Blaze::defaultDebuggerWindowHeight, nullptr, nullptr, Blaze::debuggerWindowClass.hInstance, nullptr);
	if (win32DebuggerWindow == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create debugger window: %lu", GetLastError());
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
		return 1;
	}

	// set up the debug console window

	Blaze::debugConsoleWindowClass.lpfnWndProc = debugConsoleWindowProc;
	Blaze::debugConsoleWindowClass.hInstance = mainWindowInfo.info.win.hinstance;
	Blaze::debugConsoleWindowClass.lpszClassName = Blaze::debugConsoleWindowClassName;

	RegisterClass(&Blaze::debugConsoleWindowClass);

	win32DebugConsoleWindow = CreateWindowEx(0, Blaze::debugConsoleWindowClassName, TEXT("Debug Console"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Blaze::defaultDebugConsoleWindowWidth, Blaze::defaultDebugConsoleWindowHeight, nullptr, nullptr, Blaze::debugConsoleWindowClass.hInstance, nullptr);
	if (win32DebugConsoleWindow == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create debugger window: %lu", GetLastError());
		SDL_DestroyWindow(mainWindow);
		SDL_Quit();
		return 1;
	}
#endif // _WIN32

	bus.cpu.putCharacterHook = [&](char character) {
		Blaze::print("user-code", std::string(1, character));
	};

	bus.invalidAccess = [&](Blaze::Address address, Blaze::Byte bitSize, bool forWrite, Blaze::Address valueWhenWriting) {
		std::string output = "Invalid " + std::to_string(bitSize) + "-bit bus access to " + Blaze::valueToHexString(address, 6, "$") + " for ";
		if (forWrite) {
			output += "writing " + Blaze::valueToHexString(valueWhenWriting, 6, "$");
		} else {
			output += "reading";
		}
		Blaze::printLine("bus", output);
	};

	if (argc > 1) {
		std::string path = argv[1];
		std::stringstream output;
		bool romSuccessfullyLoaded = false;

		romLoaded = false;

		output << "Got ROM: " << path;
		output << '\n';

		try {
			bus.rom.load(path);

			if (bus.rom.type() == Blaze::ROM::Type::INVALID) {
				output << "Failed to load ROM";
			} else {
				output << "Loaded ROM with name: " << bus.rom.name();

				// when a ROM is loaded, we need to reset all components
				bus.reset();

				romSuccessfullyLoaded = true;
			}
		} catch (const std::runtime_error& e) {
			output << "Failed to load ROM:\n" << e.what();
		}

		Blaze::clear();
		Blaze::printLine("rom", output.str());

		romLoaded = romSuccessfullyLoaded;

		updateDisassembly();
	}

	// set up the GL context
	glClearColor(Blaze::defaultWindowColor.r, Blaze::defaultWindowColor.g, Blaze::defaultWindowColor.b, Blaze::defaultWindowColor.a);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW_MATRIX);
	glLoadIdentity();
	glViewport(0, 0, Blaze::defaultWindowWidth, Blaze::defaultWindowHeight);

	// create the CPU thread
	cpuThread = std::thread(cpuThreadMain, mainWindow);

	// main event loop
	while (running) {
		ppu.beginVBlank();

		// process all events for this frame
		while (SDL_PollEvent(&event)) {
			int snesKey;

			switch (event.type) {
				case SDL_QUIT:
					// exit if window closed
					running = false;
					break;

				case SDL_KEYDOWN: {
					bool holdingControl = false;
					bool holdingShift = false;

					switch (event.key.keysym.sym) {
						case SDLK_LCTRL: holdingLeftControl = true; break;
						case SDLK_RCTRL: holdingRightControl = true; break;
						case SDLK_LSHIFT: holdingLeftShift = true; break;
						case SDLK_RSHIFT: holdingRightShift = true; break;
					}

					snesKey = mapSDLToSNES(event.key.keysym.sym);
					holdingControl = holdingLeftControl || holdingRightControl;
					holdingShift = holdingLeftShift || holdingRightShift;

					if (holdingControl && !holdingShift && event.key.keysym.sym == SDLK_o) {
#if _WIN32
						PostMessage(win32MainWindow, WM_COMMAND, static_cast<WPARAM>(Blaze::MenuID::FileOpen), 0);
#else
						#warning TODO
#endif
					} else if (holdingControl && holdingShift && event.key.keysym.sym == SDLK_o) {
#if _WIN32
						PostMessage(win32MainWindow, WM_COMMAND, static_cast<WPARAM>(Blaze::MenuID::FileClose), 0);
#else
						#warning TODO
#endif
					} else if (holdingControl && holdingShift && event.key.keysym.sym == SDLK_x) {
#if _WIN32
						PostMessage(win32MainWindow, WM_COMMAND, static_cast<WPARAM>(Blaze::MenuID::EditContinuousExecution), 0);
#else
						#warning TODO
#endif
					} else if (holdingControl && !holdingShift && event.key.keysym.sym == SDLK_d) {
#if _WIN32
						PostMessage(win32MainWindow, WM_COMMAND, static_cast<WPARAM>(Blaze::MenuID::ViewShowDebugger), 0);
#else
						#warning TODO
#endif
					} else if (holdingControl && holdingShift && event.key.keysym.sym == SDLK_c) {
#if _WIN32
						PostMessage(win32MainWindow, WM_COMMAND, static_cast<WPARAM>(Blaze::MenuID::ViewShowDebugConsole), 0);
#else
						#warning TODO
#endif
					} else if (event.key.keysym.sym == SDLK_F5) {
						// F5 -> continue
#if _WIN32
						PostMessage(win32DebuggerWindow, WM_COMMAND, MAKELONG(Blaze::DebuggerContinue, BN_CLICKED), 0);
#else
						#warning TODO
#endif
					} else if (event.key.keysym.sym == SDLK_F6) {
						// F6 -> pause
#if _WIN32
						PostMessage(win32DebuggerWindow, WM_COMMAND, MAKELONG(Blaze::DebuggerPause, BN_CLICKED), 0);
#else
						#warning TODO
#endif
					} else if (event.key.keysym.sym == SDLK_F10) {
						// F10 -> next
#if _WIN32
						PostMessage(win32DebuggerWindow, WM_COMMAND, MAKELONG(Blaze::DebuggerNext, BN_CLICKED), 0);
#else
						#warning TODO
#endif
					} else if (event.key.keysym.sym == SDLK_F11) {
						// F11 -> step into
#if _WIN32
						PostMessage(win32DebuggerWindow, WM_COMMAND, MAKELONG(Blaze::DebuggerInto, BN_CLICKED), 0);
#else
						#warning TODO
#endif
					}

					// update emulator state
				} break;

				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_LCTRL: holdingLeftControl = false; break;
						case SDLK_RCTRL: holdingRightControl = false; break;
						case SDLK_LSHIFT: holdingLeftShift = false; break;
						case SDLK_RSHIFT: holdingRightShift = false; break;
					}
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
							bool romSuccessfullyLoaded = false;

							romLoaded = false;

							if (openROMDialog(path)) {
								output << "Got ROM: " << path;
								output << '\n';

								try {
									bus.rom.load(path);

									if (bus.rom.type() == Blaze::ROM::Type::INVALID) {
										output << "Failed to load ROM";
									} else {
										output << "Loaded ROM with name: " << bus.rom.name();

										// when a ROM is loaded, we need to reset all components
										bus.reset();

										romSuccessfullyLoaded = true;
									}
								} catch (const std::runtime_error& e) {
									output << "Failed to load ROM:\n" << e.what();
								}
							} else {
								output << "Failed to open ROM selection dialog";
							}

							Blaze::clear();
							Blaze::printLine("rom", output.str());

							romLoaded = romSuccessfullyLoaded;

							updateDisassembly();
						} break;

						case Blaze::MenuID::FileClose: {
							romLoaded = false;

							// when a ROM is unloaded, we need to reset all components
							bus.reset();
							bus.rom.reset(&bus); // we also reset the ROM

							updateDisassembly();

							Blaze::clear();
						} break;

						case Blaze::MenuID::FileExit: {
							running = false;
						} break;

						case Blaze::MenuID::EditOptions: {
							// TODO
						} break;

						case Blaze::MenuID::EditContinuousExecution: {
							setContinuousExecution(!Blaze::continuousExecution);
							updateDisassembly();
						} break;

						case Blaze::MenuID::ViewShowDebugger: {
							ShowWindow(win32DebuggerWindow, SW_SHOW);
						} break;

						case Blaze::MenuID::ViewShowDebugConsole: {
							ShowWindow(win32DebugConsoleWindow, SW_SHOW);
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

			case SDL_WINDOWEVENT: {
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					glViewport(0, 0, event.window.data1, event.window.data2);
				}
			} break;

			default:
				break;
			}
		}

		if (!running) {
			break;
		}

		// clear the display
		glClear(GL_COLOR_BUFFER_BIT);

		ppu.endVBlank();
		SDL_GL_SwapWindow(mainWindow);
	}

	cpuThread.join();

	SDL_GL_DeleteContext(theGLContext);
	SDL_DestroyWindow(mainWindow);

	SDL_Quit();

	return 0;
};
