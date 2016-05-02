#include "particle.h"
#include <iostream>

using namespace std;

Particle::Particle(const Vec3f& x,float m, int t):pos(x),mass(m),type(t)
{

}


Particle_Frame::Particle_Frame(void):ts(0),p(std::vector<Particle>{})
{

}

Particle_Frame::Particle_Frame(const std::vector<Particle> new_p,const float& new_ts):ts(new_ts),p(new_p)
{

}

ParticleObject::ParticleObject(ParticleObject const& that):pc(that.pc),type(that.type),c_pos(that.c_pos),c_vel(that.c_vel),ang_vel(that.ang_vel),has_collide(false)
{

}

ParticleObject::ParticleObject(std::vector<Particle> const& t_pc,int const& t_type,Vec3f const& t_c_pos, Vec3f const& t_c_vel, Vec3f const& t_ang_vel):pc(t_pc),type(t_type),c_pos(t_c_pos),c_vel(t_c_vel),ang_vel(t_ang_vel),has_collide(false)
{

}

void ParticleObject::update_particle(void)
{
	auto b = pc.begin();
	auto e = pc.end();
	while(b!=e){
		b->pos = b->pos + this->c_vel*(1.0f/30.0f);
		b++;
	}
}
