#include "reaction.h"
#include "utils.h"
#include "patterns.h"
#include "tweakvars.h"
#include <algorithm>

double PatternReq(TIMING& p1, TIMING& p2, TIMING& p3, double CSpx)
{
	const int XPOS = 0;
	const int YPOS = 1;
	const int TIME = 2;

	Vector2d point1 = Vector2d(p1.pos.X, p1.pos.Y);
	Vector2d point2 = Vector2d(p2.pos.X, p2.pos.Y);
	Vector2d point3 = Vector2d(p3.pos.X, p3.pos.Y);

	double dist_12 = point1.getDistanceFrom(point2);
	double dist_23 = point2.getDistanceFrom(point3);
	double dist = dist_12 + dist_23;

	double angle = GetAngle(point1,point2,point3);

	double time = abs(p3.time - p1.time);
	time = (time < 16) ? 16 : time; // 16ms @ 60 FPS

	// 2 * _CSpx = 1 diameter od CS since CS here is being calculated in terms of radius
	return time / ((dist / (2 * CSpx))*((M_PI - angle) / M_PI));  
}

bool isHitobjectAt(std::vector<HitObject>& _hitobjects, long _prevTime, long _currTime)
{
	int i = FindHitobjectAt(_hitobjects, _currTime);
	if (BTWN(_prevTime, _hitobjects[i].time, _currTime)) return true;				  // a normal note
	if (BTWN(_hitobjects[i].time, _currTime, _hitobjects[i].endTime)) return true;  // a hold note

	return false;
}


TIMING getNextTickPoint(std::vector<HitObject>& _hitobjects, long* _time)
{
	TIMING tickPoint;
	unsigned int i = FindHitobjectAt(_hitobjects, *_time, true);
	
	// if we reached the end, make timing.data = -1
	if (i >= _hitobjects.size() - 1) return TIMING({0, -1}); 

	// if the time is between 2 hitobjects, return the start of the next hitobject
	if (!isHitobjectAt(_hitobjects, *_time - 1, *_time))
	{
		*_time = _hitobjects[i + 1].time;
		Vector2d pos = _hitobjects[i + 1].pos;

		tickPoint.pos = Vector2d(pos.X, pos.Y);
		tickPoint.time = *_time;
		tickPoint.data = 0;
		tickPoint.press = false;
		return tickPoint;
	}
	else
	{
		// if it is a slider, return the next closest tick
		if (IsHitObjectType(_hitobjects[i].type, SLIDER))
		{
			std::vector<int> ticks = _hitobjects[i].ticks;
			for (unsigned int tick = 1; tick < ticks.size(); tick++)
			{
				if (BTWN(ticks[tick - 1], *_time, ticks[tick]))
				{
					*_time = ticks[tick];
					Vector2d pos = GetSliderPos(_hitobjects[i], ticks[tick]);

					tickPoint.pos = Vector2d(pos.X, pos.Y);
					tickPoint.time = *_time;
					tickPoint.data = 0;
					tickPoint.press = true;
					return tickPoint;
				}
			}

			// else slider had no second tick
			*_time = _hitobjects[i + 1].time;
			Vector2d pos = _hitobjects[i + 1].pos;

			tickPoint.pos = Vector2d(pos.X, pos.Y);
			tickPoint.time = *_time;
			tickPoint.data = 0;
			tickPoint.press = false;
			return tickPoint;
		}
		else // if it is a regular hitobject, return the start of the next hitobject
		{
			*_time = _hitobjects[i + 1].time;
			Vector2d pos = _hitobjects[i + 1].pos;

			tickPoint.pos = Vector2d(pos.X, pos.Y);
			tickPoint.time = *_time;
			tickPoint.data = 0;
			tickPoint.press = false;
			return tickPoint;
		}
	}
}

// Original model can be found at https://www.desmos.com/calculator/k9r2uipjfq
double Pattern2Reaction(TIMING& p1, TIMING& p2, TIMING& p3, double ARms, double CSpx)
{
	double damping = GetVar("Reaction", "PatternDamping");		// opposite of sensitivity; how much the patterns' influence is damped
	double curveSteepness = /*(300.0 / (ARms + 250.)) **/ damping;
	double patReq = PatternReq(p1, p2, p3, CSpx);

	return ARms - ARms*std::exp(-curveSteepness*patReq) /*+ curveSteepness*sqrt(curveSteepness*patReq)*/;
}

double react2Skill(double _timeToReact)
{
	// Original model can be found at https://www.desmos.com/calculator/lg2jqyesnu
	double a = pow(2.0, log(78608.0 / 15625.0) / log(34.0 / 25.0))*pow(125.0, log(68.0 / 25.0) / log(34.0 / 25.0));
	double b = log(2.0) / (log(2.0) - 2.0*log(5.0) + log(17.0));
	return a / pow(_timeToReact, b);
}

double getReactionSkillAt(std::vector<TIMING>& targetpoints, TIMING& targetpoint, std::vector<HitObject>& hitobjects, double CS, double AR, bool hidden)
{
	double timeToReact = 0.0;
	double FadeInReactReq = GetVar("Reaction", "FadeinPercent"); // players can react once the note is 10% faded in
	unsigned int index = FindTimingAt(targetpoints, targetpoint.time);

	if (index >= targetpoints.size() - 2)
	{
		timeToReact = AR2ms(AR);
	}
	else if (index < 3)
	{
		std::pair<int, int> visibilityTimes = getVisiblityTimes(hitobjects[0], AR, hidden, FadeInReactReq, 1.0);
		timeToReact = hitobjects[0].time - visibilityTimes.first;
	}
	else
	{
		TIMING t1 = targetpoints[index];
		TIMING t2 = targetpoints[index + 1];
		TIMING t3 = targetpoints[index + 2];

		double timeSinceStart = 0;

		if (targetpoint.press == true)
			timeSinceStart = abs(targetpoint.time - hitobjects[targetpoint.key].time);  // Time since started holding slider

		std::pair<int, int> visibilityTimes = getVisiblityTimes(hitobjects[0], AR, hidden, FadeInReactReq, 1.0);
		double actualARTime = (hitobjects[0].time - visibilityTimes.first) + timeSinceStart;

		double result = Pattern2Reaction(t1, t2, t3, actualARTime, CS2px(CS));
		timeToReact = sqrt(timeToReact*timeToReact + result*result);
	}

	//return 28.0*pow(react2Skill(timeToReact), 0.524); // to fit it on scale compared to other skills (v1)
	return GetVar("Reaction", "VerScale")*pow(react2Skill(timeToReact), GetVar("Reaction", "CurveExp")); // to fit it on scale compared to other skills (v2)
}

void CalculateReaction(Beatmap& beatmap, bool hidden)
{
	double max = 0;
	double avg = 0;
	double weight = GetVar("Reaction", "AvgWeighting");

	int lastObj = beatmap.hitObjects[beatmap.hitObjects.size() - 1].time;

	for (auto tick : beatmap.targetPoints)
	{
		double val = getReactionSkillAt(beatmap.targetPoints, tick, beatmap.hitObjects, beatmap.cs, beatmap.ar, hidden);

		if (val > max)			max = val;
		if (val > max / 2.0)	avg = weight*val + (1 - weight)*avg;
	}

	beatmap.skills.reaction = (max + avg) / 2.0;
}