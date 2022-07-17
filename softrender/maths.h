#ifndef MATHS_H
#define MATHS_H

#undef near
#undef far

#include <algorithm>
#include <glm/glm.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;
using Color3 = glm::vec3;
using Color4 = glm::vec4;

#undef TRANSPARENT

namespace Color
{
	inline static const Color4 WHITE		= Color4(1.0f, 1.0f, 1.0f, 1.0f);
	inline static const Color4 BLACK		= Color4(0.0f, 0.0f, 0.0f, 1.0f);
	inline static const Color4 RED			= Color4(1.0f, 0.0f, 0.0f, 1.0f);
	inline static const Color4 GREEN		= Color4(0.0f, 1.0f, 0.0f, 1.0f);
	inline static const Color4 BLUE			= Color4(0.0f, 0.0f, 1.0f, 1.0f);
	inline static const Color4 TRANSPARENT  = Color4(0.0f, 0.0f, 0.0f, 0.0f);
}
inline constexpr float EPS = 1e-4f;
inline constexpr float PI = 3.14159265358979323846f;

inline Vec2 vec2(float x)		   { return Vec2(x, x);				}
inline Vec2 vec2(const Vec3& vec3) { return Vec2(vec3.x, vec3.y);	}
inline Vec2 vec2(const Vec4& vec4) { return Vec2(vec4.x, vec4.y);	}

inline Vec3 vec3(float x)							{ return Vec3(x, x, x);					}
inline Vec3 vec3(const Vec2& vec2, float z = 0.0f)	{ return Vec3(vec2, z);					}
inline Vec3 vec3(const Vec4& vec4)					{ return Vec3(vec4.x, vec4.y, vec4.z);  }
inline Vec3 vec3(float x, float y, float z)			{ return Vec3(x, y, z); }

inline Vec4 vec4(const Vec2& vec2, float z = 0.0f, float w = 1.0f)  { return Vec4(vec2, z, w); }
inline Vec4 vec4(const Vec3& vec3, float w = 1.0f)					{ return Vec4(vec3, w);    }
inline Vec4 vec4(float x, float y, float z = 0.0f, float w = 1.0f)  { return Vec4(x, y, z, w); }
inline Vec4 vec4(float x)										    { return Vec4(x, x, x, 1.0f); }

inline Mat3 mat3(const Mat4& mat4)
{
	Mat3 mat3;
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			mat3[i][j] = mat4[i][j];
	return mat3;
}
inline Mat4 mat4(const Mat3& mat3)
{
	Mat4 mat4;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mat4[i][j] = i < 3 && j < 3 ? mat3[i][j] : i == j;
	return mat4;
}

template<class T>
inline T lerp(const T& a, const T& b, float t)
{
	return a + (b - a) * t;
}

template<class T>
inline T clamp(const T& x, const T& l, const T& r) 
{
	return std::min(r, std::max(l, x));
}

inline int sign(float x)
{
	return x < -EPS ? -1 : x > EPS;
}

inline float area(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

inline float deg2rad(float deg) { return deg / 180.0f * PI; }
inline float rad2deg(float rad) { return rad / PI * 180.0f; }

namespace trans
{
	Mat4 identity();

	Mat4 translate(const Vec3& v);

	Mat4 translate(float x, float y, float z);

	Mat4 rotate(float rad, const Vec3& axis);

	Mat4 rotate(float rad, float ax, float ay, float az);

	Mat4 scale(const Vec3& v);

	Mat4 scale(float x, float y, float z);

	Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

	Mat4 perspective(float fov, float aspect, float near, float far);

	Mat4 ortho(float left, float right, float bottom, float top, float near, float far);

	Mat3 normalTransformMat(const Mat4& modelview);
}

#endif
