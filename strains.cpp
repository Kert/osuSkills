#include "strains.h"
#include <algorithm>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "utils.h"
#include "tweakvars.h"

void GetAngleDecayFunc(Beatmap &beatmap, std::vector<double>& output);
void GetChaosDecayFunc(Beatmap &beatmap, std::vector<double>& output);
void GetPrecisionDecayFunc(Beatmap &beatmap, std::vector<double>& output);

void CalculateTapStrains(Beatmap &beatmap)
{
	int c = 0;
	double oldbonus;
	double strain = 0;
	for (auto interval : beatmap.pressIntervals)
	{
		if (c == 0)
		{
			if (interval >= GetVar("Stamina", "LargestInterval"))
				strain = 0; // maybe delete this
			else
			{
				strain = GetVar("Stamina", "Scale") / pow(interval, pow(interval, GetVar("Stamina", "Pow")) * GetVar("Stamina", "Mult"));
			}
			beatmap.tapStrains.push_back(strain);
		}
		else
		{
			if (interval >= GetVar("Stamina", "LargestInterval"))
				strain *= GetVar("Stamina", "DecayMax");
			else
			{
				if (interval <= 1)
				{
					continue;
				}
				strain = GetVar("Stamina", "Scale") / pow(interval, pow(interval, GetVar("Stamina", "Pow")) * GetVar("Stamina", "Mult"));
				//cout << "Strain for current: " << strain << endl;
				strain += oldbonus * GetVar("Stamina", "Decay");
				//cout << "New strain " << strain << ", bonus: " << oldbonus * SEQUENCE_MULTIPLIER << endl;
			}
			beatmap.tapStrains.push_back(strain);
		}
		oldbonus = strain;
		c++;
	}
}

double GetWeightedAimDistance(double distance)
{
	double distanceBonus = pow(1 + (distance * GetVar("Agility", "DistMult")), GetVar("Agility", "DistPow"));
	distanceBonus /= GetVar("Agility", "DistDivisor");
	double weighted = distance * distanceBonus;
	return weighted;
}

double GetWeightedAimTime(double time)
{
	double timeBonus;
	timeBonus = pow(time * GetVar("Agility", "TimeMult"), GetVar("Agility", "TimePow"));
	double weighted = time * timeBonus;
	return weighted;
}

void CalculateAimStrains(Beatmap &beatmap)
{
	double oldstrain = 0;
	for (int i = 0; i < (int)beatmap.aimPoints.size(); i++)
	{
		double strain = 0;
		if (i)
		{
			double distance = GetWeightedAimDistance(beatmap.aimPoints[i].pos.getDistanceFrom(beatmap.aimPoints[i - 1].pos));
			int interval = beatmap.aimPoints[i].time - beatmap.aimPoints[i - 1].time; 
			double time = GetWeightedAimTime(interval);
			double angleBonus = 1;
			if(i > 1)
				angleBonus = 1 + (GetVar("Agility", "AngleMult") * beatmap.angleBonuses[i - 2]);

			if (time > 0)
				strain = distance / time * angleBonus;
			else
			{
				std::cout << beatmap.name << " Agility strain calc: time == 0" << std::endl;
				continue;
			}

			// decrease weight for a slider/slider end
			if (beatmap.aimPoints[i].type == AIM_POINT_SLIDEREND || beatmap.aimPoints[i-1].type == AIM_POINT_SLIDEREND)
				strain *= GetVar("Agility", "SliderStrainDecay");

			oldstrain -= GetVar("Agility", "StrainDecay") * interval;
			if (oldstrain < 0)
				oldstrain = 0;

			strain += oldstrain;
		}
		beatmap.aimStrains.push_back(strain);
		oldstrain = strain;
	}
}

void GetAngleDecayFunc(Beatmap &beatmap, std::vector<double>& output)
{
	std::vector<double> angleSpeeds;
	for (unsigned i = 1; i < beatmap.aimPoints.size() - 1; i++)
	{
		Vector2d prevPos = beatmap.aimPoints.at(i - 1).pos,
				 currPos = beatmap.aimPoints.at(i).pos,
				 nextPos = beatmap.aimPoints.at(i + 1).pos;

		double timeDelta = beatmap.aimPoints.at(i + 1).time - beatmap.aimPoints.at(i - 1).time,
			   rad = GetDirAngle(prevPos, currPos, nextPos), 
			   angle = M_PI - rad;
		double radSpeed = angle / (timeDelta);

	/// Angles v3 [BEGIN]
		Vector2d prevPrevPos;
		if (i > 1)
			prevPrevPos = beatmap.aimPoints.at(i - 2).pos;
		else
			prevPrevPos = beatmap.aimPoints.at(i - 1).pos;

		const double equ = 1.07560910263708;
		if (angle < equ)
			radSpeed *= 1 + (0.5 / (1 + 40 * exp(-angle*4.5) + 0.5)) * 100;   // 1.5 more weight on 180 deg angles
		else
			radSpeed *= 1 + abs(sin(angle)) * 100;  // 2.0 more weight on 90 deg angles

		// get the midpoint of the 2 points surrounding the center point
		Vector2d currMidpoint = (prevPos + nextPos) / 2.0,
				 prevMidpoint = (prevPrevPos + currPos) / 2.0;

		// use that to get the vector from the center of the angle to the midpoint, aka the Normal of the angle.
		Vector2d currNormal = Vector2d(currPos.X - currMidpoint.X, currPos.Y - currMidpoint.Y),
			     prevNormal = Vector2d(prevPos.X - prevMidpoint.X, prevPos.Y - prevMidpoint.Y);

		// if true, then the angles are facing away from each other. Weight these 1.5 more
		if (isOppositeParity(currNormal.X, prevNormal.X) || isOppositeParity(currNormal.Y, prevNormal.Y))
			radSpeed *= 1.5 * 100;
	/// Angles v3 [END]

		angleSpeeds.push_back(radSpeed);
	}
	getDecayFunction(angleSpeeds, 0.9, output);
}

void GetChaosDecayFunc(Beatmap &beatmap, std::vector<double>& output)
{
	std::vector<double> chaosVals;
	for (unsigned i = 1; i < beatmap.aimPoints.size() - 1; i++)
	{
		double chaos = GetChaosAt(beatmap, i);
		//if (chaos == 0) chaos = 1;
		chaosVals.push_back(chaos);
	}
	getDecayFunction(chaosVals, 0.9, output);
}

void GetPrecisionDecayFunc(Beatmap &beatmap, std::vector<double>& output)
{
	std::vector<double> PrecisionVals;
	for (unsigned i = 1; i < beatmap.aimPoints.size() - 1; i++)
	{
		double humanTime = log2(GetNoteDistanceAt(beatmap, i, true) / (2 * CS2px(beatmap.cs)) + 1) * 5,
			   actualTime = beatmap.aimPoints.at(i + 1).time - beatmap.aimPoints.at(i).time,
			   precisionDiff = 0;

		if (humanTime == 0) // if no movement is involved
			precisionDiff = 0;
		else if (actualTime - humanTime < 0) // if it's impossible for human standards
			precisionDiff = INFINITY;
		else
		    precisionDiff = (1000.0 * 1000.0) / pow(actualTime - humanTime, 2);
		PrecisionVals.push_back(precisionDiff);
	}
	getDecayFunction(PrecisionVals, 0.9, output);

	std::vector<double> topWeights;
	getTopVals(output, 1000, topWeights);

	beatmap.skills.precision = getWeightedValue(topWeights, 0.99) * 0.5;
}

// TODO: Take account sliders
// TODO: Take account SV
// TODO: Take account reverse angles  [DONE] [TESTING NEEDED]
// TODO: Take account breaks
void CalculateAgilityStrains(Beatmap &beatmap)
{
	std::vector<double> angles, chaos, precision;
	std::vector<double> weightVals, weightFunc;

	GetAngleDecayFunc(beatmap, angles);
	GetChaosDecayFunc(beatmap, chaos);
	GetPrecisionDecayFunc(beatmap, precision);

	int size = std::min({(int)angles.size(), (int)chaos.size(), (int)precision.size()});
	for (int i = 0; i < size; i++)
	{
		//double weight = angles[i] * chaos[i] * beatmap.cs; // Agility v2 
		double weight = getMagnitude({ angles[i], chaos[i], precision[i] * 0.1 });  // Agility v3
		weightVals.push_back(weight);
	}
	getDecayFunction(weightVals, 0.9, weightFunc);

	for (unsigned i = 0; i < weightVals.size(); i++)
		beatmap.angleStrains.push_back(weightFunc[i]);

	std::vector<double> topWeights;
	getTopVals(weightVals, 1000, topWeights);

	beatmap.skills.agility = getWeightedValue(topWeights, 0.99);
}

void ClearStrains(Beatmap &beatmap)
{
	beatmap.tapStrains.clear();
	beatmap.aimStrains.clear();
}