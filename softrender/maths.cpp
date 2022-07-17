#include "maths.h"

namespace trans
{
    Mat4 identity()
    {
        Mat4 m;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = i == j;
        return m;
    }
	
    Mat4 translate(const Vec3& v)
    {
        Mat4 m = identity();
		m[0][3] = v.x;
		m[1][3] = v.y;
		m[2][3] = v.z;
		return m;
    }
	
    Mat4 translate(float x, float y, float z)
    {
        return translate(Vec3{ x, y, z });
    }
	
    Mat4 rotate(float rad, const Vec3& axis)
    {
		Mat4 m = identity();
		float c = cos(rad);
		float s = sin(rad);
		float omc = 1 - c;
		Vec3 a = glm::normalize(axis);
		float x = a.x;
		float y = a.y;
		float z = a.z;
		m[0][0] = x * x * omc + c;
		m[0][1] = x * y * omc - z * s;
		m[0][2] = x * z * omc + y * s;
		m[1][0] = y * x * omc + z * s;
		m[1][1] = y * y * omc + c;
		m[1][2] = y * z * omc - x * s;
		m[2][0] = z * x * omc - y * s;
		m[2][1] = z * y * omc + x * s;
		m[2][2] = z * z * omc + c;
		return m;
    }
	
    Mat4 rotate(float rad, float ax, float ay, float az)
    {
        return rotate(rad, Vec3{ ax, ay, az });
    }
	
    Mat4 scale(const Vec3& v)
    {
        Mat4 m = identity();
		m[0][0] = v.x;
		m[1][1] = v.y;
		m[2][2] = v.z;
		return m;
    }
	
    Mat4 scale(float x, float y, float z)
    {
        return scale(Vec3{ x, y, z });
    }
	
    Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
    {
		Vec3 f = glm::normalize(center - eye);
		Vec3 u = glm::normalize(up);
		Vec3 s = glm::normalize(glm::cross(f, u));
		u = glm::cross(s, f);
		Mat4 m;
		m[0][0] = s.x;
		m[1][0] = s.y;
		m[2][0] = s.z;
		m[0][1] = u.x;
		m[1][1] = u.y;
		m[2][1] = u.z;
		m[0][2] = -f.x;
		m[1][2] = -f.y;
		m[2][2] = -f.z;
		m[0][3] = -glm::dot(s, eye);
		m[1][3] = -glm::dot(u, eye);
		m[2][3] = glm::dot(f, eye);
		return m;
    }

	Mat4 perspective(float fov, float aspect, float near, float far)
	{
		Mat4 m;
		float f = 1.0f / tan(fov / 2);
		m[0][0] = f / aspect;
		m[1][1] = f;
		m[2][2] = (far + near) / (near - far);
		m[3][2] = -1;
		m[2][3] = (2 * far * near) / (near - far);
		return m;
	}
		
	Mat4 ortho(float left, float right, float bottom, float top, float near, float far)
	{
		Mat4 m;
		m[0][0] = 2 / (right - left);
		m[1][1] = 2 / (top - bottom);
		m[2][2] = -2 / (far - near);
		m[0][3] = -(right + left) / (right - left);
		m[1][3] = -(top + bottom) / (top - bottom);
		m[2][3] = -(far + near) / (far - near);
		return m;
	}

	Mat3 normalTransformMat(const Mat4& modelview)
	{
		return glm::inverse(mat3(modelview));
	}
}
