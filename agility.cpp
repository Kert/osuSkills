#include "agility.h"

#include <algorithm>
#include "tweakvars.h"
#include "utils.h"

double CalculateAgility(Beatmap &beatmap)
{
	auto max = max_element(std::begin(beatmap.aimStrains), std::end(beatmap.aimStrains));
	int index = std::distance(beatmap.aimStrains.begin(), max);
	int time = beatmap.aimPoints[index].time;
	beatmap.skills.agility = *max;

	std::vector<double> topWeights;
	getPeakVals(beatmap.aimStrains, topWeights);

	beatmap.skills.agility = getWeightedValue2(topWeights, GetVar("Agility", "Weighting"));
	beatmap.skills.agility = GetVar("Agility", "TotalMult") * pow(beatmap.skills.agility, GetVar("Agility", "TotalPow"));
	return beatmap.skills.agility;
}
