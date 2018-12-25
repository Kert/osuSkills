#include "precision.h"
#include "utils.h"
#include "patterns.h"
#include "tweakvars.h"

#include <algorithm>

double CalculatePrecision(Beatmap& beatmap, bool hidden)
{
	double scaledAgility;
	if (beatmap.skills.agility > GetVar("Precision", "AgilityLimit"))
		scaledAgility = 1;

	scaledAgility = pow(beatmap.skills.agility + 1, GetVar("Precision", "AgilityPow")) - GetVar("Precision", "AgilitySubtract");

	// the magic number above is to make an agility of 10 become 1 when scaled
	beatmap.skills.precision = scaledAgility * beatmap.cs;
	beatmap.skills.precision = GetVar("Precision", "TotalMult") * pow(beatmap.skills.precision, GetVar("Precision", "TotalPow"));

	return beatmap.skills.precision;
}