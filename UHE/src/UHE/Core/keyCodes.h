#pragma once

namespace UHE {

	using KeyCode = uint16_t;

	namespace Key
	{
		enum : KeyCode
		{
			/* Printable keys */
			Space = 32,
			Apostrophe = 39, /* ' */
			Comma = 44, /* , */
			Minus = 45, /* - */
			Period = 46, /* . */
			Slash = 47,

			D0 = 48,
			D1 = 49,
			D2 = 50,
			D3 = 51,
			D4 = 52,
			D5 = 53,
			D6 = 54,
			D7 = 55,
			D8 = 56,
			D9 = 57,

			Semicolon = 59, /* ; */
			Equal = 61, /* = */

			A = 65,
			B = 66,
			C = 67,
			D = 68,
			E = 69,
			F = 70,
			G = 71,
			H = 72,
			I = 73,
			J = 74,
			K = 75,
			L = 76,
			M = 77,
			N = 78,
			O = 79,
			P = 80,
			Q = 81,
			R = 82,
			S = 83,
			T = 84,
			U = 85,
			V = 86,
			W = 87,
			X = 88,
			Y = 89,
			Z = 90,

			LeftBracket = 91,  /* [ */
			Backslash = 92,  /* \ */
			RightBracket = 93,  /* ] */
			GraveAccent = 96,  /* ` */

			/* Function keys */

			Escape = 256,
			Enter = 257,
			Tab = 258,
			Backspace = 259,
			Insert = 260,
			Delete = 261,
			Right = 262,
			Left = 263,
			Down = 264,
			Up = 265,
			PageUp = 266,
			PageDown = 267,
			Home = 268,
			End = 269,
			CapsLock = 280,
			ScrollLock = 281,
			NumLock = 282,
			PrintScreen = 283,
			Pause = 284,

			F1 = 290,
			F2 = 291,
			F3 = 292,
			F4 = 293,
			F5 = 294,
			F6 = 295,
			F7 = 296,
			F8 = 297,
			F9 = 298,
			F10 = 299,
			F11 = 300,
			F12 = 301,
			F13 = 302,
			F14 = 303,
			F15 = 304,
			F16 = 305,
			F17 = 306,
			F18 = 307,
			F19 = 308,
			F20 = 309,
			F21 = 310,
			F22 = 311,
			F23 = 312,
			F24 = 313,
			F25 = 314,

			/* Keypad */

			KP0 = 320,
			KP1 = 321,
			KP2 = 322,
			KP3 = 323,
			KP4 = 324,
			KP5 = 325,
			KP6 = 326,
			KP7 = 327,
			KP8 = 328,
			KP9 = 329,
			KPDecimal = 330,
			KPDivide = 331,
			KPMultiply = 332,
			KPSubtract = 333,
			KPAdd = 334,
			KPEnter = 335,
			KPEqual = 336,

			/* Modifiers */

			LeftShift = 340,
			LeftControl = 341,
			LeftAlt = 342,
			LeftSuper = 343,
			RightShift = 344,
			RightControl = 345,
			RightAlt = 346,
			RightSuper = 347,
			Menu = 348
		};
	}
}

/* Macro helpers */

#define UHE_KEY_SPACE           ::UHE::Key::Space
#define UHE_KEY_APOSTROPHE      ::UHE::Key::Apostrophe
#define UHE_KEY_COMMA           ::UHE::Key::Comma
#define UHE_KEY_MINUS           ::UHE::Key::Minus
#define UHE_KEY_PERIOD          ::UHE::Key::Period
#define UHE_KEY_SLASH           ::UHE::Key::Slash
#define UHE_KEY_0               ::UHE::Key::D0
#define UHE_KEY_1               ::UHE::Key::D1
#define UHE_KEY_2               ::UHE::Key::D2
#define UHE_KEY_3               ::UHE::Key::D3
#define UHE_KEY_4               ::UHE::Key::D4
#define UHE_KEY_5               ::UHE::Key::D5
#define UHE_KEY_6               ::UHE::Key::D6
#define UHE_KEY_7               ::UHE::Key::D7
#define UHE_KEY_8               ::UHE::Key::D8
#define UHE_KEY_9               ::UHE::Key::D9

#define UHE_KEY_A               ::UHE::Key::A
#define UHE_KEY_B               ::UHE::Key::B
#define UHE_KEY_C               ::UHE::Key::C
#define UHE_KEY_D               ::UHE::Key::D
#define UHE_KEY_E               ::UHE::Key::E
#define UHE_KEY_F               ::UHE::Key::F
#define UHE_KEY_G               ::UHE::Key::G
#define UHE_KEY_H               ::UHE::Key::H
#define UHE_KEY_I               ::UHE::Key::I
#define UHE_KEY_J               ::UHE::Key::J
#define UHE_KEY_K               ::UHE::Key::K
#define UHE_KEY_L               ::UHE::Key::L
#define UHE_KEY_M               ::UHE::Key::M
#define UHE_KEY_N               ::UHE::Key::N
#define UHE_KEY_O               ::UHE::Key::O
#define UHE_KEY_P               ::UHE::Key::P
#define UHE_KEY_Q               ::UHE::Key::Q
#define UHE_KEY_R               ::UHE::Key::R
#define UHE_KEY_S               ::UHE::Key::S
#define UHE_KEY_T               ::UHE::Key::T
#define UHE_KEY_U               ::UHE::Key::U
#define UHE_KEY_V               ::UHE::Key::V
#define UHE_KEY_W               ::UHE::Key::W
#define UHE_KEY_X               ::UHE::Key::X
#define UHE_KEY_Y               ::UHE::Key::Y
#define UHE_KEY_Z               ::UHE::Key::Z

#define UHE_KEY_ESCAPE          ::UHE::Key::Escape
#define UHE_KEY_ENTER           ::UHE::Key::Enter
#define UHE_KEY_TAB             ::UHE::Key::Tab
#define UHE_KEY_BACKSPACE       ::UHE::Key::Backspace
#define UHE_KEY_INSERT          ::UHE::Key::Insert
#define UHE_KEY_DELETE          ::UHE::Key::Delete

#define UHE_KEY_RIGHT           ::UHE::Key::Right
#define UHE_KEY_LEFT            ::UHE::Key::Left
#define UHE_KEY_DOWN            ::UHE::Key::Down
#define UHE_KEY_UP              ::UHE::Key::Up

#define UHE_KEY_LEFT_SHIFT      ::UHE::Key::LeftShift
#define UHE_KEY_LEFT_CONTROL    ::UHE::Key::LeftControl
#define UHE_KEY_LEFT_ALT        ::UHE::Key::LeftAlt
#define UHE_KEY_LEFT_SUPER      ::UHE::Key::LeftSuper
#define UHE_KEY_RIGHT_SHIFT     ::UHE::Key::RightShift
#define UHE_KEY_RIGHT_CONTROL   ::UHE::Key::RightControl
#define UHE_KEY_RIGHT_ALT       ::UHE::Key::RightAlt
#define UHE_KEY_RIGHT_SUPER     ::UHE::Key::RightSuper