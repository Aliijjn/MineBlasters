#include "Vector2.hpp"

Vec2    normaliseVec2(Vec2 a)
{
    float   totalLen = sqrtf(a.x * a.x + a.y * a.y);

    return { a.x / totalLen, a.y / totalLen };
}

Vec2    normaliseVec2Safe(Vec2 a)
{
	if (a.x == 0 && a.y == 0)
		return { 0, 0 };

	float   totalLen = sqrtf(a.x * a.x + a.y * a.y);

	return { a.x / totalLen, a.y / totalLen };
}

bool	AABB(Vec2 a_min, Vec2 a_max, Vec2 b_min, Vec2 b_max)
{
	return (a_min.x <= b_max.x && a_max.x >= b_min.x) && (a_min.y <= b_max.y && a_max.y >= b_min.y);
}

bool	AABBMiddle(Vec2 a, Vec2 aSize, Vec2 b, Vec2 bSize)
{
	Vec2	a_min = subtractVec2(a, multiplyVec2(aSize, 0.5f));
	Vec2	a_max = addVec2(a, multiplyVec2(aSize, 0.5f));
	Vec2	b_min = subtractVec2(b, multiplyVec2(bSize, 0.5f));
	Vec2	b_max = addVec2(b, multiplyVec2(bSize, 0.5f));

	return (AABB(a_min, a_max, b_min, b_max));
}

IVec2   toIVec2(Vec2 cpy)
{
	return { (int)cpy.x, (int)cpy.y };
}

IVec2   toIVec2(Vec2 cpy, int scalar)
{
	return { (int)(cpy.x * scalar), (int)(cpy.y * scalar) };
}

Vec2   toVec2(IVec2 cpy)
{
	return { (float)cpy.x, (float)cpy.y };
}

Vec2   toVec2(IVec2 cpy, float scalar)
{
	return { (float)(cpy.x * scalar), (float)(cpy.y * scalar) };
}

Vec2    addVec2(Vec2 a, Vec2 b)
{
	return { a.x + b.x, a.y + b.y };
}

Vec2    subtractVec2(Vec2 a, Vec2 b)
{
	return { a.x - b.x, a.y - b.y };
}

Vec2    multiplyVec2(Vec2 a, float scalar)
{
	return { a.x * scalar, a.y * scalar };
}



IVec2    addIVec2(IVec2 a, IVec2 b)
{
	return { a.x + b.x, a.y + b.y };
}

IVec2    subtractIVec2(IVec2 a, IVec2 b)
{
	return { a.x - b.x, a.y - b.y };
}

IVec2    multiplyIVec2(IVec2 a, float scalar)
{
	return { (int)(a.x * scalar), (int)(a.y * scalar) };
}

//// Vector2D constructors/destructors
//
//constexpr Vec2::Vec2() : x(0), y(0) {}
//
//constexpr Vec2::Vec2(float newX, float newY) : x(newX), y(newY) {}
//
//Vec2::Vec2(const Vec2& cpy) : x(cpy.x), y(cpy.y) {}
//
////Vec2::Vec2(const IVec2& cpy) : x((float)(cpy.x)), y((float)cpy.y) {}
//
//Vec2& Vec2::operator=(const Vec2& cpy)
//{
//    x = cpy.x;
//    y = cpy.y;
//    return *this;
//}
//
//Vec2::~Vec2() {}
//
//// Vector2D math
//
//Vec2 Vec2::operator+(const Vec2& other) const
//{
//    return Vec2(x + other.x, y + other.y);
//}
//
//Vec2 Vec2::operator-(const Vec2& other) const
//{
//    return Vec2(x - other.x, y - other.y);
//}
//
//Vec2 Vec2::operator*(const Vec2& other) const
//{
//    return Vec2(x * other.x, y * other.y);
//}
//
//Vec2 Vec2::operator*(float scale) const
//{
//    return Vec2(x * scale, y * scale);
//}
//
//Vec2 Vec2::operator/(float scale) const
//{
//    if (scale == 0)
//        return Vec2(0, 0);
//    return Vec2(x * scale, y * scale);
//}
//
//// Vector2D logic
//
//bool Vec2::operator==(const Vec2& other) const
//{
//    return x == other.x && y == other.y;
//}
//
//bool Vec2::operator!=(const Vec2& other) const
//{
//    return !(x == other.x && y == other.y);
//}
//
//// ----------------------------- Integer Vector2 ------------------------------
//
//// IVector2D constructors/destructors
// 
//constexpr IVec2::IVec2() : x(0), y(0) {}
//
//constexpr IVec2::IVec2(const int32_t newX, const int32_t newY) : x(newX), y(newY) {}
//
//IVec2::IVec2(const Vec2& cpy) : x((int32_t)cpy.x), y((int32_t)cpy.y) {}
//
//IVec2::IVec2(const IVec2& cpy) : x(cpy.x), y(cpy.y) {}
//
//IVec2& IVec2::operator=(const IVec2& cpy)
//{
//    x = cpy.x;
//    y = cpy.y;
//    return *this;
//}
//
//IVec2::~IVec2() {}
//
//// IVector2D math
//
//IVec2 IVec2::operator+(const IVec2& other) const
//{
//    return IVec2(x + other.x, y + other.y);
//}
//
//IVec2 IVec2::operator-(const IVec2& other) const
//{
//    return IVec2(x - other.x, y - other.y);
//}
//
//IVec2 IVec2::operator*(const IVec2& other) const
//{
//    return IVec2(x * other.x, y * other.y);
//}
//
//IVec2 IVec2::operator*(float scale) const
//{
//    return IVec2(x * scale, y * scale);
//}
//
//IVec2 IVec2::operator/(float scale) const
//{
//    if (scale == 0)
//        return IVec2(0, 0);
//    return IVec2(x * scale, y * scale);
//}
//
//// IVector2D logic
//
//bool IVec2::operator==(const IVec2& other) const
//{
//    return x == other.x && y == other.y;
//}
//
//bool IVec2::operator!=(const IVec2& other) const
//{
//    return !(x == other.x && y == other.y);
//}
