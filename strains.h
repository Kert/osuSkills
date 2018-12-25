#ifndef _strains_h_
#define _strains_h_

#include "globals.h"
#include <vector>

void CalculateTapStrains(Beatmap &beatmap);
void CalculateAimStrains(Beatmap &beatmap);
void CalculateAgilityStrains(Beatmap &beatmap);
void ClearStrains(Beatmap &beatmap);

#endif