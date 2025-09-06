#ifndef _globals_h_
#define _globals_h_

#include <map>
#include <string>
#include "vector2d.h"

struct TIMING
{
	long time;		// The what time this occurs at
	double data;	// generic data associated with this timing
	int key;		// generic integral data (can be used for which key pressed, timing state, etc)
	bool press;		// generic boolean data (can be used to determine slider or not, pressed key or not, etc)

	Vector2d pos;   // additional timing info for 2d coordinates
};

enum MODS
{
	NF = 1,
	EZ = 2,
	HD = 8,
	HR = 16,
	SD = 32,
	DT = 64,
	RL = 128,
	HT = 256,
	FL = 1024,
	AU = 2048,
	SO = 4096,
	AP = 8192
};

struct TimingPoint
{
	int offset = -1;
	double beatInterval = 0;
	int meter = -1;
	bool inherited = 0;
	double sm = -1; // calculated
	double bpm = -1; // calculated
};
	
enum HitObjectType
{
	Normal = 1,
	SLIDER = 2,
	NewCombo = 4,
	NormalNewCombo = 5,
	SliderNewCombo = 6,
	Spinner = 8,
	ColourHax = 112,
	Hold = 128,
	ManiaLong = 128
};

enum CurveType
{
	PerfectCurve = 'P',
	BezierCurve = 'B',
	LinearCurve = 'L',
	CatmullCurve = 'C'
};

struct HitObject
{
	Vector2d pos;
	int time;
	int type;

	// slider-specific
	char curveType;
	std::vector<Vector2d> curves; // from file
	std::vector<Vector2d> lerpPoints; // calculated
	int ncurve; // number of calculated points
	int repeat; // number of repeats
	std::vector<int> repeatTimes; // the time of repeats
	double pixelLength;
	int endTime;	   // also for spinner
	int toRepeatTime;  // time to travel to reverse arrow
	Vector2d endPoint; // calculated
	std::vector<int> ticks;
};

struct Burst
{
	int interval;
	double strain;
};

struct Stream
{
	int interval;  // how long the stream lasts
	int length;    // # of notes
};

enum AIM_POINT_TYPES
{
	AIM_POINT_NONE,
	AIM_POINT_CIRCLE,
	AIM_POINT_SLIDER,
	AIM_POINT_SLIDERREVERSE,
	AIM_POINT_SLIDEREND	
};

struct AimPoint
{
	int time;
	Vector2d pos;
	AIM_POINT_TYPES type;
};

struct Skills
{
	double stamina = 0;
	double tenacity = 0;
	double agility = 0;
	double precision = 0;
	double reading = 0;
	double memory = 0;
	double accuracy = 0;
	double reaction = 0;
};

struct Beatmap
{
	int format = -1; // *.osu format

	// metadata
	std::string artist;
	std::string title;
	std::string version; // difficulty name
	std::string creator;
	std::string name; // Artist - Title (Creator) [Difficulty]

	// difficulties
	int hp = -1;
	double cs = -1;
	double od = -1;
	double ar = -1;
	double sm = -1;     // slider velocity
	double st = -1;     // slider tick rate
	double bpmMin = -1; // calculated
	double bpmMax = -1; // calculated
	std::vector<TimingPoint> timingPoints;
	std::vector<HitObject> hitObjects;
	std::vector<AimPoint> aimPoints;
	std::vector<TIMING> targetPoints;
	int spinners = 0;

	// helper for using time as an index to hitObjects vector
	std::map<int, int> timeMapper;

	// Aim
	struct
	{
		std::vector<double> X;
		std::vector<double> Y;
		std::vector<double> Xchange;
		std::vector<double> Ychange;
	} velocities;
	std::vector<double> distances;
	std::vector<double> aimStrains;
	std::vector<double> angleStrains;

	// Reading
	std::vector<double> angles;
	std::vector<double> angleBonuses;
	std::vector<int> reactionTimes;

	// Tapping
	std::vector<int> pressIntervals;
	std::vector<double> tapStrains;

	// Tenacity related
	std::map<int, std::vector<std::vector<int>>> streams;

	// Stamina related
	std::map<int, std::vector<std::vector<int>>> bursts;

	// Calculated skill values
	Skills skills;

	// Selected mods
	int mods = 0;
	std::string modsString;

	// patterns
	struct
	{
		std::vector<int> compressedStream;
		std::vector<int> stream;
		std::vector<int> stack;
	} patterns;
};

#endif
