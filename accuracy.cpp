#include "accuracy.h"
#include <iostream>
#include <fstream>
#include "tweakvars.h"
#include "utils.h"
#include "mods.h"
#include "math.h"

double CalculateAccuracy(Beatmap &beatmap)
{
	double circles = 0;

	for (auto &obj : beatmap.hitObjects)
		if (IsHitObjectType(obj.type, HitObjectType::Normal))
			circles++;

	double od_ms = OD2ms(beatmap.od);

	     if (HasMod(beatmap, MODS::DT)) od_ms /= 1.5;
	else if (HasMod(beatmap, MODS::HT)) od_ms /= 0.75;
	
	//std::cout << "circles = " << circles << "   od_ms: " << od_ms << "    stamina = " << beatmap.skills.stamina << "\n";

	double tapping = 0;
	if(beatmap.skills.stamina == 0) tapping = erf(INFINITY);
	else                            tapping = erf(od_ms / (GetVar("Accuracy", "AccScale") * beatmap.skills.stamina * beatmap.skills.stamina));

	beatmap.skills.accuracy = -GetVar("Accuracy", "VerScale")*circles*log(tapping);
	return beatmap.skills.accuracy;
}