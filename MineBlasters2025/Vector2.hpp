#pragma once

#include <cmath>
#include <cstdint>
#include <compare>

//class IVec2;
//
//// --------------------------------- Vector2D ---------------------------------
//
//class Vec2
//{
//    public:
//        float x, y;
//        constexpr Vec2();
//        constexpr Vec2(float newX, float newY);
//        Vec2(const Vec2& cpy);
//        /*Vec2(const IVec2& cpy);*/
//        Vec2& operator=(const Vec2& cpy);
//        ~Vec2();
//
//        Vec2 operator+(const Vec2& other) const;
//        Vec2 operator-(const Vec2& other) const;
//        Vec2 operator*(const Vec2& other) const;
//        Vec2 operator*(float scale) const;
//        Vec2 operator/(float scale) const;
//
//        bool operator==(const Vec2& other) const;
//        bool operator!=(const Vec2& other) const;
//};
//
//// --------------------------- Integer Vector2D -------------------------------
//
//class IVec2
//{
//    public:
//        int32_t x, y;
//        constexpr IVec2();
//        constexpr IVec2(const int32_t newX, const int32_t newY);
//        IVec2(const Vec2& cpy);
//        IVec2(const IVec2& cpy);
//        IVec2& operator=(const IVec2& cpy);
//        ~IVec2();
//
//        IVec2 operator+(const IVec2& other) const;
//        IVec2 operator-(const IVec2& other) const;
//        IVec2 operator*(const IVec2& other) const;
//        IVec2 operator*(float scale) const;
//        IVec2 operator/(float scale) const;
//
//        bool operator==(const IVec2& other) const;
//        bool operator!=(const IVec2& other) const;
//};

struct Vec2
{
	float x, y;

	bool operator<(const Vec2& other) const
	{
		return x + y < other.x + other.y;
	}
};

struct IVec2
{
	int32_t x, y;

	bool operator<(const IVec2& other) const
	{
		return x + y < other.x + other.y;
	}
};

bool	AABB(Vec2 a_min, Vec2 a_max, Vec2 b_min, Vec2 b_max);
bool	AABBMiddle(Vec2 a, Vec2 aSize, Vec2 b, Vec2 bSize);
Vec2    normaliseVec2(Vec2 a);
Vec2    normaliseVec2Safe(Vec2 a);

Vec2    addVec2(Vec2 a, Vec2 b);
Vec2    subtractVec2(Vec2 a, Vec2 b);
Vec2    multiplyVec2(Vec2 a, float scalar = 1);
Vec2	toVec2(IVec2 cpy, float scalar = 1);

IVec2   toIVec2(Vec2 cpy);
IVec2   addIVec2(IVec2 a, IVec2 b);
IVec2   subtractIVec2(IVec2 a, IVec2 b);
IVec2   multiplyIVec2(IVec2 a, float scalar = 1);
