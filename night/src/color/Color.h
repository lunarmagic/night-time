#pragma once

//#include "utility.h"
#include "core.h"
//#include "random/random.h"

#define COLOR_ZERO	night::Color(0.0f, 0.0f, 0.0f, 0.0f)
#define INVISIBLE	night::Color(1.0f, 1.0f, 1.0f, 0.0f)
#define COLOR_NULL	night::Color(-1.0f, -1.0f, -1.0f, -1.0f)
#define LIGHT		night::Color(0.8f, 0.8f, 0.8f, 1.0f)
#define BLACK		night::Color(0.0f, 0.0f, 0.0f, 1.0f)
#define WHITE		night::Color(1.0f, 1.0f, 1.0f, 1.0f)
#define DARK_GREY	night::Color(0.25f, 0.25f, 0.25f, 1.0f)
#define GREY		night::Color(0.5f, 0.5f, 0.5f, 1.0f)
#define RED			night::Color(1.0f, 0.0f, 0.0f, 1.0f)
#define GREEN		night::Color(0.0f, 1.0f, 0.0f, 1.0f)
#define BLUE		night::Color(0.0f, 0.0f, 1.0f, 1.0f)
#define LIGHT_BLUE  night::Color(0.04296875f, 0.63671875f, 1.0f, 1.0f)
#define CYAN		night::Color(0.0f, 1.0f, 1.0f, 1.0f)
#define YELLOW		night::Color(1.0f, 1.0f, 0.0f, 1.0f)
#define PURPLE		night::Color(1.0f, 0.0f, 1.0f, 1.0f)
#define ORANGE		night::Color(1.0f, 0.5f, 0.0f, 1.0f)
#define PINK		night::Color(1.0f, 0.4f, 0.7f, 1.0f)
#define BROWN		night::Color(0.57f, 0.31f, 0.08f, 1.0f)

namespace night
{

	struct Color8;

	struct NIGHT_API Color
	{
		real r{ 1.0f };
		real g{ 1.0f };
		real b{ 1.0f };
		real a{ 1.0f };

		Color() = default;
		Color(const Color& other) = default;
		Color(real r, real g, real b, real a = 1.0f) : r(r), g(g), b(b), a(a) {}
		Color(const Color8& color8);

		inline const Color operator* (const real& f) const
		{
			return Color((r * f), (g * f), (b * f), a);
		}

		inline const Color operator/ (const real& f) const
		{
			return Color((r / f), (g / f), (b / f), a);
		}

		inline const Color operator*= (const real& f)
		{
			r = (r * f);
			g = (g * f);
			b = (b * f);
			return *this;
		}

		inline const Color operator/= (const real& f)
		{
			r = (r / f);
			g = (g / f);
			b = (b / f);
			return *this;
		}

		inline const u8 operator==(Color const& other) const
		{
			return (r == other.r && g == other.g && b == other.b && a == other.a);
		}

		inline const u8 operator!=(Color const& other) const
		{
			return !(r == other.r && g == other.g && b == other.b && a == other.a);
		}

		static Color random();

		static Color rainbow(real t);

		static Color lerp(const Color& a, const Color& b, real t);

		Color opaqued(real opacity) const
		{
			Color result = *this;
			result.a *= opacity;
			return result;
		}

		Color darken(real lightness) const
		{
			Color result = *this;
			result.r *= lightness;
			result.g *= lightness;
			result.b *= lightness;
			return result;
		}
	};

	struct /*NIGHT_API*/ Color8
	{
		u8 r{ 0 };
		u8 g{ 0 };
		u8 b{ 0 };
		u8 a{ 0 };

		Color8() = default;
		Color8(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}

		Color8(const Color& color)
		{
			r = (u8)(color.r * 255);
			g = (u8)(color.g * 255);
			b = (u8)(color.b * 255);
			a = (u8)(color.a * 255);
		}
	};

}