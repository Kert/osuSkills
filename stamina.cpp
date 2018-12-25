#include "stamina.h"
#include "tweakvars.h"
#include "utils.h"
#include <algorithm>

double CalculateStamina(Beatmap &beatmap)
{
	auto max = max_element(std::begin(beatmap.tapStrains), std::end(beatmap.tapStrains));
	int index = std::distance(beatmap.tapStrains.begin(), max);
	int time = beatmap.hitObjects[index].time;
	beatmap.skills.stamina = *max;
	beatmap.skills.stamina = GetVar("Stamina", "TotalMult") * pow(beatmap.skills.stamina, GetVar("Stamina", "TotalPow"));
	return beatmap.skills.stamina;
}
