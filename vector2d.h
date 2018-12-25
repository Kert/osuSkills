#ifndef _vector2d_h_
#define _vector2d_h_

#include <cmath>
#include <vector>

using namespace std;

// irrlicht
//! returns minimum of two values. Own implementation to get rid of the STL (VS6 problems)
template<class T>
inline const T& min_(const T& a, const T& b)
{
	return a < b ? a : b;
}

//! returns maximum of two values. Own implementation to get rid of the STL (VS6 problems)
template<class T>
inline const T& max_(const T& a, const T& b)
{
	return a < b ? b : a;
}

//! clamps a value between low and high
template <class T>
inline const T clamp(const T& value, const T& low, const T& high)
{
	return min_(max_(value, low), high);
}

inline double reciprocal_squareroot(const double x)
{
	return 1.0 / sqrt(x);
}

class Vector2d
{
public:
	double X = 0;
	double Y = 0;

	Vector2d() : X(0), Y(0) {}
	Vector2d(double nx, double ny) : X(nx), Y(ny) {}

	Vector2d operator-() const { return Vector2d(-X, -Y); }
	Vector2d& operator=(const Vector2d& other) { X = other.X; Y = other.Y; return *this; }

	Vector2d operator+(const Vector2d& other) const { return Vector2d(X + other.X, Y + other.Y); }
	Vector2d& operator+=(const Vector2d& other) { X += other.X; Y += other.Y; return *this; }
	Vector2d operator+(const double v) const { return Vector2d(X + v, Y + v); }
	Vector2d& operator+=(const double v) { X += v; Y += v; return *this; }

	Vector2d operator-(const Vector2d& other) const { return Vector2d(X - other.X, Y - other.Y); }
	Vector2d& operator-=(const Vector2d& other) { X -= other.X; Y -= other.Y; return *this; }
	Vector2d operator-(const double v) const { return Vector2d(X - v, Y - v); }
	Vector2d& operator-=(const double v) { X -= v; Y -= v; return *this; }

	Vector2d operator*(const Vector2d& other) const { return Vector2d(X * other.X, Y * other.Y); }
	Vector2d& operator*=(const Vector2d& other) { X *= other.X; Y *= other.Y; return *this; }
	Vector2d operator*(const double v) const { return Vector2d(X * v, Y * v); }
	Vector2d& operator*=(const double v) { X *= v; Y *= v; return *this; }

	Vector2d operator/(const Vector2d& other) const { return Vector2d(X / other.X, Y / other.Y); }
	Vector2d& operator/=(const Vector2d& other) { X /= other.X; Y /= other.Y; return *this; }
	Vector2d operator/(const double v) const { return Vector2d(X / v, Y / v); }
	Vector2d& operator/=(const double v) { X /= v; Y /= v; return *this; }

	bool operator==(const Vector2d& other) const { return (abs(X - other.X) < 0.000001 && abs(Y - other.Y) < 0.000001); }
	bool operator!=(const Vector2d& other) const { return !(abs(X - other.X) < 0.00001 && abs(Y - other.Y) < 0.00001); }

	double getLength() const { return sqrt(X*X + Y*Y); }
	double getDistanceFrom(const Vector2d& other) const
	{
		return Vector2d(X - other.X, Y - other.Y).getLength();
	}

	Vector2d& normalize()
	{
		double length = double(X*X + Y*Y);
		if (length == 0)
			return *this;
		length = reciprocal_squareroot(length);
		X = (double)(X * length);
		Y = (double)(Y * length);
		return *this;
	}

	Vector2d& set(double nx, double ny) { X = nx; Y = ny; return *this; }

	Vector2d& rotateBy(double degrees, const Vector2d& center = Vector2d())
	{
		degrees *= 3.1415926535897932384626433832795028841971693993751 / 180.0;
		const double cs = cos(degrees);
		const double sn = sin(degrees);

		X -= center.X;
		Y -= center.Y;

		set((double)(X*cs - Y*sn), (double)(X*sn + Y*cs));

		X += center.X;
		Y += center.Y;
		return *this;
	}

	inline double getAngle() const
	{
		if (Y == 0) // corrected thanks to a suggestion by Jox
			return X < 0 ? 180 : 0;
		else if (X == 0)
			return Y < 0 ? 90 : 270;

		// don't use getLength here to avoid precision loss with s32 vectors
		// avoid floating-point trouble as sqrt(y*y) is occasionally larger than y, so clamp
		const double tmp = clamp(Y / sqrt((double)(X*X + Y*Y)), -1.0, 1.0);
		const double angle = atan(sqrt(1 - tmp*tmp) / tmp) * (180.0 / 3.1415926535897932384626433832795028841971693993751);

		if (X>0 && Y>0)
			return angle + 270;
		else
		if (X>0 && Y<0)
			return angle + 90;
		else
		if (X<0 && Y<0)
			return 90 - angle;
		else
		if (X<0 && Y>0)
			return 270 - angle;

		return angle;
	}
	double getLengthSQ() const { return X*X + Y*Y; }
	Vector2d midPoint(Vector2d o) {
		return Vector2d((X + o.X) / 2., (Y + o.Y) / 2.);
	}
	Vector2d nor()
	{
		Vector2d tmp;
		tmp.X = -Y;
		tmp.Y = X;
		return tmp;
	}
};

#endif