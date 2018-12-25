#ifndef _utils_h_
#define _utils_h_

#include <string>
#include "globals.h"

#define M_PI	3.14159265358979323846
#define DegToRad(angleDegrees) (angleDegrees * M_PI / 180.0)
#define Rad2Deg(angleRadians) (angleRadians * 180.0 / M_PI)

double BOUND(double _min, double _val, double _max);
bool BTWN(double lss, double val, double gtr);

std::string msToTimeString(int ms);

int GetLastTickTime(HitObject &hitObj);
bool IsHitObjectType(int Type, HitObjectType type);
unsigned int FindHitobjectAt(std::vector<HitObject>& _hitobjects, long _time, bool _dir = false);
double GetNoteDistanceAt(Beatmap &beatmap, int i, bool aimpoint);
int FindTimingAt(std::vector<TIMING>& _timings, double _time);
double GetChaosAt(Beatmap &beatmap, int i, int numObj = -1);

double getValue(double min, double max, double percent);
double getPercent(double _min, double _val, double _max);

double GetCircleOverlapPercentage(Beatmap &beatmap, HitObject &c1, HitObject &c2);

double GetDirAngle(Vector2d &a, Vector2d &b, Vector2d &c);
double GetAngle(Vector2d &a, Vector2d &b, Vector2d &c);

int msToBPM(int ms);

int AR2ms(double ar);
double ms2AR(int ms);

double OD2ms(double od);
double ms2OD(double ms);

int CS2px(double cs);
double BpmSv2px(double bpm, double sv);

float erfInv(float x);

// use for getting the overall combined result of values
double getMagnitude(std::vector<double> vals); 

// use for getting the combined result, with top values counting the most
double getWeightedValue(std::vector<double>& vals, double decay);
double getWeightedValue2(std::vector<double>& vals, double decay);

// use for smoothing the function based on extremes of values
void getDecayFunction(std::vector<double>& vals, double decay, std::vector<double>& output);

void getTopVals(std::vector<double>& vals, int numVals, std::vector<double>& output);
void getPeakVals(std::vector<double> &vals, std::vector<double> &output);

// return true if one value is pos and the other is neg or vice-versa
bool isOppositeParity(double x, double y);

// returns index of where the value is between 2 values using binary search
// order = 0 is decsending and order = 1 is ascending
int getValuePos(std::vector<double> list, double value, bool order);

// Calculates the binomial coefficient.
// http://en.wikipedia.org/wiki/Binomial_coefficient#Binomial_coefficient_in_programming_languages */
long binomialCoefficient(int n, int k);

/**
* Calculates the Bernstein polynomial.
* @param i the index
* @param n the degree of the polynomial (i.e. number of points)
* @param t the t value [0, 1]
*/
double bernstein(int i, int n, double t);

/**
* Linear interpolation of a and b at t.
*/
double lerp(double a, double b, double t);

template <class T>
int sign(T x)
{
	return (x > 0) - (x < 0);
};

#endif