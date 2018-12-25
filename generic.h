#ifndef _generic_h_
#define _generic_h_

#include <vector>
#include <set>
#include "globals.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

void PrepareAimData(Beatmap &beatmap);
void PrepareTapData(Beatmap &beatmap);
void PrepareTimingPoints(Beatmap &beatmap);

void CalculatePressIntervals(Beatmap &beatmap);
void CalculateMovementData(Beatmap &beatmap);

void GatherTapPatterns(Beatmap &beatmap);
Burst GetHardestBurst(Beatmap &beatmap);
void GatherTargetPoints(Beatmap &beatmap);
void GatherAimPoints(Beatmap &beatmap);
void CalculateAngles(Beatmap &beatmap);
void BakeSliderData(Beatmap &beatmap);

#endif
