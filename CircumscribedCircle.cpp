#include "CircumscribedCircle.h"
#include <iostream>

CircumscribedCircle::CircumscribedCircle(HitObject hitObject)
{
	for (unsigned i = 0; i < hitObject.curves.size(); i++)
	{
		sliderX.push_back(hitObject.curves[i].X);
		sliderY.push_back(hitObject.curves[i].Y);
	}
	x = hitObject.pos.X;
	y = hitObject.pos.Y;

	// construct the three points
	this->start = Vector2d(getX(0), getY(0));
	this->mid = Vector2d(getX(1), getY(1));
	this->end = Vector2d(getX(2), getY(2));

	// find the circle center
	Vector2d mida = start.midPoint(mid);
	Vector2d midb = end.midPoint(mid);
	Vector2d nora = (mid - start).nor();
	Vector2d norb = (mid - end).nor();

	this->circleCenter = intersect(mida, nora, midb, norb);
	if (circleCenter == Vector2d(-1, -1))
	{
		// Temporary fallback to bezier slider
		Slider slider(hitObject, true);
		curve.resize(slider.curve.size());
		curve = slider.curve;
		ncurve = slider.ncurve;
		return;
	}

	// find the angles relative to the circle center
	Vector2d startAngPoint = start - circleCenter;
	Vector2d midAngPoint = mid - circleCenter;
	Vector2d endAngPoint = end - circleCenter;

	this->startAng = atan2(startAngPoint.Y, startAngPoint.X);
	this->midAng = atan2(midAngPoint.Y, midAngPoint.X);
	this->endAng = atan2(endAngPoint.Y, endAngPoint.X);

	// find the angles that pass through midAng
	if (!isIn(startAng, midAng, endAng))
	{
		if (abs(startAng + TWO_PI - endAng) < TWO_PI && isIn(startAng + (TWO_PI), midAng, endAng))
			startAng += TWO_PI;
		else if (abs(startAng - (endAng + TWO_PI)) < TWO_PI && isIn(startAng, midAng, endAng + (TWO_PI)))
			endAng += TWO_PI;
		else if (abs(startAng - TWO_PI - endAng) < TWO_PI && isIn(startAng - (TWO_PI), midAng, endAng))
			startAng -= TWO_PI;
		else if (abs(startAng - (endAng - TWO_PI)) < TWO_PI && isIn(startAng, midAng, endAng - (TWO_PI)))
			endAng -= TWO_PI;
		else
		{
			std::cout << "Cannot find angles between midAng (%.3f %.3f %.3f)." << std::endl;
				//startAng, midAng, endAng), null, true
			return;
		}
	}

	// find an angle with an arc length of pixelLength along this circle
	this->radius = startAngPoint.getLength();
	double pixelLength = hitObject.pixelLength;
	double arcAng = pixelLength / radius;  // len = theta * r / theta = len / r

	// now use it for our new end angle
	this->endAng = (endAng > startAng) ? startAng + arcAng : startAng - arcAng;

	// calculate points
	double step = hitObject.pixelLength / CURVE_POINTS_SEPERATION;
	ncurve = (int)step;
	int len = (int)step + 1;
	for (int i = 0; i < len; i++)
	{
		Vector2d xy = pointAt(i / step);
		curve.push_back(Vector2d(xy.X, xy.Y));
	}
}

bool CircumscribedCircle::isIn(double a, double b, double c)
{
	return (b > a && b < c) || (b < a && b > c);
}

Vector2d CircumscribedCircle::intersect(Vector2d a, Vector2d ta, Vector2d b, Vector2d tb)
{
	double des = tb.X * ta.Y - tb.Y * ta.X;
	if (abs(des) < 0.00001)
	{
		std::cout << "Vectors are parallel." << std::endl;
		return Vector2d(-1, -1);
	}
	double u = ((b.Y - a.Y) * ta.X + (a.X - b.X) * ta.Y) / des;
	b.X += tb.X * u;
	b.Y += tb.Y * u;
	return b;
}

Vector2d CircumscribedCircle::pointAt(double t)
{
	double ang = lerp(startAng, endAng, t);
	return Vector2d((cos(ang) * radius + circleCenter.X),
					(sin(ang) * radius + circleCenter.Y));
}