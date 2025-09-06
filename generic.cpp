#include "generic.h"

#include <climits>
#include "utils.h"
#include "strains.h"
#include "Slider.h"
#include "CircumscribedCircle.h"

using namespace std;

void PrepareAimData(Beatmap &beatmap)
{
	CalculateMovementData(beatmap);
	GatherTargetPoints(beatmap);
	GatherAimPoints(beatmap);
	CalculateAngles(beatmap);
}

void PrepareTapData(Beatmap &beatmap)
{
	CalculatePressIntervals(beatmap);
	GatherTapPatterns(beatmap);
}

void PrepareTimingPoints(Beatmap &beatmap)
{
	// Get min and maxbpm
	beatmap.bpmMin = 10000;
	beatmap.bpmMax = 0;
	double BPM = 0;
	double SliderMult = -100;
	double oldbeat = -100;

	TimingPoint* TP;
	for (unsigned i = 0; i < beatmap.timingPoints.size(); i++)
	{
		TP = &beatmap.timingPoints[i];
		if (TP->inherited)
		{
			if (TP->beatInterval <= 0)
			{
				SliderMult = TP->beatInterval;
				oldbeat = TP->beatInterval;
			}
			else
				SliderMult = oldbeat;
		}
		else
		{
			SliderMult = -100;
			BPM = 60000 / TP->beatInterval;
			if (beatmap.bpmMin > BPM)
				beatmap.bpmMin = BPM;
			if (beatmap.bpmMax < BPM)
				beatmap.bpmMax = BPM;			
		}
		TP->bpm = BPM;
		TP->sm = SliderMult;
	}
}

void CalculatePressIntervals(Beatmap &beatmap)
{
	int previousTime = -1;
	for(auto &hitObj : beatmap.hitObjects)
	{
		if(IsHitObjectType(hitObj.type, HitObjectType::Normal) || IsHitObjectType(hitObj.type, HitObjectType::SLIDER))
		{
			if(previousTime != -1)
				beatmap.pressIntervals.push_back(hitObj.time - previousTime);
			previousTime = hitObj.time;
		}
	}
}

void CalculateMovementData(Beatmap &beatmap)
{
	Vector2d previousPos;
	int previousTime = -1;

	for (unsigned i = 0; i < beatmap.hitObjects.size(); i++)
	{
		if ((IsHitObjectType(beatmap.hitObjects[i].type, HitObjectType::Normal) || IsHitObjectType(beatmap.hitObjects[i].type, HitObjectType::SLIDER)) &&
		previousTime != -1)
		{
			double distance = beatmap.hitObjects[i].pos.getDistanceFrom(previousPos);
			double radSubtract = 2 * CS2px(beatmap.cs);
			double interval = beatmap.hitObjects[i].time - previousTime;
			if(distance >= radSubtract)
				distance -= radSubtract;
			else
				distance /= 2;
			double speed = distance / interval;
			beatmap.distances.push_back(distance);
			double distX = beatmap.hitObjects[i].pos.X - previousPos.X;
			beatmap.velocities.X.push_back(distX / interval);
			double distY = beatmap.hitObjects[i].pos.Y - previousPos.Y;
			beatmap.velocities.Y.push_back(distY / interval);
		}
		if(IsHitObjectType(beatmap.hitObjects[i].type, HitObjectType::Normal))
		{
			previousPos = beatmap.hitObjects[i].pos;
			previousTime = beatmap.hitObjects[i].time;
		}
		else if(IsHitObjectType(beatmap.hitObjects[i].type, HitObjectType::SLIDER))
		{
			previousPos = beatmap.hitObjects[i].pos;
			previousTime = beatmap.hitObjects[i].time;
		}
	}
	
	// Calculate velocity changes
	double oldvelX = 0, oldvelY = 0;
	for (unsigned i = 0; i < beatmap.velocities.X.size(); i++)
	{
		double velX = beatmap.velocities.X[i];
		double velY = beatmap.velocities.Y[i];
		if (i)
		{
			beatmap.velocities.Xchange.push_back(velX - oldvelX);
			beatmap.velocities.Ychange.push_back(velY - oldvelY);
		}
		oldvelX = velX;
		oldvelY = velY;
	}
}

void GatherTapPatterns(Beatmap &beatmap)
{
	std::map<int, std::vector<std::vector<int>>> sections;
	// Get sections of 1/2 1/4 e.t.c.
	
	int old = 0;
	std::vector<int> tmp;
	int i = 0;
	std::set<int> uniq;
	int const OFFSET_MAX_DISPLACEMENT = 2;
	for(auto interval : beatmap.pressIntervals)
	{
		auto it = uniq.find(interval);
		if(it == uniq.end())
		{
			bool found = false;
			for(int p = (interval - OFFSET_MAX_DISPLACEMENT); p <= (interval + OFFSET_MAX_DISPLACEMENT); p++)
			{
				auto it2 = uniq.find(p);
				if(it2 != uniq.end())
				{
					interval = p;
					found = true;
					break;
				}
			}
			if(!found)
			{
				uniq.insert(interval);
				sections[interval] = std::vector<std::vector<int>>();
				beatmap.streams[interval] = std::vector<std::vector<int>>();
				beatmap.bursts[interval] = std::vector<std::vector<int>>();
			}
		}

		if(abs(interval - old) > OFFSET_MAX_DISPLACEMENT)
		{
			// disregard everything shorter than a triple
			if(tmp.size() > 1)
			{
				sections[old].push_back(tmp);
				if(tmp.size() > 6)
					beatmap.streams[old].push_back(tmp);
				else
					beatmap.bursts[old].push_back(tmp);
			}
			tmp.clear();
		}
		tmp.push_back(beatmap.hitObjects[i].time);
		old = interval;
		i++;
	}
	if(tmp.size() > 0)
	{
		// disregard everything shorter than a triple
		if(tmp.size() > 1)
		{
			sections[old].push_back(tmp);
			if(tmp.size() > 6)
				beatmap.streams[old].push_back(tmp);
			else
				beatmap.bursts[old].push_back(tmp);
		}
	}
// Log unique intervals
//	for(auto p : uniq)
//		cout << p << " ";
		
	// Log sections of 1/2 1/4 e.t.c.
//	i = 0;
//	int all = 0;
//	for(auto section : sections)
//	{
//		cout << "Section of " << msToBPM(section.first) << "bpm (" << section.first << "ms) groups:" << endl;
//		cout << endl;
//		int groups = 0, inGroup = 0, total = 0;
//		for(auto j : section.second)
//		{
//			inGroup = 0;
//			for(auto k : j)
//			{
//				cout << k << " ";
//				inGroup++;
//				total++;
//			}
//			cout << "Group by " << inGroup+1 << endl;
//			groups++;
//		}
//		cout << groups << " groups with " << total << " objects total" << endl;
//		all+=total;
//		cout << endl;
//		i++;
//	}
//	
//	cout << "Objects in sections: " << all << ". In hitObjects: " << beatmap.pressIntervals.size() << endl;
//	
//	// Log streams
//	i = 0;
//	for(auto stream : beatmap.streams)
//	{
//		cout << "Streams at " << msToBPM(stream.first) << "bpm (" << stream.first << "ms):" << endl;
//		cout << endl;
//		int groups = 0, inGroup = 0, total = 0;
//		for(auto j : stream.second)
//		{
//			inGroup = 0;
//			for(auto k : j)
//			{
//				cout << k << " ";
//				inGroup++;
//				total++;
//			}
//			cout << "has " << inGroup+1 << " objects" << endl;
//			groups++;
//		}
//		cout << groups << " streams with " << total << " objects total" << endl;
//		all+=total;
//		cout << endl;
//		i++;
//	}
//	
//	// Log bursts
//	i = 0;
//	for(auto burst : beatmap.bursts)
//	{
//		cout << "Bursts at " << msToBPM(burst.first) << "bpm (" << burst.first << "ms):" << endl;
//		cout << endl;
//		int groups = 0, inGroup = 0, total = 0;
//		for(auto j : burst.second)
//		{
//			inGroup = 0;
//			for(auto k : j)
//			{
//				cout << k << " ";
//				inGroup++;
//				total++;
//			}
//			cout << "has " << inGroup+1 << " objects" << endl;
//			groups++;
//		}
//		cout << groups << " bursts with " << total << " objects total" << endl;
//		all+=total;
//		cout << endl;
//		i++;
//	}
}

void GatherTargetPoints(Beatmap &beatmap)
{
	TIMING targetPoint;
	int i = 0;
	int prev_time = INT_MIN;

	for (auto &hitObj : beatmap.hitObjects)
	{
		// Hack around objects that need to be hit impossibly fast
		if (abs(hitObj.time - prev_time) < 5) continue;  // filter out hitobjects that occur less than 5ms after another
		prev_time = hitObj.time;

		if (IsHitObjectType(hitObj.type, HitObjectType::Normal))
		{
			targetPoint.time = hitObj.time;
			targetPoint.pos = hitObj.pos;
			targetPoint.key = i;
			targetPoint.press = false;

			beatmap.targetPoints.push_back(targetPoint);
		}
		else if (IsHitObjectType(hitObj.type, HitObjectType::SLIDER))
		{
			for (auto &tick : hitObj.ticks)
			{
				targetPoint.time = tick;
				targetPoint.pos = GetSliderPos(hitObj, tick);
				targetPoint.key = i;
				targetPoint.press = true;

				beatmap.targetPoints.push_back(targetPoint);
			}
				
		}

		i++;
	}

	// Print aim points to file
	// std::ofstream file("targetpoints.txt");
	// if (file.is_open())
	// {
		// for (auto point : beatmap.targetPoints)
			// file << point.time << '\t' << point.pos.X << '\t' << point.pos.Y << endl;
	// }

	// file.close();
}

void GatherAimPoints(Beatmap &beatmap)
{
	for(auto &hitObj : beatmap.hitObjects)
	{
		if(IsHitObjectType(hitObj.type, HitObjectType::Normal))
		{
			beatmap.aimPoints.push_back({hitObj.time, hitObj.pos, AIM_POINT_CIRCLE});
		}
		else if(IsHitObjectType(hitObj.type, HitObjectType::SLIDER))
		{
			beatmap.aimPoints.push_back({hitObj.time, hitObj.pos, AIM_POINT_SLIDER});
//			beatmap.aimPoints.push_back({GetRepeat(hitObj), hitObj.pos, AIM_POINT_SLIDERREVERSE});
//			beatmap.aimPoints.push_back({hitObj.endTime, hitObj.endPoint, AIM_POINT_SLIDEREND});
			
			int endTime = GetLastTickTime(hitObj);
			Vector2d endPos = GetSliderPos(hitObj, endTime);

			// Don't add an end aim point for a very short slider
			if (hitObj.ticks.size() || hitObj.pos.getDistanceFrom(endPos) > 2 * CS2px(beatmap.cs))
				beatmap.aimPoints.push_back({ endTime, endPos, AIM_POINT_SLIDEREND });
		}
	}
	//// Print aim points to file
	//std::ofstream file("aimpoints.txt");
	//int i = 0;
	//if(file.is_open())
	//{
	//	for(auto point : beatmap.aimPoints)
	//	{
	//		file << point.time << '\t' << point.pos.X << '\t' << point.pos.Y << endl;
	//		i++;
	//		//cout << msToBPM(interval) << "bpm from " << interval << "ms" << endl;
	//	}
	//}
	//file.close(); 
}

void CalculateAngles(Beatmap &beatmap)
{
	for (unsigned i = 0; i + 2 < beatmap.aimPoints.size(); i++)
	{
		double angle = GetDirAngle(beatmap.aimPoints[i].pos, beatmap.aimPoints[i + 1].pos, beatmap.aimPoints[i + 2].pos);
		//		cout << msToTimeString(beatmap.aimPoints[i].time) << " pos1 " << beatmap.aimPoints[i].pos.X <<
		//		" " << beatmap.aimPoints[i].pos.Y << " pos2 " << beatmap.aimPoints[i+1].pos.X << ' ' << 
		//			beatmap.aimPoints[i+1].pos.Y << " pos3 " << beatmap.aimPoints[i+2].pos.X << ' ' << 
		//			beatmap.aimPoints[i+2].pos.Y << endl;
		beatmap.angles.push_back(angle);
	}

	int i = 0;
	double oldangle = beatmap.angles[0] - 2 * beatmap.angles[0];
	for (auto angle : beatmap.angles)
	{
		double bonus = 0;
		double absd = abs(angle);
		if (sign(angle) == sign(oldangle))
		{
			if (absd < 90)
				bonus = sin(DegToRad(absd) * 0.784 + 0.339837);
			else
				bonus = sin(DegToRad(absd));
		}
		else // when signs change
		{
			if (absd < 90)
				bonus = sin(DegToRad(absd) * 0.536 + 0.72972);
			else
				bonus = sin(DegToRad(absd)) / 2;
		}
		beatmap.angleBonuses.push_back(bonus);
		oldangle = angle;
		i++;
	}
}

void BakeSliderData(Beatmap &beatmap)
{
	for (auto &hitObject : beatmap.hitObjects)
	{
		if (IsHitObjectType(hitObject.type, HitObjectType::SLIDER))
		{
			switch (hitObject.curveType)
			{
				case 'B':
				{
					Slider slider(hitObject, false);
					hitObject.lerpPoints.resize(slider.curve.size());
					hitObject.lerpPoints = slider.curve;
					hitObject.ncurve = slider.ncurve;
					break;
				}
				case 'P':
				{
					if (hitObject.curves.size() == 2)
					{
						CircumscribedCircle circle(hitObject);
						hitObject.lerpPoints.resize(circle.curve.size());
						hitObject.lerpPoints = circle.curve;
						hitObject.ncurve = circle.ncurve;
					}
					else
					{
						Slider slider(hitObject, false);
						hitObject.lerpPoints.resize(slider.curve.size());
						hitObject.lerpPoints = slider.curve;
						hitObject.ncurve = slider.ncurve;
					}
					break;
				}
				case 'L': case 'C':
				{
					Slider slider(hitObject, true);
					hitObject.lerpPoints.resize(slider.curve.size());
					hitObject.lerpPoints = slider.curve;
					hitObject.ncurve = slider.ncurve;
					break;
				}
			}
			hitObject.endPoint = (hitObject.repeat % 2) ? hitObject.lerpPoints.back() : hitObject.lerpPoints.front();
		}
	}
}