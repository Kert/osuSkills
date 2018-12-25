#include "Bezier.h"

Bezier::Bezier(std::vector<Vector2d> _points)
{
	points = _points;

	// approximate by finding the length of all points
	// (which should be the max possible length of the curve)
	double approxlength = 0;
	for (unsigned i = 0; i < points.size() - 1; i++)
		approxlength += points[i].getDistanceFrom(points[i + 1]);

	init(approxlength);
}

void Bezier::init(double approxlength)
{
	// subdivide the curve
	ncurve = (int)(approxlength / 4.0) + 2;
	for (int i = 0; i < ncurve; i++)
		curvePoints.push_back(pointAt(i / (double)(ncurve - 1)));

	// find the distance of each point from the previous point
	//curveDis = new double[ncurve];
	totalDistance = 0;
	for (int i = 0; i < ncurve; i++) 
	{
		curveDis.push_back((i == 0) ? 0 : curvePoints[i].getDistanceFrom(curvePoints[i - 1]));
		totalDistance += curveDis[i];
	}
}

Vector2d Bezier::pointAt(double t)
{
	Vector2d c;
	int n = points.size() - 1;
	for (int i = 0; i <= n; i++) 
	{
		double b = bernstein(i, n, t);
		c.X += points[i].X * b;
		c.Y += points[i].Y * b;
	}
	return c;
}

// Returns the points along the curve of the Bezier curve.
std::vector<Vector2d> Bezier::getCurvePoint() { return curvePoints; }

// Returns the distances between a point of the curve and the last point.
std::vector<double> Bezier::getCurveDistances() { return curveDis; }

// Returns the number of points along the curve.
int Bezier::getCurvesCount() { return ncurve; }

// Returns the total distances of this Bezier curve.
double Bezier::TotalDistance() { return totalDistance; }