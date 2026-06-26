#include "reading.h"
#include <algorithm>
#include "strains.h"
#include "tweakvars.h"
#include "utils.h"

const double distance_influence_threshold = 150;
const double minimum_angle_relevancy_time = 2000;
const double maximum_angle_relevancy_time = 200;
const double reading_window_size = 3000;
const double hidden_multiplier = 0.28;
const double FADE_IN_DURATION_MULTIPLIER = 0.4;
const double FADE_OUT_DURATION_MULTIPLIER = 0.3;
const double strainDecayBase = 0.8;
const double skillMultiplier = 1;
const int MIN_DELTA_TIME = 15;

void CalculateReading(Beatmap& beatmap, bool hidden)
{
	int readingPointsCount = beatmap.readingPoints.size();
	double strain = 0;
	for(int i = 0; i < readingPointsCount; i++)
	{
		ReadingPoint *current = &beatmap.readingPoints[i];
		ReadingPoint *previous = nullptr;
		if(i - 1 >= 0)
			previous = &beatmap.readingPoints[i - 1];
		ReadingPoint *next;
		double interval = reading_window_size;
		if(i + 1 >= readingPointsCount)
			next = nullptr;
		else
		{
			next = &beatmap.readingPoints[i + 1];
			interval = std::max(MIN_DELTA_TIME, next->time - current->time);
		}
		double diff = EvaluateDifficultyOf(beatmap.readingPoints, *current, *previous, *next, beatmap.ar, hidden);
		double decay = pow(strainDecayBase, interval / 1000);

		strain *= decay;
		strain += diff * (1 - decay);
		beatmap.readingStrains.push_back(strain);
	}

	beatmap.skills.reading = *max_element(std::begin(beatmap.readingStrains), std::end(beatmap.readingStrains));

	std::vector<double> topWeights;
	getPeakVals(beatmap.readingStrains, topWeights);

	beatmap.skills.reading = getWeightedValue2(topWeights, GetVar("Reading", "Weighting"));
	beatmap.skills.reading = GetVar("Reading", "TotalMult") * pow(beatmap.skills.reading, GetVar("Reading", "TotalPow"));
}

double EvaluateDifficultyOf(std::vector<ReadingPoint> &points, ReadingPoint &current, ReadingPoint &previous, ReadingPoint &next, double AR, bool hidden)
{
	double interval = reading_window_size;
	if(&previous)
		interval = std::max(MIN_DELTA_TIME, current.time - previous.time);
	double velocity = std::max(1.0, current.dist / interval);
	double pastObjectDifficultyInfluence = getPastObjectDifficultyInfluence(points, current, AR);

	double constantAngleNerfFactor = getConstantAngleNerfFactor(points, current);
	double currentVisibleObjectDensity = retrieveCurrentVisibleObjectDensity(points, current, AR);
	double noteDensityDifficulty = calculateDensityDifficulty(&current, &next, velocity, constantAngleNerfFactor, pastObjectDifficultyInfluence, currentVisibleObjectDensity);
	double hiddenDifficulty = 0.;
	if(hidden)
		hiddenDifficulty = calculateHiddenDifficulty(current, previous, AR, pastObjectDifficultyInfluence, currentVisibleObjectDensity, velocity, constantAngleNerfFactor);

	double readingDifficulty = norm(1.5, std::vector<double>{hiddenDifficulty, noteDensityDifficulty});

	// Having less time to process information is harder
	readingDifficulty *= 1. / (1. - pow(0.5, interval / 1000));

	return readingDifficulty;
}

double calculateDensityDifficulty(ReadingPoint *current, ReadingPoint *next, double velocity, double constantAngleNerfFactor, double pastObjectDifficultyInfluence, double currentVisibleObjectDensity)
{
	// Consider future densities too because it can make the path the cursor takes less clear
	double futureObjectDifficultyInfluence = sqrt(currentVisibleObjectDensity);

	if(next != nullptr)
	{
		// Reduce difficulty if movement to next object is small
		futureObjectDifficultyInfluence *= smootherstep(next->pos.getDistanceFrom(current->pos), 15, distance_influence_threshold);
	}

	// Value higher note densities exponentially
	double noteDensityDifficulty = pow(pastObjectDifficultyInfluence + futureObjectDifficultyInfluence, 1.7) * 0.4 * constantAngleNerfFactor * velocity;

	const double density_difficulty_base = 2.5;
	const double density_multiplier = 2.4;

	// Award only denser than average maps.
	noteDensityDifficulty = std::max(0.0, noteDensityDifficulty - density_difficulty_base);

	// Apply a soft cap to general density reading to account for partial memorization
	noteDensityDifficulty = pow(noteDensityDifficulty, 0.45) * density_multiplier;

	return noteDensityDifficulty;
}

// Returns a factor of how often the current object's angle has been repeated in a certain time frame.
// It does this by checking the difference in angle between current and past objects and sums them based on a range of similarity.
// https://www.desmos.com/calculator/eb057a4822
double getConstantAngleNerfFactor(std::vector<ReadingPoint> &points, ReadingPoint &current)
{
	double constantAngleCount = 0;
	double currentTimeGap = 0;

	ReadingPoint *loopObjPrev0 = &current;
	ReadingPoint *loopObjPrev1 = nullptr;
	ReadingPoint *loopObjPrev2 = nullptr;

	while(currentTimeGap < minimum_angle_relevancy_time)
	{
		int loopId = loopObjPrev0->index - 1;
		if(loopId < 0)
			break;

		ReadingPoint *loopObj = &points[loopId];

		// Account less for objects that are close to the time limit.

		currentTimeGap = current.time - loopObj->time;

		double longIntervalFactor = 1 - reverseLerp(std::max(currentTimeGap, 15.), maximum_angle_relevancy_time, minimum_angle_relevancy_time);

		if(loopObj && current.angle != 0.0 && loopObjPrev1 && loopObjPrev2)
		{
			double angleDifference = abs(current.angle - loopObj->angle);
			double angleDifferenceAlternating = 180.0;

			angleDifferenceAlternating = abs(loopObjPrev1->angle - loopObj->angle);
			angleDifferenceAlternating += abs(loopObjPrev2->angle - loopObjPrev0->angle);

			double weight = 1.0;

			// Be sure that one of the angles is very sharp, when other is wide
			weight *= reverseLerp(std::min(loopObj->angle, loopObjPrev0->angle), 20, 5);
			weight *= reverseLerp(std::max(loopObj->angle, loopObjPrev0->angle), 60, 120);

			// Lerp between max angle difference and rescaled alternating difference, with more harsh scaling compared to normal difference
			angleDifferenceAlternating = lerp(180, 0.1 * angleDifferenceAlternating, weight);

			double stackFactor = smootherstep(loopObj->dist, 0, 50);
			constantAngleCount += cos(3 * std::min(30., std::min(angleDifference, angleDifferenceAlternating) * stackFactor)) * longIntervalFactor;
		}

		loopObjPrev2 = loopObjPrev1;
		loopObjPrev1 = loopObjPrev0;
		loopObjPrev0 = loopObj;
	}

	return clamp(2. / constantAngleCount, 0.2, 1.);
}

double getPastObjectDifficultyInfluence(std::vector<ReadingPoint> &points, ReadingPoint &current, double AR)
{
	double pastObjectDifficultyInfluence = 0;

	ReadingPoint *previous = nullptr;
	for(auto &loopObj : retrievePastVisibleObjects(points, current))
	{
		double loopDifficulty = GetOpacityAt(current, loopObj->time, AR, false);

		// When aiming an object small distances mean previous objects may be cheesed, so it doesn't matter whether they were arranged confusingly.
		if(previous)
			loopDifficulty *= smootherstep(loopObj->dist, 15, distance_influence_threshold);
		else
			loopDifficulty = 0;

		// Account less for objects close to the max reading window
		double timeBetweenCurrAndLoopObj = current.time - loopObj->time;
		double timeNerfFactor = getTimeNerfFactor(timeBetweenCurrAndLoopObj);

		loopDifficulty *= timeNerfFactor;
		pastObjectDifficultyInfluence += loopDifficulty;
		previous = loopObj;
	}

	return pastObjectDifficultyInfluence;
}

double GetOpacityAt(ReadingPoint &point, int time, double AR, bool hidden)
{
	if(time > point.time)
	{
		// Consider a ReadingPoint as being invisible when its start time is passed.
		// In reality the ReadingPoint will be visible beyond its start time up until its hittable window has passed,
		// but this is an approximation and such a case is unlikely to be hit where this function is used.
		return 0.0;
	}

	double fadeInStartTime = point.preempt;

	double fadeInDuration = std::min(AR2ms(AR), 400);

	if(hidden)
	{
		fadeInDuration = 0.4 * AR2ms(AR);
		double fadeOutStartTime = point.fadeout;
		double fadeOutDuration = point.preempt * FADE_OUT_DURATION_MULTIPLIER;

		return std::min(
			clamp((time - fadeInStartTime) / fadeInDuration, 0.0, 1.0),
			1.0 - clamp((time - fadeOutStartTime) / fadeOutDuration, 0.0, 1.0)
		);
	}

	return clamp((time - fadeInStartTime) / fadeInDuration, 0.0, 1.0);
}

// Returns a nerfing factor for when objects are very distant in time, affecting reading less.
double getTimeNerfFactor(double deltaTime)
{
	return clamp(2. - deltaTime / (reading_window_size / 2.), 0., 1.);
}

// Returns a list of objects that are visible on screen at the point in time the current object becomes visible.
std::vector<ReadingPoint*> retrievePastVisibleObjects(std::vector<ReadingPoint> &points, ReadingPoint &current)
{
	std::vector<ReadingPoint*> pastObjects;
	for(int i = current.index; i > 0; i--)
	{
		ReadingPoint *prevPoint = &points[i - 1];

		if(current.time - prevPoint->time > reading_window_size ||
			prevPoint->time < current.preempt) // Current object not visible at the time object needs to be clicked
			break;

		pastObjects.push_back(prevPoint);
	}
	std::reverse(pastObjects.begin(), pastObjects.end());
	return pastObjects;
}

// Returns the density of objects visible at the point in time the current object needs to be clicked capped by the reading window.
double retrieveCurrentVisibleObjectDensity(std::vector<ReadingPoint> &points, ReadingPoint &current, double AR)
{
	double visibleObjectCount = 0;

	for(int i = current.index; i < (int)points.size() - 1; i++)
	{
		ReadingPoint *nextPoint = &points[i + 1];
		double timeDiff = nextPoint->time - current.time;
		if(timeDiff > reading_window_size ||
			current.time < nextPoint->preempt) // Object not visible at the time current object needs to be clicked.
			break;

		double timeNerfFactor = getTimeNerfFactor(timeDiff);

		visibleObjectCount += GetOpacityAt(*nextPoint, current.time, AR, false) * timeNerfFactor;
	}
	return visibleObjectCount;
}

double calculateHiddenDifficulty(ReadingPoint &currObj, ReadingPoint &previous, double AR, double pastObjectDifficultyInfluence, double currentVisibleObjectDensity, double velocity, double constantAngleNerfFactor)
{
	if(&previous == nullptr)
		return 0;

	// Higher preempt means that time spent invisible is higher too, we want to reward that
	double preemptFactor = pow(currObj.time - currObj.preempt, 2.2) * 0.01;
	preemptFactor = 1;

	// Account for both past and current densities
	double densityFactor = pow(currentVisibleObjectDensity + pastObjectDifficultyInfluence, 3.3) * 3;

	double hiddenDifficulty = (preemptFactor + densityFactor) * constantAngleNerfFactor * velocity * 0.01;

	// Apply a soft cap to general HD reading to account for partial memorization
	hiddenDifficulty = pow(hiddenDifficulty, 0.4) * hidden_multiplier;

	// Buff perfect stacks only if current note is completely invisible at the time you click the previous note.
	if(currObj.dist == 0.0 && GetOpacityAt(currObj, previous.time, AR, true) == 0 && previous.time > currObj.preempt)
		hiddenDifficulty += hidden_multiplier * 2500 / pow(currObj.time, 1.5); // Perfect stacks are harder the less time between notes

	return hiddenDifficulty;
}