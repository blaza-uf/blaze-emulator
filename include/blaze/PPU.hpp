#pragma once

#include <blaze/MMIO.hpp>
#include <blaze/util.hpp>
#include <blaze/color.hpp>

#include <mutex>
#include <array>
#include <functional>

// forward declarations
struct SDL_Renderer;
struct SDL_Surface;

namespace Blaze {
	struct Bus;

	static constexpr Address PPU_SPECIAL_OFFSET_NMITIMEN = 0xffff00;
	static constexpr Address PPU_SPECIAL_OFFSET_RDNMI = 0xffff80;

	// note that anything labeled "word address" means it's an address where each increment is a word (16 bits), not a byte.
	class PPU: public MMIODevice {
	public:
		enum class AddressRemapping: Byte {
			NoRemap = 0,
			_2bpp   = 1,
			_4bpp   = 2,
			_8bpp   = 3,
		};

		enum class AddressIncrementMode: bool {
			Low  = false,
			High = true,
		};

		enum class WindowMaskLogic: Byte {
			OR = 0,
			AND = 1,
			XOR = 2,
			XNOR = 3,
		};

		enum class SpriteSize: Byte {
			_8x8And16x16 = 0,
			_8x8And32x32 = 1,
			_8x8And64x64 = 2,
			_16x16And32x32 = 3,
			_16x16And64x64 = 4,
			_32x32And64x64 = 5,
			_16x32And32x64 = 6,
			_16x32And32x32 = 7,
		};

		static constexpr std::pair<Byte, Byte> spriteSizeForType(SpriteSize type, bool large) {
			switch (type) {
				case SpriteSize::_8x8And16x16:   return large ? std::make_pair<Byte, Byte>( 8,  8) : std::make_pair<Byte, Byte>(16, 16);
				case SpriteSize::_8x8And32x32:   return large ? std::make_pair<Byte, Byte>( 8,  8) : std::make_pair<Byte, Byte>(32, 32);
				case SpriteSize::_8x8And64x64:   return large ? std::make_pair<Byte, Byte>( 8,  8) : std::make_pair<Byte, Byte>(64, 64);
				case SpriteSize::_16x16And32x32: return large ? std::make_pair<Byte, Byte>(16, 16) : std::make_pair<Byte, Byte>(32, 32);
				case SpriteSize::_16x16And64x64: return large ? std::make_pair<Byte, Byte>(16, 16) : std::make_pair<Byte, Byte>(64, 64);
				case SpriteSize::_32x32And64x64: return large ? std::make_pair<Byte, Byte>(32, 32) : std::make_pair<Byte, Byte>(64, 64);
				case SpriteSize::_16x32And32x64: return large ? std::make_pair<Byte, Byte>(16, 32) : std::make_pair<Byte, Byte>(32, 64);
				case SpriteSize::_16x32And32x32: return large ? std::make_pair<Byte, Byte>(16, 32) : std::make_pair<Byte, Byte>(32, 32);
				default:
					return std::make_pair<Byte, Byte>(0, 0);
			}
		};

		enum class ColorRegion: Byte {
			NoRegions = 0,
			OutsideWindow = 1,
			InsideWindow = 2,
			AllRegions = 3,
		};

		enum class ColorAddened: bool {
			FixedColor = false,
			Subscreen = true,
		};

		enum class TileFormat: Byte {
			INVALID = 0,
			_2bpp,
			_4bpp,
			_8bpp,
			_8bppDirect,
			Mode7,
			Mode7Direct,
		};

		static constexpr Byte tileFormatWordSize(const TileFormat format) {
			switch (format) {
				case TileFormat::_2bpp:       return  8;
				case TileFormat::_4bpp:       return 16;
				case TileFormat::_8bpp:
				case TileFormat::_8bppDirect: return 32;
				case TileFormat::Mode7:
				case TileFormat::Mode7Direct: return 64;
				default:                      return  0;
			}
		};

		static constexpr Byte tileFormatPixelBitPlanes(const TileFormat format) {
			switch (format) {
				case TileFormat::_2bpp:       return 2;
				case TileFormat::_4bpp:       return 4;
				case TileFormat::_8bpp:
				case TileFormat::_8bppDirect: return 8;
				default:                      return 0;
			}
		};

		struct Sprite {
			int16_t x;
			Byte y;
			Byte tileIndex;
			Byte paletteGroup;
			Byte priority;
			bool large: 1;
			bool secondTilePage: 1;
			bool flipVertically: 1;
			bool flipHorizontally: 1;
		};

		struct TilemapEntry {
			Word tileIndex;
			Byte paletteGroup;
			bool highPriority: 1;
			bool flipVertically: 1;
			bool flipHorizontally: 1;
		};

	private:
		struct Background {
			Byte tilemapAddressAndSize = 0;
			Byte nba = 0;
			Word horizontalScroll = 0;
			Word verticalScroll = 0;
			WindowMaskLogic windowMaskLogic = WindowMaskLogic::OR;
			bool doubleCharSize: 1;
			bool enableMosaic: 1;
			bool enableWindow1: 1;
			bool invertWindow1: 1;
			bool enableWindow2: 1;
			bool invertWindow2: 1;
			bool enableOnMainScreen: 1;
			bool enableOnSubscreen: 1;
			bool enableWindowsOnMainScreen: 1;
			bool enableWindowsOnSubscreen: 1;
			bool enableColorMath: 1;

			inline Word tilemapWordAddress() const {
				return static_cast<Word>((tilemapAddressAndSize >> 2) & 0x3f) << 10;
			};

			inline Byte tilemapHorizontalCount() const {
				return (tilemapAddressAndSize & (1 << 0)) != 0 ? 2 : 1;
			};

			inline Byte tilemapVerticalCount() const {
				return (tilemapAddressAndSize & (1 << 1)) != 0 ? 2 : 1;
			};

			inline Word chrBaseWordAddress() const {
				return static_cast<Word>(nba & 0x0f) << 12;
			};

			void reset();
			void render(SDL_Renderer* renderer, const Word* vram, const Word* cgram, bool highPriority, Byte backgroundIndex, const PPU& ppu);
		};

		Bus* _bus = nullptr;

		Byte _inidisp = 0;
		Byte _objsel = 0;
		Word _oamByteAddress = 0;
		Byte _oamLatch = 0;
		Byte _bgmode = 0;
		Byte _mosaic = 0;
		Byte _vmain = 0;
		Word _vramWordAddress = 0;
		Word _vramLatch = 0;
		Byte _cgramWordAddress = 0;
		Byte _cgramLatch = 0;
		Byte _window1Left = 0;
		Byte _window1Right = 0;
		Byte _window2Left = 0;
		Byte _window2Right = 0;
		WindowMaskLogic _spriteWindowMaskLogic = WindowMaskLogic::OR;
		WindowMaskLogic _colorWindowMaskLogic = WindowMaskLogic::OR;
		Byte _cgwsel = 0;
		Byte _fixedBlue = 0;
		Byte _fixedGreen = 0;
		Byte _fixedRed = 0;
		Byte _setini = 0;
		Byte _bgScrollLatch = 0;
		Byte _bgHorizontalScrollLatch = 0;
		Byte _nmitimen = 0;
		Byte _rdnmi = 0;
		bool _cgramHighByte: 1;
		bool _enableSpriteWindow1: 1;
		bool _invertSpriteWindow1: 1;
		bool _enableSpriteWindow2: 1;
		bool _invertSpriteWindow2: 1;
		bool _enableColorWindow1: 1;
		bool _invertColorWindow1: 1;
		bool _enableColorWindow2: 1;
		bool _invertColorWindow2: 1;
		bool _enableSpriteOnMainScreen: 1;
		bool _enableSpriteOnSubscreen: 1;
		bool _enableSpriteWindowsOnMainScreen: 1;
		bool _enableSpriteWindowsOnSubscreen: 1;
		bool _enableSpriteColorMath: 1;
		bool _colorMathMinus: 1;
		bool _halfColorMath: 1;
		bool _enableBackdropColorMath: 1;
		bool _oamPriorityRotation: 1;

		std::array<Background, 4> _backgrounds;
		std::array<Byte, 544> _oamData;
		std::array<Word, 256> _cgram;
		std::array<Word, 32 * 1024> _vram;

		std::mutex _rdnmiMutex;

		SDL_Renderer* _renderer = nullptr;
		SDL_Renderer* _backbufferRenderer = nullptr;

		mutable std::mutex _renderTextureMutex;
		bool _swapped = true;
		SDL_Surface* _renderSurface;
		SDL_Surface* _renderBackbuffer;

		void renderSpriteLayer(Byte priority);

	public:
		inline Byte addressIncrementAmountInWords() const {
			switch (_vmain & 3) {
				case 0: return 1;
				case 1: return 32;
				case 2: [[fallthrough]];
				case 3: return 128;
				default: unreachable();
			}
		};

		inline AddressRemapping addressRemapping() const {
			return static_cast<AddressRemapping>((_vmain >> 2) & 3);
		};

		inline AddressIncrementMode addressIncrementMode() const {
			return (_vmain & (1 << 7)) != 0 ? AddressIncrementMode::High : AddressIncrementMode::Low;
		};

		inline bool screenInterlacing() const {
			return (_setini & (1 << 0)) != 0;
		};

		inline bool spriteInterlacing() const {
			return (_setini & (1 << 1)) != 0;
		};

		inline bool highResolutionMode() const {
			return (_setini & (1 << 3)) != 0;
		};

		inline bool mode7SecondLayerEffect() const {
			return (_setini & (1 << 6)) != 0;
		};

		inline bool externalSync() const {
			return (_setini & (1 << 7)) != 0;
		};

		inline bool forcedBlanking() const {
			return (_inidisp & (1 << 7)) != 0;
		};

		inline Byte screenBrightness() const {
			return _inidisp & 0x0f;
		};

		inline Word nameBaseWordAddress() const {
			return static_cast<Word>(_objsel & 7) << 13;
		};

		inline Word nameSelectWordOffset() const {
			return (static_cast<Word>((_objsel >> 3) & 3) + 1) << 12;
		};

		inline SpriteSize spriteSize() const {
			return static_cast<SpriteSize>((_objsel >> 5) & 7);
		};

		inline bool mode1HighPriorityBackground3() const {
			return (_bgmode & (1 << 3)) != 0;
		};

		inline Byte backgroundMode() const {
			return _bgmode & 7;
		};

		inline Byte mosaicPixelSize() const {
			return ((_mosaic >> 4) & 0x0f) + 1;
		};

		inline ColorRegion mainScreenColorBlackRegion() const {
			return static_cast<ColorRegion>((_cgwsel >> 6) & 3);
		};

		inline ColorRegion subscreenColorTransparentRegion() const {
			return static_cast<ColorRegion>((_cgwsel >> 4) & 3);
		};

		inline ColorAddened colorAddened() const {
			return (_cgwsel & (1 << 1)) != 0 ? ColorAddened::Subscreen : ColorAddened::FixedColor;
		};

		inline bool enableDirectColor() const {
			return (_cgwsel & (1 << 0)) != 0;
		};

		PPU();
		~PPU() override;

		// the PPU cannot be moved or copied
		PPU(const PPU&) = delete;
		PPU(PPU&) = delete;
		PPU& operator=(PPU&&) = delete;
		PPU& operator=(const PPU&) = delete;

		Byte registerSize(Address offset, Byte attemptedAccessSize) override;
		Address read(Address offset, Byte bitSize) override;
		void write(Address offset, Byte bitSize, Address value) override;

		void reset(Bus* bus) override;

		void beginVBlank();
		void endVBlank();

		inline bool overscan() const {
			return (_setini & (1 << 2)) != 0;
		};

		inline void renderSurface(std::function<void(SDL_Surface* surface)> renderer) {
			std::unique_lock lock(_renderTextureMutex);
			if (_swapped) {
				_swapped = false;
				renderer(_renderSurface);
			}
		};

		static std::vector<Byte> readTile(const Word* vram, Word vramWordAddress, Byte width, Byte height, TileFormat format);
		static Color readColor(const Word* cgram, Byte index);
		static Sprite readSprite(const Byte* oam, Byte index);
		static TilemapEntry readTilemapEntry(const Word* vram, Word tilemapBaseWordAddress, Byte x, Byte y);
	};
};
