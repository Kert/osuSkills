#ifndef _tenacity_h_
#define _tenacity_h_

#include "globals.h"

Stream GetLongestStream(std::map<int, std::vector<std::vector<int>>> &streams);
double CalculateTenacity(Beatmap &beatmap);

#endif