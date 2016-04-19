#pragma once
#pragma warning(disable : 4786)

#include <functional>
#include <iostream>
#include <cmath>
#include "vec.h"

enum ParticleType{
	GRAVITY,SMOKE
};

class Particle{
public:
	Particle(void);
	Particle(const Vec3f& x,const Vec3f& v, float m,int t);
	
	int type;
	Vec3f pos;
	Vec3f vel;
	float mass;
};

class Particle_Frame{
public:
	Particle_Frame(void);
	Particle_Frame(const std::vector<Particle> new_p,const float & new_ts);
	float ts;//timestamp
	std::vector<Particle> p;
};
