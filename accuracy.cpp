#include "accuracy.h"
#include <iostream>
#include <fstream>
#include "tweakvars.h"
#include "utils.h"
#include "mods.h"
#include "math.h"

double CalculateAccuracy(Beatmap &beatmap)
{
	int Circles = 0;
	for (auto &obj : beatmap.hitObjects)
		if (IsHitObjectType(obj.type, HitObjectType::Normal))
			Circles++;

	double Cert = 0.1;
	double SS_UR = (5 * sqrt(2) * OD2ms(beatmap.od)) / erfInv((double)pow(Cert, (1 / (double)Circles)));
	if (HasMod(beatmap, MODS::DT))
		SS_UR /= 1.5;
	else if(HasMod(beatmap, MODS::HT))
		SS_UR /= 0.75;
	beatmap.skills.accuracy = pow(beatmap.skills.stamina, GetVar("Accuracy", "VerScale")) / SS_UR;
	beatmap.skills.accuracy = GetVar("Accuracy", "TotalMult") * pow(beatmap.skills.accuracy, GetVar("Accuracy", "TotalPow"));
	
	return beatmap.skills.accuracy;
}
