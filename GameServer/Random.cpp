#include "pch.h"
#include "Random.h"

std::mt19937 Random::S_Generator;

void Random::Initialize()
{
	std::random_device randomDevice;
	Random::Seed(randomDevice());
}

void Random::Seed(uint32 seed)
{
	S_Generator.seed(seed);
}

float Random::GetZeroToOne()
{
	return GetRandom(0.0f, 1.0f);
}

Vector2 Random::GetRandomVector(const Vector2& min, const Vector2& max)
{
	Vector2 r = Vector2(GetZeroToOne(), GetZeroToOne());
	return min + (max - min) * r;
}
