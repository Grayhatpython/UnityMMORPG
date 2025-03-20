#pragma once
#include <random>
#include "Math.h"

class Random
{
public:
	static void		Initialize();
	static void		Seed(uint32 seed);
	static float	GetZeroToOne();

	template<typename T>
	static T GetRandom(T min, T max)
	{
		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(S_Generator);
		}
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(S_Generator);
		}
	}

	static Vector2	GetRandomVector(const Vector2& min, const Vector2& max);

private:
	static std::mt19937 S_Generator;
};
