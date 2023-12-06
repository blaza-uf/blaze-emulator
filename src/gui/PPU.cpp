#include <blaze/PPU.hpp>
#include <blaze/Bus.hpp>
#include <SDL.h>
#include <cassert>
#include <blaze/debug.hpp>

namespace Blaze {
	static constexpr int snesWidth = 256;
	static constexpr int snesHeight = 224;
};

struct PPUMMIORegister {
	enum IgnoreMe: Blaze::Address {
		INIDISP     = 0x00,
		OBJSEL      = 0x01,
		OAMADDL     = 0x02,
		OAMADDH     = 0x03,
		OAMDATA     = 0x04,
		BGMODE      = 0x05,
		MOSAIC      = 0x06,
		BG1SC       = 0x07,
		BG2SC       = 0x08,
		BG3SC       = 0x09,
		BG4SC       = 0x0A,
		BG12NBA     = 0x0B,
		BG34NBA     = 0x0C,
		BG1HOFS     = 0x0D,
		M7HOFS      = 0x0D,
		BG1VOFS     = 0x0E,
		M7VOFS      = 0x0E,
		BG2HOFS     = 0x0F,
		BG2VOFS     = 0x10,
		BG3HOFS     = 0x11,
		BG3VOFS     = 0x12,
		BG4HOFS     = 0x13,
		BG4VOFS     = 0x14,
		VMAIN       = 0x15,
		VMADDL      = 0x16,
		VMADDH      = 0x17,
		VMDATAL     = 0x18,
		VMDATAH     = 0x19,
		M7SEL       = 0x1A,
		M7A         = 0x1B,
		M7B         = 0x1C,
		M7C         = 0x1D,
		M7D         = 0x1E,
		M7X         = 0x1F,
		M7Y         = 0x20,
		CGADD       = 0x21,
		CGDATA      = 0x22,
		W12SEL      = 0x23,
		W34SEL      = 0x24,
		WOBJSEL     = 0x25,
		WH0         = 0x26,
		WH1         = 0x27,
		WH2         = 0x28,
		WH3         = 0x29,
		WBGLOG      = 0x2A,
		WOBJLOG     = 0x2B,
		TM          = 0x2C,
		TS          = 0x2D,
		TMW         = 0x2E,
		TSW         = 0x2F,
		CGWSEL      = 0x30,
		CGADSUB     = 0x31,
		COLDATA     = 0x32,
		SETINI      = 0x33,
		MPYL        = 0x34,
		MPYM        = 0x35,
		MPYH        = 0x36,
		SLHV        = 0x37,
		OAMDATAREAD = 0x38,
		VMDATALREAD = 0x39,
		VMDATAHREAD = 0x3A,
		CGDATAREAD  = 0x3B,
		OPHCT       = 0x3C,
		OPVCT       = 0x3D,
		STAT77      = 0x3E,
		STAT78      = 0x3F,
	};
};

void Blaze::PPU::Background::reset() {
	tilemapAddressAndSize = 0;
	nba = 0;
	horizontalScroll = 0;
	verticalScroll = 0;
	windowMaskLogic = WindowMaskLogic::OR;

	doubleCharSize = false;
	enableMosaic = false;
	enableWindow1 = false;
	invertWindow1 = false;
	enableWindow2 = false;
	invertWindow2 = false;
	enableOnMainScreen = false;
	enableOnSubscreen = false;
	enableWindowsOnMainScreen = false;
	enableWindowsOnSubscreen = false;
	enableColorMath = false;
};

Blaze::Byte Blaze::PPU::registerSize(Address offset, Byte attemptedAccessSize) {
	return 8;
};

Blaze::Address Blaze::PPU::read(Address offset, Byte bitSize) {
	switch (offset) {
		case PPUMMIORegister::INIDISP: return _inidisp;
		case PPUMMIORegister::OBJSEL:  return _objsel;
		case PPUMMIORegister::OAMADDL: return (_oamByteAddress >> 1) & 0xff;
		case PPUMMIORegister::OAMADDH: return ((_oamByteAddress >> 9) & 1) | (_oamPriorityRotation ? (1 << 7) : 0);
		case PPUMMIORegister::BGMODE:  return _bgmode;
		case PPUMMIORegister::MOSAIC:  return _mosaic;
		case PPUMMIORegister::BG1SC:   return _backgrounds[0].tilemapAddressAndSize;
		case PPUMMIORegister::BG2SC:   return _backgrounds[1].tilemapAddressAndSize;
		case PPUMMIORegister::BG3SC:   return _backgrounds[2].tilemapAddressAndSize;
		case PPUMMIORegister::BG4SC:   return _backgrounds[3].tilemapAddressAndSize;
		case PPUMMIORegister::BG12NBA: return _backgrounds[0].nba | (_backgrounds[1].nba << 4);
		case PPUMMIORegister::BG34NBA: return _backgrounds[2].nba | (_backgrounds[3].nba << 4);
		case PPUMMIORegister::VMAIN:   return _vmain;
		case PPUMMIORegister::VMADDL:  return _vramWordAddress & 0xff;
		case PPUMMIORegister::VMADDH:  return (_vramWordAddress >> 8) & 0xff;
		case PPUMMIORegister::CGADD:   return _cgramWordAddress;
		case PPUMMIORegister::W12SEL:  return
			getBit<Byte>(0, _backgrounds[0].invertWindow1) |
			getBit<Byte>(1, _backgrounds[0].enableWindow1) |
			getBit<Byte>(2, _backgrounds[0].invertWindow2) |
			getBit<Byte>(3, _backgrounds[0].enableWindow2) |
			getBit<Byte>(4, _backgrounds[1].invertWindow1) |
			getBit<Byte>(5, _backgrounds[1].enableWindow1) |
			getBit<Byte>(6, _backgrounds[1].invertWindow2) |
			getBit<Byte>(7, _backgrounds[1].enableWindow2) ;
		case PPUMMIORegister::W34SEL:  return
			getBit<Byte>(0, _backgrounds[2].invertWindow1) |
			getBit<Byte>(1, _backgrounds[2].enableWindow1) |
			getBit<Byte>(2, _backgrounds[2].invertWindow2) |
			getBit<Byte>(3, _backgrounds[2].enableWindow2) |
			getBit<Byte>(4, _backgrounds[3].invertWindow1) |
			getBit<Byte>(5, _backgrounds[3].enableWindow1) |
			getBit<Byte>(6, _backgrounds[3].invertWindow2) |
			getBit<Byte>(7, _backgrounds[3].enableWindow2) ;
		case PPUMMIORegister::WOBJSEL:  return
			getBit<Byte>(0, _invertSpriteWindow1) |
			getBit<Byte>(1, _enableSpriteWindow1) |
			getBit<Byte>(2, _invertSpriteWindow2) |
			getBit<Byte>(3, _enableSpriteWindow2) |
			getBit<Byte>(4, _invertColorWindow1)  |
			getBit<Byte>(5, _enableColorWindow1)  |
			getBit<Byte>(6, _invertColorWindow2)  |
			getBit<Byte>(7, _enableColorWindow2)  ;
		case PPUMMIORegister::WH0:      return _window1Left;
		case PPUMMIORegister::WH1:      return _window1Right;
		case PPUMMIORegister::WH2:      return _window2Left;
		case PPUMMIORegister::WH3:      return _window2Right;
		case PPUMMIORegister::WBGLOG:   return
			(static_cast<Byte>(_backgrounds[0].windowMaskLogic) << 0) |
			(static_cast<Byte>(_backgrounds[1].windowMaskLogic) << 2) |
			(static_cast<Byte>(_backgrounds[2].windowMaskLogic) << 4) |
			(static_cast<Byte>(_backgrounds[3].windowMaskLogic) << 6) ;
		case PPUMMIORegister::WOBJLOG:  return (static_cast<Byte>(_spriteWindowMaskLogic) << 0) | (static_cast<Byte>(_colorWindowMaskLogic) << 2);
		case PPUMMIORegister::TM:       return
			getBit<Byte>(0, _backgrounds[0].enableOnMainScreen) |
			getBit<Byte>(1, _backgrounds[1].enableOnMainScreen) |
			getBit<Byte>(2, _backgrounds[2].enableOnMainScreen) |
			getBit<Byte>(3, _backgrounds[3].enableOnMainScreen) |
			getBit<Byte>(4, _enableSpriteOnMainScreen)          ;
		case PPUMMIORegister::TS:       return
			getBit<Byte>(0, _backgrounds[0].enableOnSubscreen) |
			getBit<Byte>(1, _backgrounds[1].enableOnSubscreen) |
			getBit<Byte>(2, _backgrounds[2].enableOnSubscreen) |
			getBit<Byte>(3, _backgrounds[3].enableOnSubscreen) |
			getBit<Byte>(4, _enableSpriteOnSubscreen)          ;
		case PPUMMIORegister::TMW:      return
			getBit<Byte>(0, _backgrounds[0].enableWindowsOnMainScreen) |
			getBit<Byte>(1, _backgrounds[1].enableWindowsOnMainScreen) |
			getBit<Byte>(2, _backgrounds[2].enableWindowsOnMainScreen) |
			getBit<Byte>(3, _backgrounds[3].enableWindowsOnMainScreen) |
			getBit<Byte>(4, _enableSpriteWindowsOnMainScreen)          ;
		case PPUMMIORegister::TSW:      return
			getBit<Byte>(0, _backgrounds[0].enableWindowsOnSubscreen) |
			getBit<Byte>(1, _backgrounds[1].enableWindowsOnSubscreen) |
			getBit<Byte>(2, _backgrounds[2].enableWindowsOnSubscreen) |
			getBit<Byte>(3, _backgrounds[3].enableWindowsOnSubscreen) |
			getBit<Byte>(4, _enableSpriteWindowsOnSubscreen)          ;
		case PPUMMIORegister::CGWSEL:   return _cgwsel;
		case PPUMMIORegister::CGADSUB:  return
			getBit<Byte>(0, _backgrounds[0].enableColorMath) |
			getBit<Byte>(1, _backgrounds[1].enableColorMath) |
			getBit<Byte>(2, _backgrounds[2].enableColorMath) |
			getBit<Byte>(3, _backgrounds[3].enableColorMath) |
			getBit<Byte>(4, _enableSpriteColorMath)          |
			getBit<Byte>(5, _enableBackdropColorMath)        |
			getBit<Byte>(6, _halfColorMath)                  |
			getBit<Byte>(7, _colorMathMinus)                 ;
		case PPUMMIORegister::SETINI:   return _setini;

		case PPUMMIORegister::OAMDATAREAD: {
			auto val = _oamData[_oamByteAddress];
			_oamByteAddress = (_oamByteAddress + 1) % _oamData.size();
			return val;
		} break;

		case PPUMMIORegister::VMDATALREAD: {
			auto val = lo8(_vramLatch);
			if (addressIncrementMode() == AddressIncrementMode::Low) {
				_vramLatch = _vram[_vramWordAddress];
				_vramWordAddress = (_vramWordAddress + 1) & _vram.size();
			}
			return val;
		} break;

		case PPUMMIORegister::VMDATAHREAD: {
			auto val = hi8(_vramLatch, true);
			if (addressIncrementMode() == AddressIncrementMode::High) {
				_vramLatch = _vram[_vramWordAddress];
				_vramWordAddress = (_vramWordAddress + 1) & _vram.size();
			}
			return val;
		} break;

		case PPUMMIORegister::CGDATAREAD: {
			auto val = _cgram[_cgramWordAddress];
			if (_cgramHighByte) {
				_cgramHighByte = false;
				_cgramWordAddress = (_cgramWordAddress + 1) % _cgram.size();
				return hi8(val, true);
			} else {
				_cgramHighByte = true;
				return lo8(val);
			}
		} break;

		case PPU_SPECIAL_OFFSET_NMITIMEN:
			return _nmitimen;
		case PPU_SPECIAL_OFFSET_RDNMI: {
			std::unique_lock lock(_rdnmiMutex);
			auto val = _rdnmi;
			// clear the vblank flag
			_rdnmi &= ~(1 << 7);
			return val;
		} break;

		default:
			return 0;
	}
};

void Blaze::PPU::write(Address offset, Byte bitSize, Address value) {
	switch (offset) {
		case PPUMMIORegister::INIDISP:
			_inidisp = value;
			break;
		case PPUMMIORegister::OBJSEL:
			_objsel = value;
			break;
		case PPUMMIORegister::OAMADDL:
			_oamByteAddress = (_oamByteAddress & (1 << 9)) | (value << 1);
			break;
		case PPUMMIORegister::OAMADDH:
			_oamByteAddress = (_oamByteAddress & ~(1 << 9)) | ((value & 1) << 9);
			_oamPriorityRotation = (value & (1 << 7)) != 0;
			break;
		case PPUMMIORegister::OAMDATA: {
			if ((_oamByteAddress & 1) == 0) {
				_oamLatch = value;
			}

			if (_oamByteAddress < 0x200 && (_oamByteAddress & 1) != 0) {
				_oamData[_oamByteAddress - 1] = _oamLatch;
				_oamData[_oamByteAddress] = value;
			}

			if (_oamByteAddress >= 0x200) {
				_oamData[_oamByteAddress] = value;
			}

			_oamByteAddress = (_oamByteAddress + 1) % _oamData.size();
		} break;
		case PPUMMIORegister::BGMODE:
			_bgmode = value;
			break;
		case PPUMMIORegister::MOSAIC:
			_mosaic = value;
			break;
		case PPUMMIORegister::BG1SC:
			_backgrounds[0].tilemapAddressAndSize = value;
			break;
		case PPUMMIORegister::BG2SC:
			_backgrounds[1].tilemapAddressAndSize = value;
			break;
		case PPUMMIORegister::BG3SC:
			_backgrounds[2].tilemapAddressAndSize = value;
			break;
		case PPUMMIORegister::BG4SC:
			_backgrounds[3].tilemapAddressAndSize = value;
			break;
		case PPUMMIORegister::BG12NBA:
			_backgrounds[0].nba = value & 0x0f;
			_backgrounds[1].nba = (value >> 4) & 0x0f;
			break;
		case PPUMMIORegister::BG34NBA:
			_backgrounds[2].nba = value & 0x0f;
			_backgrounds[3].nba = (value >> 4) & 0x0f;
			break;
		case PPUMMIORegister::BG1HOFS:
			_backgrounds[0].horizontalScroll = (value << 8) | (_bgScrollLatch & ~7) |(_bgHorizontalScrollLatch & 7);
			_bgScrollLatch = value;
			_bgHorizontalScrollLatch = value;
			break;
		case PPUMMIORegister::BG1VOFS:
			_backgrounds[0].verticalScroll = (value << 8) | (_bgScrollLatch);
			_bgScrollLatch = value;
			break;
		case PPUMMIORegister::BG2HOFS:
			_backgrounds[1].horizontalScroll = (value << 8) | (_bgScrollLatch & ~7) |(_bgHorizontalScrollLatch & 7);
			_bgScrollLatch = value;
			_bgHorizontalScrollLatch = value;
			break;
		case PPUMMIORegister::BG2VOFS:
			_backgrounds[1].verticalScroll = (value << 8) | (_bgScrollLatch);
			_bgScrollLatch = value;
			break;
		case PPUMMIORegister::BG3HOFS:
			_backgrounds[2].horizontalScroll = (value << 8) | (_bgScrollLatch & ~7) |(_bgHorizontalScrollLatch & 7);
			_bgScrollLatch = value;
			_bgHorizontalScrollLatch = value;
			break;
		case PPUMMIORegister::BG3VOFS:
			_backgrounds[2].verticalScroll = (value << 8) | (_bgScrollLatch);
			_bgScrollLatch = value;
			break;
		case PPUMMIORegister::BG4HOFS:
			_backgrounds[3].horizontalScroll = (value << 8) | (_bgScrollLatch & ~7) |(_bgHorizontalScrollLatch & 7);
			_bgScrollLatch = value;
			_bgHorizontalScrollLatch = value;
			break;
		case PPUMMIORegister::BG4VOFS:
			_backgrounds[3].verticalScroll = (value << 8) | (_bgScrollLatch);
			_bgScrollLatch = value;
			break;
		case PPUMMIORegister::VMAIN:
			_vmain = value;
			break;
		case PPUMMIORegister::VMADDL:
			_vramWordAddress = hi8(_vramWordAddress, false) | lo8(value);
			_vramLatch = _vram[_vramWordAddress];
			break;
		case PPUMMIORegister::VMADDH:
			_vramWordAddress = lo8(_vramWordAddress) | (value << 8);
			_vramLatch = _vram[_vramWordAddress];
			break;
		case PPUMMIORegister::VMDATAL:
			_vram[_vramWordAddress] = hi8(_vram[_vramWordAddress], false) | lo8(value);
			if (addressIncrementMode() == AddressIncrementMode::Low) {
				_vramWordAddress = (_vramWordAddress + 1) % _vram.size();
			}
			break;
		case PPUMMIORegister::VMDATAH:
			_vram[_vramWordAddress] = lo8(_vram[_vramWordAddress]) | (value << 8);
			if (addressIncrementMode() == AddressIncrementMode::High) {
				_vramWordAddress = (_vramWordAddress + 1) % _vram.size();
			}
			break;
		case PPUMMIORegister::CGADD:
			_cgramWordAddress = value;
			_cgramHighByte = false;
			break;
		case PPUMMIORegister::CGDATA:
			if (_cgramHighByte) {
				_cgramHighByte = false;
				_cgram[_cgramWordAddress] = (value << 8) | _cgramLatch;
				_cgramWordAddress = (_cgramWordAddress + 1) % _cgram.size();
			} else {
				_cgramHighByte = true;
				_cgramLatch = value;
			}
			break;
		case PPUMMIORegister::W12SEL:
			_backgrounds[0].invertWindow1 = testBit(value, 0);
			_backgrounds[0].enableWindow1 = testBit(value, 1);
			_backgrounds[0].invertWindow2 = testBit(value, 2);
			_backgrounds[0].enableWindow2 = testBit(value, 3);
			_backgrounds[1].invertWindow1 = testBit(value, 4);
			_backgrounds[1].enableWindow1 = testBit(value, 5);
			_backgrounds[1].invertWindow2 = testBit(value, 6);
			_backgrounds[1].enableWindow2 = testBit(value, 7);
			break;
		case PPUMMIORegister::W34SEL:
			_backgrounds[2].invertWindow1 = testBit(value, 0);
			_backgrounds[2].enableWindow1 = testBit(value, 1);
			_backgrounds[2].invertWindow2 = testBit(value, 2);
			_backgrounds[2].enableWindow2 = testBit(value, 3);
			_backgrounds[3].invertWindow1 = testBit(value, 4);
			_backgrounds[3].enableWindow1 = testBit(value, 5);
			_backgrounds[3].invertWindow2 = testBit(value, 6);
			_backgrounds[3].enableWindow2 = testBit(value, 7);
			break;
		case PPUMMIORegister::WOBJSEL:
			_invertSpriteWindow1 = testBit(value, 0);
			_enableSpriteWindow1 = testBit(value, 1);
			_invertSpriteWindow2 = testBit(value, 2);
			_enableSpriteWindow2 = testBit(value, 3);
			_invertColorWindow1  = testBit(value, 4);
			_enableColorWindow1  = testBit(value, 5);
			_invertColorWindow2  = testBit(value, 6);
			_enableColorWindow2  = testBit(value, 7);
			break;
		case PPUMMIORegister::WH0:
			_window1Left = value;
			break;
		case PPUMMIORegister::WH1:
			_window1Right = value;
			break;
		case PPUMMIORegister::WH2:
			_window2Left = value;
			break;
		case PPUMMIORegister::WH3:
			_window2Right = value;
			break;
		case PPUMMIORegister::WBGLOG:
			_backgrounds[0].windowMaskLogic = static_cast<WindowMaskLogic>((value >> 0) & 3);
			_backgrounds[1].windowMaskLogic = static_cast<WindowMaskLogic>((value >> 2) & 3);
			_backgrounds[2].windowMaskLogic = static_cast<WindowMaskLogic>((value >> 4) & 3);
			_backgrounds[3].windowMaskLogic = static_cast<WindowMaskLogic>((value >> 6) & 3);
			break;
		case PPUMMIORegister::WOBJLOG:
			_spriteWindowMaskLogic = static_cast<WindowMaskLogic>((value >> 0) & 3);
			_colorWindowMaskLogic  = static_cast<WindowMaskLogic>((value >> 2) & 3);
			break;
		case PPUMMIORegister::TM:
			_backgrounds[0].enableOnMainScreen = testBit(value, 0);
			_backgrounds[1].enableOnMainScreen = testBit(value, 1);
			_backgrounds[2].enableOnMainScreen = testBit(value, 2);
			_backgrounds[3].enableOnMainScreen = testBit(value, 3);
			_enableSpriteOnMainScreen          = testBit(value, 4);
			break;
		case PPUMMIORegister::TS:
			_backgrounds[0].enableOnSubscreen = testBit(value, 0);
			_backgrounds[1].enableOnSubscreen = testBit(value, 1);
			_backgrounds[2].enableOnSubscreen = testBit(value, 2);
			_backgrounds[3].enableOnSubscreen = testBit(value, 3);
			_enableSpriteOnSubscreen          = testBit(value, 4);
			break;
		case PPUMMIORegister::TMW:
			_backgrounds[0].enableWindowsOnMainScreen = testBit(value, 0);
			_backgrounds[1].enableWindowsOnMainScreen = testBit(value, 1);
			_backgrounds[2].enableWindowsOnMainScreen = testBit(value, 2);
			_backgrounds[3].enableWindowsOnMainScreen = testBit(value, 3);
			_enableSpriteWindowsOnMainScreen          = testBit(value, 4);
			break;
		case PPUMMIORegister::TSW:
			_backgrounds[0].enableWindowsOnSubscreen = testBit(value, 0);
			_backgrounds[1].enableWindowsOnSubscreen = testBit(value, 1);
			_backgrounds[2].enableWindowsOnSubscreen = testBit(value, 2);
			_backgrounds[3].enableWindowsOnSubscreen = testBit(value, 3);
			_enableSpriteWindowsOnSubscreen          = testBit(value, 4);
			break;
		case PPUMMIORegister::CGWSEL:
			_cgwsel = value;
			break;
		case PPUMMIORegister::CGADSUB:
			_backgrounds[0].enableColorMath = testBit(value, 0);
			_backgrounds[1].enableColorMath = testBit(value, 1);
			_backgrounds[2].enableColorMath = testBit(value, 2);
			_backgrounds[3].enableColorMath = testBit(value, 3);
			_enableSpriteColorMath          = testBit(value, 4);
			_enableBackdropColorMath        = testBit(value, 5);
			_halfColorMath                  = testBit(value, 6);
			_colorMathMinus                 = testBit(value, 7);
			break;
		case PPUMMIORegister::COLDATA: {
			Byte colorVal = value & 0x1f;
			if (testBit(value, 5)) {
				_fixedRed = colorVal;
			}
			if (testBit(value, 6)) {
				_fixedGreen = colorVal;
			}
			if (testBit(value, 7)) {
				_fixedBlue = colorVal;
			}
		} break;
		case PPUMMIORegister::SETINI:
			_setini = value;
			break;

		case PPU_SPECIAL_OFFSET_NMITIMEN: {
			auto prev = _nmitimen;

			_nmitimen = value;

			{
				std::unique_lock lock(_rdnmiMutex);
				if ((prev & (1 << 7)) == 0 && (_nmitimen & (1 << 7)) != 0) {
					_bus->cpu.nmi();
				}
			}
		} break;
	}
};

void Blaze::PPU::reset(Bus* bus) {
	_bus = bus;

	_inidisp = 0;
	_objsel = 0;
	_oamByteAddress = 0;
	_oamLatch = 0;
	_bgmode = 0;
	_mosaic = 0;
	_vmain = 0;
	_vramWordAddress = 0;
	_vramLatch = 0;
	_cgramWordAddress = 0;
	_cgramLatch = 0;
	_window1Left = 0;
	_window1Right = 0;
	_window2Left = 0;
	_window2Right = 0;
	_spriteWindowMaskLogic = WindowMaskLogic::OR;
	_colorWindowMaskLogic = WindowMaskLogic::OR;
	_cgwsel = 0;
	_fixedBlue = 0;
	_fixedGreen = 0;
	_fixedRed = 0;
	_setini = 0;
	_bgScrollLatch = 0;
	_bgHorizontalScrollLatch = 0;
	_nmitimen = 0;
	_rdnmi = 0;

	_cgramHighByte = false;
	_enableSpriteWindow1 = false;
	_invertSpriteWindow1 = false;
	_enableSpriteWindow2 = false;
	_invertSpriteWindow2 = false;
	_enableColorWindow1 = false;
	_invertColorWindow1 = false;
	_enableColorWindow2 = false;
	_invertColorWindow2 = false;
	_enableSpriteOnMainScreen = false;
	_enableSpriteOnSubscreen = false;
	_enableSpriteWindowsOnMainScreen = false;
	_enableSpriteWindowsOnSubscreen = false;
	_enableSpriteColorMath = false;
	_colorMathMinus = false;
	_halfColorMath = false;
	_enableBackdropColorMath = false;
	_oamPriorityRotation = false;

	for (auto& background: _backgrounds) {
		background.reset();
	}
};

// note that we actually render in the opposite order of beginning and ending vblank:
// we render to the backbuffer after ending vblank, and we swap the backbuffer with the
// frontbuffer when beginning vblank. this matches the SNES PPU behavior: the vblank
// period is used for the *software* to set up what will be rendered and then the period
// *outside* vblank is used by the PPU to perform the rendering.

void Blaze::PPU::beginVBlank() {
	// swap the render texture with the backbuffer
	{
		std::unique_lock lock(_renderTextureMutex);
		SDL_RenderPresent(_backbufferRenderer);

		auto tmp = _renderer;
		_renderer = _backbufferRenderer;
		_backbufferRenderer = tmp;

		auto tmp2 = _renderSurface;
		_renderSurface = _renderBackbuffer;
		_renderBackbuffer = tmp2;

		_swapped = true;
	}

	{
		std::unique_lock lock(_rdnmiMutex);
		_rdnmi |= (1 << 7);

		if ((_nmitimen & (1 << 7)) != 0) {
			_bus->cpu.nmi();
		}
	}
};

void Blaze::PPU::endVBlank() {
	{
		std::unique_lock lock(_rdnmiMutex);
		_rdnmi &= ~(1 << 7);
	}

	// begin rendering

	// the backdrop is always index 0 in CGRAM
	auto backdrop = readColor(_cgram.data(), 0);

	// clear the frame with the backdrop color
	SDL_SetRenderDrawColor(_backbufferRenderer, backdrop.r, backdrop.g, backdrop.b, backdrop.a);
	SDL_RenderClear(_backbufferRenderer);

	// note that we render from back to front

	//Blaze::printLine("ppu", "Ended vblank; rendering in mode " + std::to_string(backgroundMode()));

	switch (backgroundMode()) {
		case 0:
		case 1:
			if (backgroundMode() == 0) {
				_backgrounds[3].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 3, *this);
			}

			_backgrounds[2].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 2, *this);
			renderSpriteLayer(0);

			if (backgroundMode() == 0) {
				_backgrounds[3].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 3, *this);
			}

			if (backgroundMode() != 1 || !mode1HighPriorityBackground3()) {
				_backgrounds[2].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 2, *this);
			}

			renderSpriteLayer(1);
			_backgrounds[1].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 1, *this);
			_backgrounds[0].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 0, *this);
			renderSpriteLayer(2);
			_backgrounds[1].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 1, *this);
			_backgrounds[0].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 0, *this);
			renderSpriteLayer(3);

			// render special BG3 high priority in mode 1 above all else
			if (backgroundMode() == 1 && mode1HighPriorityBackground3()) {
				_backgrounds[2].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 2, *this);
			}
			break;

		default:
			if (backgroundMode() != 6) {
				_backgrounds[1].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 1, *this);
			}

			renderSpriteLayer(0);
			_backgrounds[0].render(_backbufferRenderer, _vram.data(), _cgram.data(), false, 0, *this);
			renderSpriteLayer(1);

			if (backgroundMode() != 6) {
				_backgrounds[1].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 1, *this);
			}

			renderSpriteLayer(2);

			if (backgroundMode() != 7) {
				_backgrounds[0].render(_backbufferRenderer, _vram.data(), _cgram.data(), true, 0, *this);
			}

			renderSpriteLayer(3);
			break;
	}
};

Blaze::PPU::PPU() {
	_cgram.fill(0);
	_vram.fill(0);
	_oamData.fill(0);

	_renderSurface = SDL_CreateRGBSurfaceWithFormat(0, Blaze::snesWidth, Blaze::snesHeight, 32, SDL_PIXELFORMAT_RGBA8888);
	if (_renderSurface == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create PPU render surface: %s", SDL_GetError());
	}

	_renderBackbuffer = SDL_CreateRGBSurfaceWithFormat(0, Blaze::snesWidth, Blaze::snesHeight, 32, SDL_PIXELFORMAT_RGBA8888);
	if (_renderBackbuffer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create PPU render backbuffer surface: %s", SDL_GetError());
	}

	_renderer = SDL_CreateSoftwareRenderer(_renderSurface);
	if (_renderer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create PPU renderer: %s", SDL_GetError());
	}

	_backbufferRenderer = SDL_CreateSoftwareRenderer(_renderBackbuffer);
	if (_backbufferRenderer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create PPU backbuffer renderer: %s", SDL_GetError());
	}
};

Blaze::PPU::~PPU() {
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyRenderer(_backbufferRenderer);
	SDL_FreeSurface(_renderSurface);
	SDL_FreeSurface(_renderBackbuffer);
};

std::vector<Blaze::Byte> Blaze::PPU::readTile(const Word* vram, Word vramWordAddress, Byte width, Byte height, TileFormat format) {
	std::vector<Byte> result;
	size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
	auto basicTileWordSize = tileFormatWordSize(format);
	auto bitPlanes = tileFormatPixelBitPlanes(format);

	for (size_t pixel = 0; pixel < pixels; ++pixel) {
		Byte row = pixel / width;
		Byte column = pixel % width;
		size_t baseWordOffset = 0;

		baseWordOffset = static_cast<size_t>(basicTileWordSize) * ((static_cast<size_t>(16) * (row / 8)) + (column / 8));
		row %= 8;
		column %= 8;

		baseWordOffset += row;

		Byte pixelMask = 1 << (7 - column);

		switch (format) {
			case TileFormat::Mode7:
			case TileFormat::Mode7Direct:
				break;

			default: {
				Byte pixelValue = 0;

				for (Byte bitPlane = 0; bitPlane < bitPlanes; ++bitPlane) {
					Byte bitPlaneWordIndex = bitPlane % 2;
					size_t bitPlaneWordOffset = bitPlane / 2;
					auto bitPlaneWord = vram[vramWordAddress + baseWordOffset + (bitPlaneWordOffset * 8)];
					Byte bitPlaneByte = bitPlaneWord >> (bitPlaneWordIndex * 8);
					Byte bit = ((bitPlaneByte & pixelMask) != 0) ? 1 : 0;
					pixelValue |= bit << bitPlane;
				}

				result.push_back(pixelValue);
			} break;
		}
	}

	return result;
};

// these are basically magic values that have been adapted from SDL.
// the idea is to try to map the range of 5-bit values onto the full range of 8-bit values
// and distribute them more-or-less evenly.
static constexpr std::array<Blaze::Byte, 32> RANGE_CONVERSION_5_TO_8_BIT = {
	0,
	8,
	16,
	24,
	32,
	41,
	49,
	57,
	65,
	74,
	82,
	90,
	98,
	106,
	115,
	123,
	131,
	139,
	148,
	156,
	164,
	172,
	180,
	189,
	197,
	205,
	213,
	222,
	230,
	238,
	246,
	255,
};

Blaze::Color Blaze::PPU::readColor(const Word* cgram, Byte index) {
	auto value = cgram[index];
	Byte red   = (value >>  0) & 0x1f;
	Byte green = (value >>  5) & 0x1f;
	Byte blue  = (value >> 10) & 0x1f;

	return Blaze::Color(RANGE_CONVERSION_5_TO_8_BIT[red], RANGE_CONVERSION_5_TO_8_BIT[green], RANGE_CONVERSION_5_TO_8_BIT[blue]);
};

Blaze::PPU::Sprite Blaze::PPU::readSprite(const Byte* oam, Byte index) {
	Sprite result;
	size_t offset = static_cast<size_t>(index) * 4;
	size_t finalOffset = 512 + (index / 4);
	Byte finalBits = (oam[finalOffset] >> ((index % 4) * 2)) & 3;
	Byte configBits = oam[offset + 3];

	result.x = static_cast<int16_t>(oam[offset]) + static_cast<int16_t>(((finalBits & (1 << 0)) != 0) ? -256 : 0);
	result.y = oam[offset + 1];
	result.tileIndex = oam[offset + 2];
	result.secondTilePage = (configBits & (1 << 0)) != 0;
	result.paletteGroup = (configBits >> 1) & 7;
	result.priority = (configBits >> 4) & 3;
	result.flipHorizontally = (configBits & (1 << 6)) != 0;
	result.flipVertically = (configBits & (1 << 7)) != 0;
	result.large = (finalBits & (1 << 1)) != 0;

	return result;
};

Blaze::PPU::TilemapEntry Blaze::PPU::readTilemapEntry(const Word* vram, Word tilemapBaseWordAddress, Byte x, Byte y) {
	Blaze::PPU::TilemapEntry result;

	assert((tilemapBaseWordAddress & 0x3ff) == 0);

	Word index = static_cast<Word>(x) * static_cast<Word>(y);
	auto data = vram[tilemapBaseWordAddress + index];

	result.tileIndex = data & 0x3ff;
	result.paletteGroup = (data >> 10) & 7;
	result.highPriority = (data & (1 << 13)) != 0;
	result.flipHorizontally = (data & (1 << 14)) != 0;
	result.flipVertically = (data & (1 << 15)) != 0;

	return result;
};

void Blaze::PPU::Background::render(SDL_Renderer* renderer, const Word* vram, const Word* cgram, bool highPriority, Byte backgroundIndex, const PPU& ppu) {
	// TODO: subscreen

	if (!enableOnMainScreen) {
		return;
	}

	//Blaze::printLine("ppu", "Rendering BG" + std::to_string(backgroundIndex + 1) + " layer");

	std::vector<Color> palette;
	size_t subpaletteSize = 0;
	size_t subpaletteCount = 0;
	TileFormat tileFormat = TileFormat::INVALID;

	switch (ppu.backgroundMode()) {
		case 0: {
			subpaletteSize = 4;
			subpaletteCount = 8;
			tileFormat = TileFormat::_2bpp;
			for (size_t colorIndex = 0; colorIndex < subpaletteCount * subpaletteSize; ++colorIndex) {
				palette.push_back(PPU::readColor(cgram, (subpaletteCount * subpaletteSize * backgroundIndex) + colorIndex));
			}
		} break;

		case 1: {
			if (backgroundIndex == 2) {
				// BG3 uses 2bpp
				subpaletteSize = 4;
				subpaletteCount = 8;
				tileFormat = TileFormat::_2bpp;
				for (size_t colorIndex = 0; colorIndex < subpaletteCount * subpaletteSize; ++colorIndex) {
					palette.push_back(PPU::readColor(cgram, colorIndex));
				}
			}
		} [[fallthrough]];
		case 2: {
			if (backgroundIndex == 0 || backgroundIndex == 1) {
				subpaletteSize = 16;
				subpaletteCount = 8;
				tileFormat = TileFormat::_4bpp;
				for (size_t colorIndex = 0; colorIndex < subpaletteCount * subpaletteSize; ++colorIndex) {
					palette.push_back(PPU::readColor(cgram, colorIndex));
				}
			}
		} break;

		default:
			// TODO
			break;
	}

	assert(palette.size() == (subpaletteSize * subpaletteCount));

	for (size_t vertical = 0; vertical < tilemapVerticalCount(); ++vertical) {
		for (size_t horizontal = 0; horizontal < tilemapHorizontalCount(); ++horizontal) {
			constexpr size_t TILEMAP_WORDS = 32 * 32;
			size_t tilemapOffset = ((vertical * tilemapHorizontalCount()) + horizontal) * TILEMAP_WORDS;
			for (size_t tilemapY = 0; tilemapY < 32; ++tilemapY) {
				for (size_t tilemapX = 0; tilemapX < 32; ++tilemapX) {
					size_t dimensions = doubleCharSize ? 16 : 8;
					int absoluteX = static_cast<int>(((horizontal * 32) + tilemapX) * dimensions);
					int absoluteY = static_cast<int>(((vertical * 32) + tilemapY) * dimensions);

					absoluteX -= horizontalScroll;
					absoluteY -= verticalScroll;

					int absoluteEndX = absoluteX + static_cast<int>(dimensions);
					int absoluteEndY = absoluteY + static_cast<int>(dimensions);

					if (
						absoluteX > Blaze::snesWidth ||
						absoluteEndX < 0 ||
						absoluteY > Blaze::snesHeight ||
						absoluteEndY < 0
					) {
						// this tile isn't visible at all
						//Blaze::printLine("ppu", "invisible tile");
						continue;
					}

					auto entry = PPU::readTilemapEntry(vram, tilemapWordAddress() + tilemapOffset, tilemapX, tilemapY);
					const Color* subpalette = &palette[entry.paletteGroup * subpaletteSize];

					assert((palette.size() - (entry.paletteGroup * subpaletteSize)) >= subpaletteSize);

					auto tile = PPU::readTile(vram, chrBaseWordAddress() + (entry.tileIndex * tileFormatWordSize(tileFormat)), doubleCharSize ? 16 : 8, doubleCharSize ? 16 : 8, tileFormat);
					std::vector<Byte> pixels;

					pixels.reserve(tile.size() * 4);

					for (Byte subpaletteIndex: tile) {
						assert(subpaletteIndex < subpaletteSize);
						if (subpaletteIndex == 0) {
							pixels.push_back(0);
							pixels.push_back(0);
							pixels.push_back(0);
							pixels.push_back(0);
						} else {
							const auto& color = subpalette[subpaletteIndex];
							pixels.push_back(color.r);
							pixels.push_back(color.g);
							pixels.push_back(color.b);
							pixels.push_back(color.a);
						}
					}

					SDL_Texture* tileTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, dimensions, dimensions);
					SDL_Rect fullSrc {
						0, 0,
						static_cast<int>(dimensions),
						static_cast<int>(dimensions),
					};

					SDL_UpdateTexture(tileTexture, &fullSrc, pixels.data(), 4 * dimensions);

					auto leftMinus = (absoluteX < 0) ? (0 - absoluteX) : 0;
					auto topMinus = (absoluteY < 0) ? (0 - absoluteY) : 0;
					auto rightMinus = (absoluteEndX > Blaze::snesWidth) ? (absoluteEndX - Blaze::snesWidth) : 0;
					auto bottomMinus = (absoluteEndY > Blaze::snesHeight) ? (absoluteEndY - Blaze::snesHeight) : 0;

					auto renderedWidth = (static_cast<int>(dimensions) - rightMinus) - leftMinus;
					auto renderedHeight = (static_cast<int>(dimensions) - bottomMinus) - topMinus;

					SDL_Rect src {
						leftMinus,
						topMinus,
						renderedWidth,
						renderedHeight,
					};

					SDL_Rect dest {
						(absoluteX < 0) ? 0 : absoluteX,
						(absoluteY < 0) ? 0 : absoluteY,
						renderedWidth,
						renderedHeight,
					};

					SDL_RenderCopyEx(renderer, tileTexture, &src, &dest, 0, nullptr, static_cast<SDL_RendererFlip>((entry.flipHorizontally ? SDL_FLIP_HORIZONTAL : 0) | (entry.flipVertically ? SDL_FLIP_VERTICAL : 0)));

					SDL_DestroyTexture(tileTexture);

					//Blaze::printLine("ppu", "rendered visible tile from " + std::to_string(chrBaseWordAddress() + (entry.tileIndex * tileFormatWordSize(tileFormat))) + " at " + std::to_string(leftMinus) + ", " + std::to_string(topMinus) + " of " + std::to_string(renderedWidth) + ", " + std::to_string(renderedHeight));
				}
			}
		}
	}
};

void Blaze::PPU::renderSpriteLayer(Byte priority) {
	// TODO: subscreen

	if (!_enableSpriteOnMainScreen) {
		return;
	}

	//Blaze::printLine("ppu", "Rendering S" + std::to_string(priority) + " layer");

	Word firstPage = nameBaseWordAddress();
	Word secondPage = firstPage + nameSelectWordOffset();

	// again, we render from back to front
	// (so that the last sprite rendered lies on top)

	std::array<std::array<Color, 16>, 8> palette;

	for (size_t subpaletteIndex = 0; subpaletteIndex < palette.size(); ++subpaletteIndex) {
		auto& subpalette = palette[subpaletteIndex];
		for (size_t colorIndex = 0; colorIndex < subpalette.size(); ++colorIndex) {
			auto& color = subpalette[colorIndex];
			color = readColor(_cgram.data(), 0x80 + (subpaletteIndex * 16) + colorIndex);
		}
	}

	for (Byte spriteIndexPlusOne = 128; spriteIndexPlusOne > 0; --spriteIndexPlusOne) {
		Byte spriteIndex = spriteIndexPlusOne - 1;
		auto sprite = readSprite(_oamData.data(), spriteIndex);

		if (sprite.priority != priority) {
			continue;
		}

		const auto& subpalette = palette[sprite.paletteGroup];
		auto page = sprite.secondTilePage ? secondPage : firstPage;
		auto [width, height] = spriteSizeForType(spriteSize(), sprite.large);

		auto tile = PPU::readTile(_vram.data(), page + (sprite.tileIndex * tileFormatWordSize(TileFormat::_4bpp)), width, height, TileFormat::_4bpp);
		std::vector<Byte> pixels;

		pixels.reserve(tile.size() * 4);

		for (Byte subpaletteIndex: tile) {
			if (subpaletteIndex == 0) {
				pixels.push_back(0);
				pixels.push_back(0);
				pixels.push_back(0);
				pixels.push_back(0);
			} else {
				const auto& color = subpalette[subpaletteIndex];
				pixels.push_back(color.r);
				pixels.push_back(color.g);
				pixels.push_back(color.b);
				pixels.push_back(color.a);
			}
		}

		SDL_Texture* spriteTexture = SDL_CreateTexture(_backbufferRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width, height);
		SDL_Rect fullSrc {
			0, 0,
			static_cast<int>(width),
			static_cast<int>(height),
		};

		SDL_UpdateTexture(spriteTexture, &fullSrc, pixels.data(), 4 * width);

		int absoluteX = sprite.x;
		int absoluteY = sprite.y;

		int absoluteEndX = absoluteX + static_cast<int>(width);
		int absoluteEndY = absoluteY + static_cast<int>(height);

		if (
			absoluteX > Blaze::snesWidth ||
			absoluteEndX < 0 ||
			absoluteY > Blaze::snesHeight ||
			absoluteEndY < 0
		) {
			// this sprite isn't visible at all
			//Blaze::printLine("ppu", "invisible sprite");
			continue;
		}

		auto leftMinus = (absoluteX < 0) ? (0 - absoluteX) : 0;
		auto topMinus = (absoluteY < 0) ? (0 - absoluteY) : 0;
		auto rightMinus = (absoluteEndX > Blaze::snesWidth) ? (absoluteEndX - Blaze::snesWidth) : 0;
		auto bottomMinus = (absoluteEndY > Blaze::snesHeight) ? (absoluteEndY - Blaze::snesHeight) : 0;

		auto renderedWidth = (static_cast<int>(width) - rightMinus) - leftMinus;
		auto renderedHeight = (static_cast<int>(height) - bottomMinus) - topMinus;

		SDL_Rect src {
			leftMinus,
			topMinus,
			renderedWidth,
			renderedHeight,
		};

		SDL_Rect dest {
			(absoluteX < 0) ? 0 : absoluteX,
			(absoluteY < 0) ? 0 : absoluteY,
			renderedWidth,
			renderedHeight,
		};

		SDL_RenderCopyEx(_backbufferRenderer, spriteTexture, &src, &dest, 0, nullptr, static_cast<SDL_RendererFlip>((sprite.flipHorizontally ? SDL_FLIP_HORIZONTAL : 0) | (sprite.flipVertically ? SDL_FLIP_VERTICAL : 0)));

		SDL_DestroyTexture(spriteTexture);

		//Blaze::printLine("ppu", "rendered visible sprite from " + std::to_string(page + (sprite.tileIndex * tileFormatWordSize(TileFormat::_4bpp))) + " at " + std::to_string(leftMinus) + ", " + std::to_string(topMinus) + " of " + std::to_string(renderedWidth) + ", " + std::to_string(renderedHeight) + " (" + std::to_string(width) + ", " + std::to_string(height) + ")");
	}
};
