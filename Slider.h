#ifndef _slider_h_
#define _slider_h_

#include <vector>
#include "globals.h"
#include "Bezier.h"

void ApproximateSliderPoints(Beatmap &beatmap);
Vector2d GetSliderPos(HitObject hitObject, int time);
std::tuple<int, int, int, int> getPointAt(HitObject &obj, int time);

/**
* Representation of Bezier curve with equidistant points (LinearBezier).
* http://pomax.github.io/bezierinfo/#tracing
*
* @author fluddokt (https://github.com/fluddokt)
* @Porter abraker
*/

class Slider
{
	public:
		
		/**
		* Constructor.
		* @param hitObject the associated HitObject
		* @param color the color of this curve
		* @param line whether a new curve should be generated for each sequential pair
		*/
		Slider(HitObject hitObject, bool line);

		// Dummy constructor
		Slider();
		/**
		* Initialize the curve points with equal distance.
		* Must be called by inherited classes.
		* @param curvesList a list of curves to join
		*/
		void init(std::vector<Bezier> curvesList, HitObject hitObject);

		double getEndAngle();
		double getStartAngle();

		/**
		* Returns the scaled x coordinate of the control point at index i.
		* @param i the control point index
		*/
		double getX(int i);

		/**
		* Returns the scaled y coordinate of the control point at index i.
		* @param i the control point index
		*/
		double getY(int i);

		/** Points along the curve (set by inherited classes). */
		std::vector<Vector2d> curve;
		/** The number of points along the curve. */
		int ncurve;

protected:
		/** Points generated along the curve should be spaced this far apart. */
		const int CURVE_POINTS_SEPERATION = 5;
		
		/** The angles of the first and last control points for drawing. */
		double startAngle, endAngle;
		
		/** The scaled starting x, y coordinates. */
		double x, y;

		/** The scaled slider x, y coordinate lists. */
		std::vector<double> sliderX, sliderY;

};

#endif