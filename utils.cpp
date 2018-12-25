#include "utils.h"
#include <algorithm>

bool BTWN(double lss, double val, double gtr)
{
	return ((std::min(lss, gtr) <= val) && (val <= std::max(lss, gtr)));
}

int msToBPM(int ms)
{
	if (!ms) return 0;
	return 60 * 1000 / (ms * 4);
}

std::string msToTimeString(int ms)
{
	int min, sec, msec;
	min = ms / 60000;
	sec = ms - (min * 60000);
	msec = sec % 1000;
	sec = sec / 1000;
	return std::string(std::to_string(min) + ":" + std::to_string(sec) + ":" + std::to_string(msec));
}


bool IsHitObjectType(int Type, HitObjectType type)
{
	return (Type & type) > 0;
}

double BOUND(double _min, double _val, double _max)
{
	return std::min(std::max(_val, _min), _max);
}

// resolves % value between from min to max
// ex: 50% of somthing from 100 to 200 is 150
double getValue(double min, double max, double percent)
{
	return std::max(max, min) - (1.0 - percent)*(std::max(max, min) - std::min(max, min));
}

double getPercent(double _min, double _val, double _max)
{
	return 1.0 - ((_max - BOUND(_min, _val, _max)) / (_max - _min));
}

int GetLastTickTime(HitObject &hitObj)
{
	if (!hitObj.ticks.size())
	{
		if (hitObj.repeat > 1)
			return static_cast<int>(hitObj.endTime - (hitObj.endTime - hitObj.repeatTimes.back()) / 2.0);
		else
			return static_cast<int>(hitObj.endTime - (hitObj.endTime - hitObj.time) / 2.0);
	}
	else
		return static_cast<int>(hitObj.endTime - (hitObj.endTime - hitObj.ticks.back()) / 2.0);
}


// Gets the directional angle in degrees (-180 -> +180)
// Positive is counter-clock wise and negative is clock-wise
double GetDirAngle(Vector2d &a, Vector2d &b, Vector2d &c)
{
	Vector2d ab = { b.X - a.X, b.Y - a.Y };
	Vector2d cb = { b.X - c.X, b.Y - c.Y };

	double dot = (ab.X * cb.X + ab.Y * cb.Y); // dot product
	double cross = (ab.X * cb.Y - ab.Y * cb.X); // cross product

	double alpha = atan2(cross, dot);

	return alpha * 180.0 / M_PI;
}

// Returns the angle 3 points make in radians between 0 and pi
double GetAngle(Vector2d &a, Vector2d &b, Vector2d &c)
{
	return abs(DegToRad(GetDirAngle(a, b, c)));
}

double GetCircleOverlapPercentage(Beatmap &beatmap, HitObject &c1, HitObject &c2)
{
	double distance = c1.pos.getDistanceFrom(c2.pos);
	double radius = CS2px(beatmap.cs);
	double result = 1 - distance / (radius * 2);
	return (result < 0) ? 0 : result;
}

unsigned int FindHitobjectAt(std::vector<HitObject>& _hitobjects, long _time, bool _dir)
{
	unsigned int start = 0;
	unsigned int end = _hitobjects.size() - 2;
	unsigned int mid;

	while (start <= end)
	{
		mid = (start + end) / 2;

		// Between ends of a hold object
		if (BTWN(_hitobjects[mid].time, _time, _hitobjects[mid].endTime))
		{
			// Return next object if next object's timings overlap with this one
			if (BTWN(_hitobjects[mid + 1].time, _time, _hitobjects[mid + 1].endTime))
				return mid + 1;
			else
				return mid;
		}


		// Between some two objects
		if (BTWN(_hitobjects[mid].endTime, _time, _hitobjects[mid + 1].time))
			return mid + (long)_dir;

		if (_time < _hitobjects[mid].time)
			end = mid - 1;
		else
			start = mid + 1;
	}

	return _hitobjects.size() - 1;
}

double GetNoteDistanceAt(Beatmap &beatmap, int i, bool aimpoint)
{
	double distance = 0;
	Vector2d prevPos, currPos;

	if (aimpoint)
	{
		prevPos = beatmap.aimPoints.at(i - 1).pos,
		currPos = beatmap.aimPoints.at(i).pos;
	}
	else
	{
		prevPos = beatmap.hitObjects.at(i - 1).pos,
		currPos = beatmap.hitObjects.at(i).pos;
	}

	distance = currPos.getDistanceFrom(prevPos);
	if (distance < 0) distance = abs(distance); // distance is always positive
	
	distance -= CS2px(beatmap.cs);
	if (distance < 0) distance = 0; // if the notes are close enough to be hit in the same spot, then dist = 0

	return distance;
}

// numObj = num of objects to detect to the left and right
double GetChaosAt(Beatmap &beatmap, int i, int numObj)
{
	int initialTime = beatmap.aimPoints[i].time;
	int interval = AR2ms(beatmap.ar);
	const int timeToDisappear = 220;
	Vector2d avg = beatmap.aimPoints[i].pos;
	int objects = 1, numObjCount;

	// Earliest object visible (left border), Latest object visible (right border)
	numObjCount = numObj;
	for (int left = i - 1; left >= 0; left--) // left border
	{
		bool foundObject = (beatmap.aimPoints[left].time >= initialTime - 333) && (numObjCount-- != 0);
		if (foundObject)
		{
			avg += beatmap.aimPoints[left].pos;
			objects++;
		}
		else
			break;
	}

	numObjCount = numObj;
	for (unsigned right = initialTime + 1; right < beatmap.aimPoints.size(); right++) // right border
	{
		bool foundObject = (beatmap.aimPoints[right].time <= initialTime + 333) && (numObjCount-- != 0);
		if (foundObject)
		{
			avg += beatmap.aimPoints[right].pos;
			objects++;
		}
		else
			break;
	}
	avg /= objects;

	return (beatmap.aimPoints[i].pos.getDistanceFrom(avg) * 100.0) / 320.0;
}

int FindTimingAt(std::vector<TIMING>& _timings, double _time)
{
	int start = 0;
	int end = _timings.size() - 2;
	int mid;

	if (end < 0)
		return 0;

	while (start <= end)
	{
		mid = (start + end) / 2;

		if (BTWN(_timings[mid].time, _time, _timings[mid + 1].time))
			return mid + 1;

		if (_time < _timings[mid].time)
			end = mid - 1;
		else
			start = mid + 1;
	}

	if (_time < _timings[0].time)					return INT_MIN;
	if (_time > _timings[_timings.size() - 1].time) return INT_MAX;

	return (int) NAN;
}

int AR2ms(double ar) // converts AR value to ms visible before needed to click
{
	if (ar <= 5.00) return static_cast<int>(1800 - (120 * ar));
	else			return static_cast<int>(1950 - (150 * ar));
}

double ms2AR(int ms) // converts AR value to ms visible before needed to click
{
	if (ms >= 1200.0) return (1800 - ms) / 120.0;
	else			  return (1950 - ms) / 150.0;
}

double OD2ms(double od)
{
	return -6.0 * od + 79.5;
}

double ms2OD(double ms)
{
	return (79.5 - ms) / 6.0;
}

int CS2px(double cs) // converts CS value into circle radius in osu!pixels (640x480)
{
	return static_cast<int>(54.5 - (4.5 * cs));
}

double BpmSv2px(double bpm, double sv) // Converts bpm and SV to osu!pixels per second
{
	return (bpm / 60.0) * sv * 100;
}

float erfInv(float x) // Inverse Error Function 
{
	float w, p;

	w = -logf((1.0f - x) * (1.0f + x));

	if (w < 5.000000f)
	{
		w = w - 2.500000f;
		p =  2.81022636e-08f;
		p =  3.43273939e-07f + p*w;
		p = -3.5233877e-06f  + p*w;
		p = -4.39150654e-06f + p*w;
		p =  0.00021858087f  + p*w;
		p = -0.00125372503f  + p*w;
		p = -0.00417768164f  + p*w;
		p =  0.246640727f    + p*w;
		p =  1.50140941f     + p*w;
	}
	else 
	{
		w = sqrtf(w) - 3.000000f;
		p = -0.000200214257f;
		p =  0.000100950558f + p*w;
		p =  0.00134934322f  + p*w;
		p = -0.00367342844f  + p*w;
		p =  0.00573950773f  + p*w;
		p = -0.0076224613f   + p*w;
		p =  0.00943887047f  + p*w;
		p =  1.00167406f     + p*w;
		p =  2.83297682f     + p*w;
	}
	return p*x;
}

double getMagnitude(std::vector<double> vals)
{
	double sum = 0;
	for (unsigned i = 0; i < vals.size(); i++)
		sum += pow(vals[i], 2);
	return sqrt(sum);
}

double getWeightedValue(std::vector<double>& vals, double decay)
{
	double weightDecay = 1.0, result = 0;
	for (unsigned i = 0; i < vals.size(); i++)
	{
		result += weightDecay * vals[i];
		weightDecay *= decay;
	}
	return result * (1.0 - decay);
}

double getWeightedValue2(std::vector<double>& vals, double decay)
{
	double result = 0;
	for (unsigned i = 0; i < vals.size(); i++)
	{
		result += vals[i] * pow(decay, i);
	}
	return result;
}

void getDecayFunction(std::vector<double>& vals, double decay, std::vector<double>& output)
{
	double feedback = 0;
	for (unsigned i = 0; i < vals.size(); i++)
	{
		double result = decay * feedback + vals[i];
		feedback = result;
		output.push_back(result);
	}
}

void getTopVals(std::vector<double>& vals, int numVals, std::vector<double>& output)
{
	output = { 0 };
	for (unsigned i = 0; i < vals.size(); i++)
	{
		// TODO: Binary search instead of going one by one for optimization
		for (unsigned j = 0; j < output.size(); j++)
		{
			if (output[j] < vals[i])
			{
				output.insert(output.begin() + j, vals[i]);
				if (static_cast<int>(output.size()) > numVals)
					output.erase(output.end() - 1);
				break;
			}
		}
	}
}

void getPeakVals(std::vector<double> &vals, std::vector<double> &output)
{
	for (unsigned i = 1; i < vals.size() - 1; i++)
	{
		if (vals[i] > vals[i - 1] && vals[i] > vals[i + 1])
			output.push_back(vals[i]);
	}
	std::sort(output.rbegin(), output.rend());
}

bool isOppositeParity(double x, double y)
{
	return (((x < 0) && (y > 0)) || ((x > 0) && (y < 0)));
}

int getValuePos(std::vector<double> list, double value, bool order)
{
	// Until binary search is fixed, use this
	if (order == 0) // descending
	{
		for (int i = list.size() - 1; i >= 1; i--)
			if (list[i - 1] < value)
				return i;
		return 0;
	}
	else // ascending
	{
		for (unsigned i = 0; i < list.size() - 1; i++)
			if (list[i + 1] > value)
				return i;
		return list.size() - 1;
	}
}

long binomialCoefficient(int n, int k)
{
	if (k < 0 || k > n)		return 0;
	if (k == 0 || k == n)	return 1;

	k = static_cast<int>(std::fmin(k, n - k));  // take advantage of symmetry
	long c = 1;
	for (int i = 0; i < k; i++)
		c = c * (n - i) / (i + 1);

	return c;
}

double bernstein(int i, int n, double t)
{
	return binomialCoefficient(n, i) * pow(t, i) * pow(1 - t, n - i);
}

double lerp(double a, double b, double t)
{
	return a * (1 - t) + b * t;
}