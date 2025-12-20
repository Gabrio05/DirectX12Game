#pragma once
#include <vector>
#include "Maths.h"

class ThrowingScheduleTutorial {
public:
	std::vector<float> throw_at{ 2, 35, 35, 45, 60, 60, 60 };
	std::vector<Vec3> thrown_from{ Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15) };
	std::vector<float> time_to_player{ 5, 6, 6, 5, 9, 9, 9 };
	std::vector<Vec3> offset_from_player{ Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0) };
	int already_thrown = 0;
};

static const std::vector<Vec3> from = {
	Vec3(1, 0, 1),
	Vec3(0, 0, 1),
	Vec3(-1, 0, 1),
	Vec3(-15, 0, 14),
	Vec3(-15, 0, 15),
	Vec3(-15, 0, 16),
	Vec3(-1, 0, 28),
	Vec3(0, 0, 28),
	Vec3(1, 0, 28),
	Vec3(15, 0, 16),
	Vec3(15, 0, 15),
	Vec3(15, 0, 14)
};
static const std::vector<Vec3> offset = {
	Vec3(1, 0, 0),
	Vec3(0, 0, 0),
	Vec3(-1, 0, 0),
	Vec3(0, 0, -1),
	Vec3(0, 0, 0),
	Vec3(0, 0, 1),
	Vec3(-1, 0, 0),
	Vec3(0, 0, 0),
	Vec3(1, 0, 0),
	Vec3(0, 0, 1),
	Vec3(0, 0, 0),
	Vec3(0, 0, -1)
};

class ThrowingScheduleBossFight {
public:
	std::vector<float> throw_at{ 
		0, 6, 12, 18, 
		24, 24, 30, 30, 36, 36, 42, 42,  //intro

		160, 160, 160, 160, 160, 160, 160, 160,  // after breakdown
		172, 172,
		// actual start
		184, 184, 190, 190,
		196, 196, 202, 202,
		210.667, 211.333, 212, 212.667, 213.333, 216.667, 217.333, 218, 218.667, 219.333,
		222.667, 223.333, 224, 224.667, 225.333, 226, 226,

		// BPM Change Bonanza
		232, 234, 236, 238, 240, 
		242, 244, 246, 248, 250,
		252, 254, 256, 258, 260,
		262, 264, 266, 268, 269, 269, 270, 270, 271, 271,
		273, 275, 277, 279, 281,
		283, 285, 287,

		292, 292, 296.5, 301, 301, 305.5,
		310, 310, 314.5, 319, 319, 323.5,
		328, 328, 332.5, 337, 337, 341.5,
		
		346, 346, 351, 351, 356, 356, 361, 361,
		
		366, 370.5, 375, 379.5,
		
		384, 384, 392, 392,
		398, 398, 406, 406,
		412, 412, 420, 420,
		426, 426, 426, 426, 434, 434, 434, 434
	};
	std::vector<Vec3> thrown_from{
		from[1], from[0], from[1], from[2],
		from[0], from[1], from[1], from[2], from[3], from[4], from[4], from[5],  //intro

		from[0], from[2], from[3], from[5], from[6], from[8], from[9], from[11],  // after breakdown
		from[8], from[11],
		// actual start
		from[1], from[2], from[9], from[10],
		from[3], from[4], from[7], from[8],
		from[1], from[0], from[2], from[0], from[1], from[1], from[2], from[0], from[2], from[1],
		from[1], from[2], from[2], from[0], from[1], from[0], from[2],

		// BPM Change Bonanza
		from[0], from[1], from[2], from[3], from[4],
		from[6], from[7], from[8], from[9], from[10],
		from[1], from[4], from[7], from[10], from[1],
		from[10], from[7], from[4], from[1], from[0], from[1], from[0], from[2], from[1], from[2],
		from[1], from[4], from[1], from[4], from[1],
		from[4], from[1], from[4],

		from[0], from[1], from[2], from[1], from[2], from[0],
		from[3], from[4], from[5], from[4], from[5], from[3],
		from[7], from[8], from[6], from[6], from[7], from[8],

		from[9], from[10], from[10], from[11], from[10], from[11], from[9], from[10],

		from[0], from[1], from[2], from[1],

		from[3], from[4], from[4], from[5],
		from[6], from[7], from[7], from[8],
		from[9], from[10], from[10], from[11],
		from[0], from[2], from[3], from[4], from[0], from[2], from[10], from[11]
	};
	std::vector<float> time_to_player{
		6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6,  //intro

		1, 1, 1, 1, 1, 1, 1, 1,  // after breakdown
		6, 6,
		// actual start
		6, 6, 6, 6,
		6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6,

		// BPM Change Bonanza
		5, 5, 5, 5, 5,
		5, 5, 5, 5, 5,
		5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5,
		5, 5, 5,

		5, 5, 4, 5, 5, 4,
		5, 5, 4, 5, 5, 4,
		5, 5, 4, 5, 5, 4,

		5, 5, 5, 5, 5, 5, 5, 5,

		5, 4, 5, 4,

		8, 8, 6, 6,
		8, 8, 6, 6,
		8, 8, 6, 6,
		8, 8, 6, 6, 8, 8, 6, 6
	};
	std::vector<Vec3> offset_from_player{
		offset[1], offset[0], offset[1], offset[2],
		offset[0], offset[1], offset[1], offset[2], offset[3], offset[4], offset[4], offset[5],  //intro

		offset[0], offset[2], offset[3], offset[5], offset[6], offset[8], offset[9], offset[11],  // after breakdown
		offset[8], offset[11],
		// actual start
		offset[1], offset[2], offset[9], offset[10],
		offset[3], offset[4], offset[7], offset[8],
		offset[1], offset[0], offset[2], offset[0], offset[1], offset[1], offset[2], offset[0], offset[2], offset[1],
		offset[1], offset[2], offset[2], offset[0], offset[1], offset[0], offset[2],

		// BPM Change Bonanza
		offset[0], offset[1], offset[2], offset[3], offset[4],
		offset[6], offset[7], offset[8], offset[9], offset[10],
		offset[1], offset[4], offset[7], offset[10], offset[1],
		offset[10], offset[7], offset[4], offset[1], offset[0], offset[1], offset[1], offset[2], offset[0], offset[2],
		offset[1], offset[4], offset[1], offset[4], offset[1],
		offset[4], offset[1], offset[4],

		offset[0], offset[1], offset[2], offset[1], offset[2], offset[0],
		offset[3], offset[4], offset[5], offset[4], offset[5], offset[3],
		offset[7], offset[8], offset[6], offset[6], offset[7], offset[8],

		offset[9], offset[10], offset[10], offset[11], offset[10], offset[11], offset[9], offset[10],

		offset[0], offset[1], offset[2], offset[1],

		offset[3], offset[4], offset[4], offset[5],
		offset[6], offset[7], offset[7], offset[8],
		offset[9], offset[10], offset[10], offset[11],
		offset[0], offset[2], offset[3], offset[4], offset[0], offset[2], offset[10], offset[11]
	};
	int already_thrown = 0;
};