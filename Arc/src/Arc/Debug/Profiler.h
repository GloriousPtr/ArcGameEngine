#pragma once

#include <optick.h>

namespace ArcEngine::Profile
{
	struct Color
	{
		enum
		{
			Null							= static_cast<int>(0x00000000),
			AliceBlue						= static_cast<int>(0xFFF0F8FF),
			AntiqueWhite					= static_cast<int>(0xFFFAEBD7),
			Aqua							= static_cast<int>(0xFF00FFFF),
			Aquamarine						= static_cast<int>(0xFF7FFFD4),
			Azure							= static_cast<int>(0xFFF0FFFF),
			Beige							= static_cast<int>(0xFFF5F5DC),
			Bisque							= static_cast<int>(0xFFFFE4C4),
			Black							= static_cast<int>(0xFF000000),
			BlanchedAlmond					= static_cast<int>(0xFFFFEBCD),
			Blue							= static_cast<int>(0xFF0000FF),
			BlueViolet						= static_cast<int>(0xFF8A2BE2),
			Brown							= static_cast<int>(0xFFA52A2A),
			BurlyWood						= static_cast<int>(0xFFDEB887),
			CadetBlue						= static_cast<int>(0xFF5F9EA0),
			Chartreuse						= static_cast<int>(0xFF7FFF00),
			Chocolate						= static_cast<int>(0xFFD2691E),
			Coral							= static_cast<int>(0xFFFF7F50),
			CornflowerBlue					= static_cast<int>(0xFF6495ED),
			Cornsilk						= static_cast<int>(0xFFFFF8DC),
			Crimson							= static_cast<int>(0xFFDC143C),
			Cyan							= static_cast<int>(0xFF00FFFF),
			DarkBlue						= static_cast<int>(0xFF00008B),
			DarkCyan						= static_cast<int>(0xFF008B8B),
			DarkGoldenRod					= static_cast<int>(0xFFB8860B),
			DarkGray						= static_cast<int>(0xFFA9A9A9),
			DarkGreen						= static_cast<int>(0xFF006400),
			DarkKhaki						= static_cast<int>(0xFFBDB76B),
			DarkMagenta						= static_cast<int>(0xFF8B008B),
			DarkOliveGreen					= static_cast<int>(0xFF556B2F),
			DarkOrange						= static_cast<int>(0xFFFF8C00),
			DarkOrchid						= static_cast<int>(0xFF9932CC),
			DarkRed							= static_cast<int>(0xFF8B0000),
			DarkSalmon						= static_cast<int>(0xFFE9967A),
			DarkSeaGreen					= static_cast<int>(0xFF8FBC8F),
			DarkSlateBlue					= static_cast<int>(0xFF483D8B),
			DarkSlateGray					= static_cast<int>(0xFF2F4F4F),
			DarkTurquoise					= static_cast<int>(0xFF00CED1),
			DarkViolet						= static_cast<int>(0xFF9400D3),
			DeepPink						= static_cast<int>(0xFFFF1493),
			DeepSkyBlue						= static_cast<int>(0xFF00BFFF),
			DimGray							= static_cast<int>(0xFF696969),
			DodgerBlue						= static_cast<int>(0xFF1E90FF),
			FireBrick						= static_cast<int>(0xFFB22222),
			FloralWhite						= static_cast<int>(0xFFFFFAF0),
			ForestGreen						= static_cast<int>(0xFF228B22),
			Fuchsia							= static_cast<int>(0xFFFF00FF),
			Gainsboro						= static_cast<int>(0xFFDCDCDC),
			GhostWhite						= static_cast<int>(0xFFF8F8FF),
			Gold							= static_cast<int>(0xFFFFD700),
			GoldenRod						= static_cast<int>(0xFFDAA520),
			Gray							= static_cast<int>(0xFF808080),
			Green							= static_cast<int>(0xFF008000),
			GreenYellow						= static_cast<int>(0xFFADFF2F),
			HoneyDew						= static_cast<int>(0xFFF0FFF0),
			HotPink							= static_cast<int>(0xFFFF69B4),
			IndianRed						= static_cast<int>(0xFFCD5C5C),
			Indigo							= static_cast<int>(0xFF4B0082),
			Ivory							= static_cast<int>(0xFFFFFFF0),
			Khaki							= static_cast<int>(0xFFF0E68C),
			Lavender						= static_cast<int>(0xFFE6E6FA),
			LavenderBlush					= static_cast<int>(0xFFFFF0F5),
			LawnGreen						= static_cast<int>(0xFF7CFC00),
			LemonChiffon					= static_cast<int>(0xFFFFFACD),
			LightBlue						= static_cast<int>(0xFFADD8E6),
			LightCoral						= static_cast<int>(0xFFF08080),
			LightCyan						= static_cast<int>(0xFFE0FFFF),
			LightGoldenRodYellow			= static_cast<int>(0xFFFAFAD2),
			LightGray						= static_cast<int>(0xFFD3D3D3),
			LightGreen						= static_cast<int>(0xFF90EE90),
			LightPink						= static_cast<int>(0xFFFFB6C1),
			LightSalmon						= static_cast<int>(0xFFFFA07A),
			LightSeaGreen					= static_cast<int>(0xFF20B2AA),
			LightSkyBlue					= static_cast<int>(0xFF87CEFA),
			LightSlateGray					= static_cast<int>(0xFF778899),
			LightSteelBlue					= static_cast<int>(0xFFB0C4DE),
			LightYellow						= static_cast<int>(0xFFFFFFE0),
			Lime							= static_cast<int>(0xFF00FF00),
			LimeGreen						= static_cast<int>(0xFF32CD32),
			Linen							= static_cast<int>(0xFFFAF0E6),
			Magenta							= static_cast<int>(0xFFFF00FF),
			Maroon							= static_cast<int>(0xFF800000),
			MediumAquaMarine				= static_cast<int>(0xFF66CDAA),
			MediumBlue						= static_cast<int>(0xFF0000CD),
			MediumOrchid					= static_cast<int>(0xFFBA55D3),
			MediumPurple					= static_cast<int>(0xFF9370DB),
			MediumSeaGreen					= static_cast<int>(0xFF3CB371),
			MediumSlateBlue					= static_cast<int>(0xFF7B68EE),
			MediumSpringGreen				= static_cast<int>(0xFF00FA9A),
			MediumTurquoise					= static_cast<int>(0xFF48D1CC),
			MediumVioletRed					= static_cast<int>(0xFFC71585),
			MidnightBlue					= static_cast<int>(0xFF191970),
			MintCream						= static_cast<int>(0xFFF5FFFA),
			MistyRose						= static_cast<int>(0xFFFFE4E1),
			Moccasin						= static_cast<int>(0xFFFFE4B5),
			NavajoWhite						= static_cast<int>(0xFFFFDEAD),
			Navy							= static_cast<int>(0xFF000080),
			OldLace							= static_cast<int>(0xFFFDF5E6),
			Olive							= static_cast<int>(0xFF808000),
			OliveDrab						= static_cast<int>(0xFF6B8E23),
			Orange							= static_cast<int>(0xFFFFA500),
			OrangeRed						= static_cast<int>(0xFFFF4500),
			Orchid							= static_cast<int>(0xFFDA70D6),
			PaleGoldenRod					= static_cast<int>(0xFFEEE8AA),
			PaleGreen						= static_cast<int>(0xFF98FB98),
			PaleTurquoise					= static_cast<int>(0xFFAFEEEE),
			PaleVioletRed					= static_cast<int>(0xFFDB7093),
			PapayaWhip						= static_cast<int>(0xFFFFEFD5),
			PeachPuff						= static_cast<int>(0xFFFFDAB9),
			Peru							= static_cast<int>(0xFFCD853F),
			Pink							= static_cast<int>(0xFFFFC0CB),
			Plum							= static_cast<int>(0xFFDDA0DD),
			PowderBlue						= static_cast<int>(0xFFB0E0E6),
			Purple							= static_cast<int>(0xFF800080),
			Red								= static_cast<int>(0xFFFF0000),
			RosyBrown						= static_cast<int>(0xFFBC8F8F),
			RoyalBlue						= static_cast<int>(0xFF4169E1),
			SaddleBrown						= static_cast<int>(0xFF8B4513),
			Salmon							= static_cast<int>(0xFFFA8072),
			SandyBrown						= static_cast<int>(0xFFF4A460),
			SeaGreen						= static_cast<int>(0xFF2E8B57),
			SeaShell						= static_cast<int>(0xFFFFF5EE),
			Sienna							= static_cast<int>(0xFFA0522D),
			Silver							= static_cast<int>(0xFFC0C0C0),
			SkyBlue							= static_cast<int>(0xFF87CEEB),
			SlateBlue						= static_cast<int>(0xFF6A5ACD),
			SlateGray						= static_cast<int>(0xFF708090),
			Snow							= static_cast<int>(0xFFFFFAFA),
			SpringGreen						= static_cast<int>(0xFF00FF7F),
			SteelBlue						= static_cast<int>(0xFF4682B4),
			Tan								= static_cast<int>(0xFFD2B48C),
			Teal							= static_cast<int>(0xFF008080),
			Thistle							= static_cast<int>(0xFFD8BFD8),
			Tomato							= static_cast<int>(0xFFFF6347),
			Turquoise						= static_cast<int>(0xFF40E0D0),
			Violet							= static_cast<int>(0xFFEE82EE),
			Wheat							= static_cast<int>(0xFFF5DEB3),
			White							= static_cast<int>(0xFFFFFFFF),
			WhiteSmoke						= static_cast<int>(0xFFF5F5F5),
			Yellow							= static_cast<int>(0xFFFFFF00),
			YellowGreen						= static_cast<int>(0xFF9ACD32),
		};
	};

	struct Filter
	{
		enum Type : uint32_t
		{
			None,

			// CPU
			AI,
			Animation,
			Audio,
			Debug,
			Camera,
			Cloth,
			GameLogic,
			Input,
			Navigation,
			Network,
			Physics,
			Rendering,
			Scene,
			Script,
			Streaming,
			UI,
			VFX,
			Visibility,
			Wait,

			// IO
			IO,

			// GPU
			GPU_Cloth,
			GPU_Lighting,
			GPU_PostFX,
			GPU_Reflections,
			GPU_Scene,
			GPU_Shadows,
			GPU_UI,
			GPU_VFX,
			GPU_Water,
		};
	};

	struct Category
	{
		enum Type : uint64_t
		{
			// CPU
			None							= OPTICK_MAKE_CATEGORY(Filter::None, Color::Null),
			AI								= OPTICK_MAKE_CATEGORY(Filter::AI, Color::Purple),
			Animation						= OPTICK_MAKE_CATEGORY(Filter::Animation, Color::LightSkyBlue),
			Audio							= OPTICK_MAKE_CATEGORY(Filter::Audio, Color::HotPink),
			Debug							= OPTICK_MAKE_CATEGORY(Filter::Debug, Color::Black),
			Camera							= OPTICK_MAKE_CATEGORY(Filter::Camera, Color::Black),
			Cloth							= OPTICK_MAKE_CATEGORY(Filter::Cloth, Color::DarkGreen),
			GameLogic						= OPTICK_MAKE_CATEGORY(Filter::GameLogic, Color::RoyalBlue),
			Input							= OPTICK_MAKE_CATEGORY(Filter::Input, Color::Ivory),
			Navigation						= OPTICK_MAKE_CATEGORY(Filter::Navigation, Color::Magenta),
			Network							= OPTICK_MAKE_CATEGORY(Filter::Network, Color::Olive),
			Physics							= OPTICK_MAKE_CATEGORY(Filter::Physics, Color::LawnGreen),
			Rendering						= OPTICK_MAKE_CATEGORY(Filter::Rendering, Color::BurlyWood),
			Scene							= OPTICK_MAKE_CATEGORY(Filter::Scene, Color::RoyalBlue),
			Script							= OPTICK_MAKE_CATEGORY(Filter::Script, Color::Plum),
			Streaming						= OPTICK_MAKE_CATEGORY(Filter::Streaming, Color::Gold),
			UI								= OPTICK_MAKE_CATEGORY(Filter::UI, Color::PaleTurquoise),
			VFX								= OPTICK_MAKE_CATEGORY(Filter::VFX, Color::SaddleBrown),
			Visibility						= OPTICK_MAKE_CATEGORY(Filter::Visibility, Color::Snow),
			Wait							= OPTICK_MAKE_CATEGORY(Filter::Wait, Color::Tomato),
			WaitEmpty						= OPTICK_MAKE_CATEGORY(Filter::Wait, Color::White),

			// IO
			IO								= OPTICK_MAKE_CATEGORY(Filter::IO, Color::Khaki),
				
			// GPU
			GPU_Cloth						= OPTICK_MAKE_CATEGORY(Filter::GPU_Cloth, Color::DarkGreen),
			GPU_Lighting					= OPTICK_MAKE_CATEGORY(Filter::GPU_Lighting, Color::Khaki),
			GPU_PostFX						= OPTICK_MAKE_CATEGORY(Filter::GPU_PostFX, Color::Maroon),
			GPU_Reflections					= OPTICK_MAKE_CATEGORY(Filter::GPU_Reflections, Color::CadetBlue),
			GPU_Scene						= OPTICK_MAKE_CATEGORY(Filter::GPU_Scene, Color::RoyalBlue),
			GPU_Shadows						= OPTICK_MAKE_CATEGORY(Filter::GPU_Shadows, Color::LightSlateGray),
			GPU_UI							= OPTICK_MAKE_CATEGORY(Filter::GPU_UI, Color::PaleTurquoise),
			GPU_VFX							= OPTICK_MAKE_CATEGORY(Filter::GPU_VFX, Color::SaddleBrown),
			GPU_Water						= OPTICK_MAKE_CATEGORY(Filter::GPU_Water, Color::SteelBlue),
		};

		static uint32_t GetMask(Type t) { return static_cast<uint32_t>(t >> 32); }
		static uint32_t GetColor(Type t) { return static_cast<uint32_t>(t); }
	};
}

#define ARC_ENABLE_CPU_PROFILING 0
#if ARC_ENABLE_CPU_PROFILING
	#define ARC_PROFILE_FRAME(...)						::OPTICK_FRAME(__VA_ARGS__);
	#define ARC_PROFILE_SCOPE(...)						::OPTICK_EVENT(__VA_ARGS__);
	#define ARC_PROFILE_TAG(NAME, ...)					::OPTICK_TAG(NAME, __VA_ARGS__);
	#define ARC_PROFILE_SCOPE_DYNAMIC(NAME)				::OPTICK_EVENT_DYNAMIC(NAME);
	#define ARC_PROFILE_THREAD(...)						::OPTICK_THREAD(__VA_ARGS__);
	#define ARC_PROFILE_CATEGORY(NAME, CATEGORY)		::OPTICK_CATEGORY(NAME, (static_cast<Optick::Category::Type>(CATEGORY)));
#else
	#define ARC_PROFILE_FRAME(...)
	#define ARC_PROFILE_SCOPE(...)
	#define ARC_PROFILE_TAG(NAME, ...)
	#define ARC_PROFILE_SCOPE_DYNAMIC(NAME)
	#define ARC_PROFILE_THREAD(...)
	#define ARC_PROFILE_CATEGORY(NAME, CATEGORY)
#endif
