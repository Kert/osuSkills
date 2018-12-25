#include "memory.h"
#include "globals.h"
#include "utils.h"
#include "mods.h"
#include "tweakvars.h"

double GetApproachRelativeSize(int time, int hitTime, double ar)
{
	if (hitTime < time) return 1;
	else if (hitTime - AR2ms(ar) > time) return 0;
	else
	{
		double diff = hitTime - time;
		double interval = AR2ms(ar);
		return 1 + 3 * ( diff / interval);
	}
}

bool IsObservableFrom(HitObject obj, int distance, Vector2d fromPos)
{
	double dist = obj.pos.getDistanceFrom(fromPos);
	// saving (distance/time) for spaced apart parts without followpoints
	if (dist < distance)
		return true;
	return false;
}

double CalculateMemory(Beatmap &beatmap)
{
	double totalMemPoints = 0;
	HitObject old = beatmap.hitObjects[0];
	int combo = 0; // combo counter
	
	for (int i = 1; i < (int)beatmap.hitObjects.size(); i++)
	{
		HitObject cur = beatmap.hitObjects[i];
		double memPoints = 0;
		int observableDist = 160;
		if (combo < 100)
			observableDist = 160;
		else if (combo < 200)
			observableDist = 120;
		else
			observableDist = 100;
		//observableDist += CS2px(beatmap.cs);
		
		double sliderBonusFactor = 1;
		if (IsHitObjectType(old.type, SLIDER))
			sliderBonusFactor = GetVar("Memory", "SliderBuff");
		
		bool observable = false;
		int helpPixels = 0; // it's easier to navigate when you see approaches / circle border (HD)
		for (int j = i - 1; j > 0; j--)
		{
			HitObject prev = beatmap.hitObjects[j];
			if (cur.time - prev.time > AR2ms(beatmap.ar))
				break;
			if (!HasMod(beatmap, HD))
			{
				double size = GetApproachRelativeSize(prev.endTime, cur.time, beatmap.ar);
				helpPixels = static_cast<int>(size * CS2px(beatmap.cs));
			}
			else
			{
				int observableTime = cur.time;
				observableTime = cur.time - static_cast<int>(static_cast<double>(AR2ms(beatmap.ar)) * 0.3); // hd dissapear interval
				if (prev.time > observableTime)
					continue; // dissapeared already
				helpPixels = CS2px(beatmap.cs); // we can see more of a circle than just it's center point
			}
			if (IsObservableFrom(cur, observableDist + helpPixels, prev.pos))
			{
				observable = true;
				break;
			}
		}

		if (!observable)
		{
			if (!HasMod(beatmap, HD))
			{
				double size = GetApproachRelativeSize(old.endTime, cur.time, beatmap.ar);
				helpPixels = static_cast<int>(size * CS2px(beatmap.cs));
			}
			else
			{
				helpPixels = CS2px(beatmap.cs); // we can see more of a circle than just it's center point
			}
			if (IsHitObjectType(cur.type, NewCombo) || IsHitObjectType(cur.type, ColourHax)) // only new combo changes
			{
				double dist = cur.pos.getDistanceFrom(old.endPoint);
				// saving (distance/time) for spaced apart parts without followpoints
				if (dist > observableDist + helpPixels)
					memPoints = sliderBonusFactor * (dist / (double)(cur.time - old.time));
			}
			else
			{
				double dist = cur.pos.getDistanceFrom(old.endPoint);
				// saving (distance/time) for spaced apart parts with followpoints
				// treat parts with followpoints as easier ones
				if (dist > observableDist + helpPixels)
					memPoints = sliderBonusFactor * GetVar("Memory", "FollowpointsNerf") * (dist / (double)(cur.time - old.time));
			}
		}

		// count combo
		if (IsHitObjectType(cur.type, Normal) || IsHitObjectType(cur.type, Spinner))
			combo++;
		else if (IsHitObjectType(cur.type, SLIDER))
			combo += cur.ticks.size() + 2;

		old = cur; // save previous object
		totalMemPoints += memPoints;
	}
	beatmap.skills.memory = totalMemPoints;
	beatmap.skills.memory = GetVar("Memory", "TotalMult") * pow(beatmap.skills.memory, GetVar("Memory", "TotalPow"));
	return beatmap.skills.memory;
}