#pragma once
#include <vector>
#include "Maths.h"

class ThrowingScheduleTutorial {
public:
	std::vector<float> throw_at{ 25, 35, 35, 45, 60, 60, 60 };
	std::vector<Vec3> thrown_from{ Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15) };
	std::vector<float> time_to_player{ 5, 6, 6, 5, 9, 9, 9 };
	std::vector<Vec3> offset_from_player{ Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0) };
	int already_thrown = 0;
};

class ThrowingScheduleBossFight {
public:
	std::vector<float> throw_at{ 0, 6, 12, 18, 24, 
		30, 36, 36, 42, 42,
	
	    160, 160, 160, 160, 160,
	    184,
	    
	    232, 242, 252, 262, 272};
	std::vector<Vec3> thrown_from{ Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1), 
		Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1),
	
	    Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1),
	    Vec3(0, 0, 1),
	    
		Vec3(0, 0, 1), Vec3(-1, 0, 1), Vec3(1, 0, 1), Vec3(-15, 0, 15), Vec3(-1, 0, 1), };
	std::vector<float> time_to_player{ 6, 6, 6, 6, 6, 
		6, 6, 6, 6, 6,
	
		6, 6, 6, 6, 6,
	    6,
	
	    10, 10, 10, 10, 10};
	std::vector<Vec3> offset_from_player{ Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0), 
		Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0),

		Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0),
	    Vec3(0, 0, 0),
	
		Vec3(0, 0, 0), Vec3(-1, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 0), Vec3(-1, 0, 0) };
	int already_thrown = 0;
};