#ifndef _reading_h_
#define _reading_h_

#include "globals.h"

void CalculateReading(Beatmap& beatmap, bool hidden);
double EvaluateDifficultyOf(std::vector<ReadingPoint> &points, ReadingPoint &current, ReadingPoint &previous, ReadingPoint &next, double ar, bool hidden);
double calculateDensityDifficulty(ReadingPoint *current, ReadingPoint *next, double velocity, double constantAngleNerfFactor, double pastObjectDifficultyInfluence, double currentVisibleObjectDensity);
double getConstantAngleNerfFactor(std::vector<ReadingPoint> &points, ReadingPoint &current);
double getPastObjectDifficultyInfluence(std::vector<ReadingPoint> &points, ReadingPoint &current, double ar);
double GetOpacityAt(ReadingPoint &point, int time, double ar, bool hidden);
double getTimeNerfFactor(double deltaTime);
std::vector<ReadingPoint*> retrievePastVisibleObjects(std::vector<ReadingPoint> &points, ReadingPoint &current);
double retrieveCurrentVisibleObjectDensity(std::vector<ReadingPoint> &points, ReadingPoint &current, double ar);
double calculateHiddenDifficulty(ReadingPoint &currObj, ReadingPoint &previous, double ar, double pastObjectDifficultyInfluence, double currentVisibleObjectDensity, double velocity, double constantAngleNerfFactor);

#endif
