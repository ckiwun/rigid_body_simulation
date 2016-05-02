#pragma once
#pragma warning(disable : 4786)

#include <functional>
#include <iostream>
#include <cmath>
#include "vec.h"

enum ParticleType{
	GRAVITY,SMOKE,OBJECT
};

class Particle{
public:
	Particle(void) = default;
	Particle(const Vec3f& x,float m,int t);
	
	int type;
	Vec3f pos;
	float mass;
};

class Particle_Frame{
public:
	Particle_Frame(void);
	Particle_Frame(const std::vector<Particle> new_p,const float & new_ts);
	float ts;//timestamp
	std::vector<Particle> p;
};

class ParticleObject{
public:
	ParticleObject(void) = default;
	ParticleObject(ParticleObject const& that);
	ParticleObject(std::vector<Particle> const& t_pc,int const& t_type,Vec3f const& t_c_pos, Vec3f const& t_c_vel, Vec3f const& t_ang_vel);
	~ParticleObject(void) = default;
	std::vector<Particle> pc;
	int type;
	Vec3f c_pos;//center position
	Vec3f c_vel;//center velocity
	Vec3f ang_vel;//angular velocity
	bool has_collide;
	double last_update_time;

	void update_particle(void);
};
