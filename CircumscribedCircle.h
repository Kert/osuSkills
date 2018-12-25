#ifndef _CircumscribedCircle_h_
#define _CircumscribedCircle_h_

#include "Slider.h"

class CircumscribedCircle : Slider {
	
public:
	/** Points along the curve (set by inherited classes). */
	std::vector<Vector2d> curve;
	/** The number of points along the curve. */
	int ncurve;
private:
	/** PI constants. */
	const double TWO_PI = (double)(M_PI * 2.),
		HALF_PI = (double)(M_PI / 2.);

	/** The center of the Circumscribed Circle. */
	Vector2d circleCenter;

	/** The radius of the Circumscribed Circle. */
	double radius;

	/** The three points to create the Circumscribed Circle from. */
	Vector2d start, mid, end;

	/** The three angles relative to the circle center. */
	double startAng, endAng, midAng;

	/**
	* Constructor.
	* @param hitObject the associated HitObject
	* @param color the color of this curve
	*/
public:
	CircumscribedCircle(HitObject hitObject);

	/**
	* Checks to see if "b" is between "a" and "c"
	* @return true if b is between a and c
	*/
	bool isIn(double a, double b, double c);

	/**
	* Finds the point of intersection between the two parametric lines
	* {@code A = a + ta*t} and {@code B = b + tb*u}.
	* http://gamedev.stackexchange.com/questions/44720/
	* @param a  the initial position of the line A
	* @param ta the direction of the line A
	* @param b  the initial position of the line B
	* @param tb the direction of the line B
	* @return the point at which the two lines intersect
	*/
	Vector2d intersect(Vector2d a, Vector2d ta, Vector2d b, Vector2d tb);

	Vector2d pointAt(double t);
};

#endif