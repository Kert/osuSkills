#include "patterns.h"
#include "utils.h"
#include <algorithm>

using namespace std;

void AnalyzePatterns(Beatmap &beatmap)
{
	/*for (int i = 1; i < beatmap.hitObjects.size(); i++)
	{
		if (beatmap.hitObjects[i].type != HitObjectType::Normal)
			continue;
		DetectStacks(beatmap, i);
	}

	for (int i = 1; i < beatmap.hitObjects.size(); i++)
	{
		if (beatmap.hitObjects[i].type != HitObjectType::Normal)
			continue;
		DetectCompressedStream(beatmap, i);
	}*/
}

void DetectStacks(Beatmap &beatmap, int &i)
{
	/*double distance = -1;
	int currTime = beatmap.hitObjects[i].time,
		prevTime = beatmap.hitObjects[i - 1].time;
	bool stack = true, isCircle = true;
	int objects = 0;

	// If the distance is such that you can leave the cursor unmoved, it's a stack
	// Note: If the stack is long enough (a stream), it can give a false positive
	for (; (i > 0) && (i < beatmap.hitObjects.size()) && stack; i++)
	{
		distance = beatmap.hitObjects[i].pos.getDistanceFrom(beatmap.hitObjects[i - 1].pos);
		isCircle = (beatmap.hitObjects[i].type == HitObjectType::Normal);
		stack = (distance == 0) && isCircle;
		objects++;
	}

	if (objects > 1)
		cout << "Stack at " << prevTime << "  (" << msToTimeString(prevTime) << ")" << endl;*/
}

void DetectCompressedStream(Beatmap &beatmap, int &i)
{
	

	/*double distance = -1;
	int initTime = beatmap.hitObjects[i - 1].time;
	bool stream = true, isCircle = true, overlap = true;
	int objects = 0;

	// Similiar to stack detection + check for overlapping circles instead of 0 distance
	for (; (i > 0) && (i < beatmap.hitObjects.size()) && stream; i++) // find the end of the stack
	{
		distance = beatmap.hitObjects[i].pos.getDistanceFrom(beatmap.hitObjects[i - 1].pos);
		isCircle = (beatmap.hitObjects[i].type == HitObjectType::Normal);
		overlap = (distance < CS2px(beatmap.cs));
		stream = (overlap && isCircle);
		objects++;
	}

	i--;
	if (objects > 1)
		cout << "Compressed stream at " << initTime << "  (" << objects << " objects from "
		<< msToTimeString(initTime) << " to " << msToTimeString(beatmap.hitObjects[i - 1].time) << ")" << endl;*/
}

void FindPatterns(Beatmap &beatmap)
{
	/*double oldangle = 0;
	int sq = 0;
	double sqdist = 0, oldsqdist = 0;
	for (int i = 0; i + 2 < beatmap.aimPoints.size(); i++)
	{
		double angle = beatmap.angles[i];
		double absval = oldangle - angle;
		//if((absval > 60 && absval < 120) || (absval > 240 && absval < 300))
		//
		//		if((absval > 200 && absval < 240) || (absval > 100 && absval < 140) || (absval > 18 && absval < 60))
		//		{
		//			if(beatmap.aimPoints[i].time < 50000)
		//				cout << "Hardly readable at " << msToTimeString(beatmap.aimPoints[i].time) << " angle " << oldangle - angle << endl;
		//		}

		// SQUARES
		if (abs(abs(angle) - 90) < 10)
		{
			if (beatmap.aimPoints[i].type == AIM_POINT_CIRCLE)
			{
				sq++;
				sqdist = beatmap.aimPoints[i].pos.getDistanceFrom(beatmap.aimPoints[i + 1].pos);

				if (sq == 2)
				{
					if (abs(sqdist - oldsqdist) < 10)
					{
						cout << "Square at " << msToTimeString(beatmap.aimPoints[i - 1].time) << endl;
						//sqdist = 0;
						sq = 0;
					}
					else sq = 1;
				}
				oldsqdist = sqdist;
			}
			else
			{
				sq = 0;
			}
		}
		else
		{
			sq = 0;
		}

		oldangle = angle;
	}*/
}

double getVelocity(HitObject &obj)
{
	double period = obj.endTime - obj.time;
	double length = obj.pixelLength;
	return length / period;
}

std::vector<std::tuple<int, int, int, int>> getPattern(std::vector<HitObject> &obj, int time, int index, double CS, unsigned int quant)
{
	HitObject* circle = &(obj[index]);
	std::vector<std::tuple<int, int, int, int>> points;

	// If it is a slider, then iterate through and find reaction points.
	if (IsHitObjectType(circle->type, SLIDER))
	{
		double curr_time = time, prev_time = time;

		double parts = circle->pixelLength / (double)CS2px(CS); // divide the slider into parts = CS diameter
		double timeDelta = (double)(circle->endTime - circle->time) / parts; // ms between each part
		Vector2d pos = GetSliderPos(*circle, time);

		if (!BTWN(circle->time, curr_time, circle->endTime))
			curr_time = circle->time;

		// record the point
		points.push_back(getPointAt(*circle, (int) curr_time));
		curr_time -= timeDelta;

		// get 2 points before the current point which are at least a circle size apart
		while (curr_time >= circle->time && points.size() < quant)
		{
			double dist = 0;

			// find the next point that is at least a circle size apart
			while (dist < CS2px(CS) && curr_time >= circle->time && points.size() < quant)
			{
				// \TODO: make this more accurate
				Vector2d prevSliderPos = GetSliderPos(*circle, (int) prev_time);
				Vector2d currSliderPos = GetSliderPos(*circle, (int) curr_time);

				dist = prevSliderPos.getDistanceFrom(currSliderPos);
				curr_time -= timeDelta;
			}

			// record the point
			Vector2d prevPos = Vector2d(std::get<0>(points[points.size() - 1]), std::get<1>(points[points.size() - 1]));
			Vector2d newPos = GetSliderPos(*circle, (int)(curr_time + timeDelta));
			if (prevPos.getDistanceFrom(newPos) >= CS2px(CS))
			{
				points.push_back(getPointAt(obj[index], (int)(curr_time + timeDelta)));

				prev_time = curr_time;
				//curr_time -= timeDelta;
			}
		}
	}
	else
	{
		Vector2d pos = circle->pos;
		points.push_back(std::tuple<int, int, int, int>(pos.X, pos.Y, circle->time, 0));
	}

	// check if we have the required amount of points
	if (points.size() < quant && index > 0)
	{
		// if not, go to the previous hitobject
		std::vector<std::tuple<int, int, int, int>> pattern = getPattern(obj, obj[index - 1].endTime, index - 1, CS, quant - points.size());

		points.insert(points.end(),
			std::make_move_iterator(pattern.begin()),
			std::make_move_iterator(pattern.end()));

		//assert(points.size() != 0);
		return points;
	}
	else
	{
		//assert(points.size() != 0);
		return points;
	}
}

int getHitcircleAt(std::vector<HitObject> &hitcircles, int time)
{
	int start = 0;
	int end = hitcircles.size() - 2;
	int mid;

	while (start <= end)
	{
		mid = (start + end) / 2;
		if (BTWN(hitcircles[mid].time, time, hitcircles[mid + 1].time - 1))
			return mid;
		else if (time < hitcircles[mid].time)
			end = mid - 1;
		else start = mid + 1;
	}

	return -1;
}


std::pair<int, int> getVisiblityTimes(HitObject &obj, double AR, bool hidden, double opacityStart, double opacityEnd)
{
	double preampTime = obj.time - AR2ms(AR);	// Time when the AR goes into effect
	std::pair<int, int> times;

	if (hidden)
	{
		double fadeinDuration = 0.4*AR2ms(AR);				// how long the fadein period is
		double fadeinTimeEnd = preampTime + fadeinDuration; // When it is fully faded in

		times.first = (int) getValue(preampTime, fadeinTimeEnd, opacityStart);


		// If it's a slider, then the fade out period lasts from when it's fadedin to
		// 70% to the time it the slider ends
		if (IsHitObjectType(obj.type, SLIDER))
		{
			double fadeoutDuration = (obj.endTime - fadeinTimeEnd); // how long the fadeout period is
			double fadeoutTimeEnd = fadeinTimeEnd + fadeoutDuration;		   // When it is fully faded out
			times.second = (int) getValue(fadeinTimeEnd, fadeoutTimeEnd, 1.0 - opacityEnd);

			return times;
		}
		else
		{
			double fadeoutDuration = 0.7*(obj.time - fadeinTimeEnd);		// how long the fadeout period is
			double fadeoutTimeEnd = fadeinTimeEnd + fadeoutDuration;	// When it is fully faded out
			times.second = (int) getValue(fadeinTimeEnd, fadeoutTimeEnd, 1.0 - opacityStart); // <-- no this is not a mistake :D

			return times;
		}
	}
	else
	{
		double fadeinDuration = std::min(AR2ms(AR), 400);		// how long the fadein period is
		double fadeinTimeEnd = preampTime + fadeinDuration; // When it is fully faded in

		// Fadein period always lasts from preamp time to 400 ms after preamp time or
		// when the object needs to be hit, which ever is smaller
		times.first = (int) getValue(preampTime, fadeinTimeEnd, opacityStart);

		// If it is during the slider hold period, then it's fully visible.
		// Otherwise, it's not visible anymore.
		if (IsHitObjectType(obj.type, SLIDER))
		{
			times.second = obj.endTime;
			return times;
		}
		else
		{
			times.second = obj.time;
			return times;
		}
	}
}