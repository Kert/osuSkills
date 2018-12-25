#include "tenacity.h"
#include "tweakvars.h"
#include "utils.h"

Stream GetLongestStream(std::map<int, std::vector<std::vector<int>>> &streams)
{
	int max = 1;
	int interval = 0;
	for (auto stream : streams)
	{
		interval = stream.first;

		max = 1;
		for (auto j : stream.second)
		{
			int length = j.size() + 1;
			if (length > max)
				max = j.size() + 1;
		}
		if (max > 1)
			break;
	}
	return{ interval, max };
}

double CalculateTenacity(Beatmap &beatmap)
{
	Stream longestStream = GetLongestStream(beatmap.streams);

	double intervalScaled = 1.0 / pow(longestStream.interval, pow(longestStream.interval, GetVar("Tenacity", "IntervalPow")) * GetVar("Tenacity", "IntervalMult")) * GetVar("Tenacity", "IntervalMult2");
	double lengthScaled = pow(GetVar("Tenacity", "LengthDivisor") / longestStream.length, GetVar("Tenacity", "LengthDivisor") / longestStream.length * GetVar("Tenacity", "LengthMult"));
	double tenacity = intervalScaled * lengthScaled;
	beatmap.skills.tenacity = tenacity;
	beatmap.skills.tenacity = GetVar("Tenacity", "TotalMult") * pow(beatmap.skills.tenacity, GetVar("Tenacity", "TotalPow"));
	return beatmap.skills.tenacity;
}