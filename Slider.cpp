#include "Slider.h"
#include "utils.h"

#include <iostream>

Vector2d GetSliderPos(HitObject hitObject, int time)
{
	if (IsHitObjectType(hitObject.type, HitObjectType::SLIDER))
	{
		// convert time to percent
		double percent;
		if (time <= hitObject.time)
			percent = 0;
		else if (time > hitObject.endTime)
			percent = 1;
		else
		{
			int timeLength = (time - hitObject.time);
			int repeatsDone = (int)(timeLength / hitObject.toRepeatTime);
			percent = (timeLength - hitObject.toRepeatTime * repeatsDone) / (double)hitObject.toRepeatTime;
			if (repeatsDone % 2)
				percent = 1 - percent; // it's going back
		}
		
		// get the points
		int ncurve = hitObject.ncurve;
		double indexF = percent * ncurve;
		int index = (int)indexF;

		if (index >= hitObject.ncurve)
		{
			Vector2d poi = hitObject.lerpPoints[ncurve];
			return Vector2d(poi.X, poi.Y);
		}
		else
		{
			Vector2d poi = hitObject.lerpPoints[index];
			Vector2d poi2 = hitObject.lerpPoints[index + 1];
			double t2 = indexF - index;
			return Vector2d(lerp(poi.X, poi2.X, t2), lerp(poi.Y, poi2.Y, t2));
		}
	}
	else
		return Vector2d(-1, -1);
}


std::tuple<int, int, int, int> getPointAt(HitObject &obj, int time)
{
	Vector2d pos = obj.pos;
	int timeSinceStart = 0;

	if (IsHitObjectType(obj.type, SLIDER))
	{
		pos = GetSliderPos(obj, time);
		timeSinceStart = time - obj.time;
	}

	return std::tuple<int, int, int, int>(pos.X, pos.Y, obj.time + timeSinceStart, timeSinceStart);
}

void ApproximateSliderPoints(Beatmap &beatmap)
{
	std::vector<double> timingPointOffsets, beatLengths;
	double base;
	for (unsigned i = 0; i < beatmap.timingPoints.size(); i++)
	{
		timingPointOffsets.push_back(beatmap.timingPoints[i].offset);

		if (beatmap.timingPoints[i].inherited)
		{
			beatLengths.push_back(base);
		}
		else
		{
			beatLengths.push_back(beatmap.timingPoints[i].beatInterval);
			base = beatmap.timingPoints[i].beatInterval;
		}	
	}

	int i = 0;
	for (auto &hitObject : beatmap.hitObjects)
	{
		i++;
		if (IsHitObjectType(hitObject.type, HitObjectType::SLIDER))
		{
			int timingPointIndex = getValuePos(timingPointOffsets, hitObject.time, true);

			hitObject.toRepeatTime = static_cast<int>(round((double)(((-600.0 / beatmap.timingPoints[timingPointIndex].bpm) * hitObject.pixelLength * beatmap.timingPoints[timingPointIndex].sm) / (100.0 * beatmap.sm))));
			hitObject.endTime = hitObject.time + hitObject.toRepeatTime * hitObject.repeat;
			
			// Saving the time of repeats
			if (hitObject.repeat > 1)
			{
				for (int i = hitObject.time; i < hitObject.endTime; i += hitObject.toRepeatTime)
				{
					if (i > hitObject.endTime)
						break;
					hitObject.repeatTimes.push_back(i);
				}
			}

			int tickInterval = static_cast<int>(beatLengths[timingPointIndex] / beatmap.st);
			const int errInterval = 10;
			int j = 1;

			for (int i = hitObject.time + tickInterval; i < (hitObject.endTime - errInterval); i += tickInterval)
			{
				if (i > hitObject.endTime) break;
				
				int tickTime = hitObject.time + static_cast<int>(tickInterval * j);
				if (tickTime < 0) break;
				
				hitObject.ticks.push_back(tickTime);
				j++;
			}

			// If the slider starts and ends in less than 100ms and has no ticks to allow a sliderbreak, then make it a short generic slider
			if ((abs(hitObject.endTime - hitObject.time) < 100) && (hitObject.ticks.size() == 0))
			{
				HitObject hitObjectNew;
				
				hitObjectNew.curves = std::vector<Vector2d>({ Vector2d(hitObject.pos.X, hitObject.pos.Y), Vector2d(hitObject.pos.X + tickInterval / beatmap.st, hitObject.pos.Y + tickInterval / beatmap.st) });
				hitObjectNew.pos = hitObject.pos;
				hitObjectNew.type = hitObject.type;
				hitObjectNew.time = hitObject.time;
				hitObjectNew.endTime = hitObject.time + 101;
				hitObjectNew.toRepeatTime = hitObject.time + 101;
				hitObjectNew.repeat = 1;
				hitObjectNew.pixelLength = 100;
				hitObjectNew.curveType = 'L';

				Slider(hitObjectNew, true);
				hitObject = hitObjectNew;
				continue;
			}
		}
		else
		{
			hitObject.endTime = hitObject.time;
		}
	}
}

Slider::Slider(){}

Slider::Slider(HitObject hitObject, bool line)
{
	std::vector<Bezier> beziers;

	// Beziers: splits points into different Beziers if has the same points (red points)
	// a b c - c d - d e f g
	// Lines: generate a new curve for each sequential pair
	// ab  bc  cd  de  ef  fg
	int controlPoints = hitObject.curves.size() + 1;
	std::vector<Vector2d> points;  // temporary list of points to separate different Bezier curves
	Vector2d lastPoi(-1, -1);
	
	for (unsigned i = 0; i < hitObject.curves.size(); i++)
	{
		sliderX.push_back(hitObject.curves[i].X);
		sliderY.push_back(hitObject.curves[i].Y);
	}

	x = hitObject.pos.X;
	y = hitObject.pos.Y;
	
	for (int i = 0; i < controlPoints; i++)
	{
		Vector2d tpoi = Vector2d(getX(i), getY(i));
		if (line) 
		{
			if (lastPoi != Vector2d(-1, -1))
			{
				points.push_back(tpoi);
				beziers.push_back(Bezier(points));
				points.clear();
			}
		}
		else if ((lastPoi != Vector2d(-1, -1)) && (tpoi == lastPoi))
		{
			if (points.size() >= 2)
				beziers.push_back(Bezier(points));
			points.clear();
		}
		points.push_back(tpoi);
		lastPoi = tpoi;
	}

	if (line || points.size() < 2) 
	{
		// trying to continue Bezier with less than 2 points
		// probably ending on a red point, just ignore it
	}
	else 
	{
		beziers.push_back(Bezier(points));
		points.clear();
	}

	init(beziers, hitObject);
}

void Slider::init(std::vector<Bezier> curvesList, HitObject hitObject)
{
	// now try to creates points the are equidistant to each other
	ncurve = (int)(hitObject.pixelLength / CURVE_POINTS_SEPERATION);
	curve.resize(ncurve + 1);

	// if the slider has no curve points, force one in 
	// a hitobject that the player holds must have at least one point
	if (curvesList.size() == 0)
	{
		curvesList.push_back(Bezier({ hitObject.pos }));
		hitObject.endPoint = hitObject.pos;
	}

	double distanceAt = 0;
	unsigned curveCounter = 0;
	int curPoint = 0;
	Bezier curCurve = curvesList[curveCounter++];
	Vector2d lastCurve = curCurve.getCurvePoint()[0];
	double lastDistanceAt = 0;

	// length of Curve should equal pixel length (in 640x480)
	double pixelLength = hitObject.pixelLength;

	// for each distance, try to get in between the two points that are between it
	for (int i = 0; i < ncurve + 1; i++) 
	{
		int prefDistance = (int)(i * pixelLength / ncurve);
		while (distanceAt < prefDistance)
		{
			lastDistanceAt = distanceAt;
			lastCurve = curCurve.getCurvePoint()[curPoint];
			curPoint++;

			if (curPoint >= curCurve.getCurvesCount())
			{
				if (curveCounter < curvesList.size())
				{
					curCurve = curvesList[curveCounter++];
					curPoint = 0;
				}
				else 
				{
					curPoint = curCurve.getCurvesCount() - 1;
					
					// out of points even though the preferred distance hasn't been reached
					if (lastDistanceAt == distanceAt) break;	
				}
			}
			distanceAt += curCurve.getCurveDistances()[curPoint];
		}
		Vector2d thisCurve = curCurve.getCurvePoint()[curPoint];

		// interpolate the point between the two closest distances
		if (distanceAt - lastDistanceAt > 1) 
		{
			double t = (prefDistance - lastDistanceAt) / (distanceAt - lastDistanceAt);
			curve[i] = Vector2d(lerp(lastCurve.X, thisCurve.X, t), lerp(lastCurve.Y, thisCurve.Y, t));
		}
		else
			curve[i] = thisCurve;
	}
}

double Slider::getEndAngle() { return endAngle; }
double Slider::getStartAngle() { return startAngle; }


double Slider::getX(int i) { return (i == 0) ? x : sliderX[i - 1]; }
double Slider::getY(int i) { return (i == 0) ? y : sliderY[i - 1]; }
