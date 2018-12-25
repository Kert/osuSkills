#ifndef _patterns_h_
#define _patterns_h_

#include "globals.h"
#include "Slider.h"
#include <iostream>

void DetectCompressedStream(Beatmap &beatmap, int &i);
void DetectStacks(Beatmap &beatmap, int &i);
bool DetectStream(Beatmap &beatmap, int &i);

bool isInStream(Beatmap &beatmap, int i);
bool isInStack(Beatmap &beatmap, int i);

void AnalyzePatterns(Beatmap &beatmap);
void FindPatterns(Beatmap &beatmap);


// --> patterns

std::vector<std::tuple<int, int, int, int>> getPattern(std::vector<HitObject> &obj, int time, int index, double CS, unsigned int quant = 3);


// --> hitobjects

std::pair<int, int> getVisiblityTimes(HitObject &obj, double AR, bool hidden, double opacityStart, double opacityEnd);

double getVelocity(HitObject &obj);

int getHitcircleAt(std::vector<HitObject> &hitcircles, int time);

#endif