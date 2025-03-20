#include "pch.h"
#include "Random.h"

std::mt19937	Random::s_Generator;

void Random::Initialize()
{
	std::random_device rd;
	s_Generator.seed(rd());
}

float Random::GetFloatBetweenZeroAndOne()
{
	return GetFloatRange(0.f, 1.f);
}

int32 Random::GetIntBetweenZeroAndOne()
{
	return GetIntRange(0, 1);
}

float Random::GetFloatRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(s_Generator);
}

int32 Random::GetIntRange(int32 min, int32 max)
{
	std::uniform_int_distribution<int32> dist(min, max);
	return dist(s_Generator);
}

Vector2 Random::GetVector2(const Vector2& min, const Vector2& max)
{
	Vector2 random = Vector2(GetFloatBetweenZeroAndOne(), GetFloatBetweenZeroAndOne());
	return min + (max - min) * random;
}

Vector2Int Random::GetVector2Int(const Vector2Int& min, const Vector2Int& max)
{
	Vector2Int random = Vector2Int(GetIntBetweenZeroAndOne(), GetIntBetweenZeroAndOne());
	return min + (max - min) * random;
}
