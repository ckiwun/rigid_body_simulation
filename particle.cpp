#include "particle.h"
#include "mat.h"
#include <cmath>
#include <iostream>

using namespace std;

Particle::Particle(Vec3f const& x,float m, Vec3f const&n,int t):pos(x),mass(m),normal(n),type(t)
{

}


Particle_Frame::Particle_Frame(void):ts(0),p(std::vector<Particle>{})
{

}

Particle_Frame::Particle_Frame(const std::vector<Particle> new_p,const float& new_ts):ts(new_ts),p(new_p)
{

}

ParticleObject::ParticleObject(ParticleObject const& that):pc(that.pc),type(that.type),c_pos(that.c_pos),c_vel(that.c_vel),qt(that.qt),ang_vel(that.ang_vel),has_collide(false)
{

}

ParticleObject::ParticleObject(std::vector<Particle> const& t_pc,int const& t_type,Vec3f const& t_c_pos, Vec3f const& t_c_vel,Vec4f const& q, Vec3f const& t_ang_vel):pc(t_pc),type(t_type),c_pos(t_c_pos),c_vel(t_c_vel),qt(q),ang_vel(t_ang_vel),has_collide(false)
{

}

void ParticleObject::update_particle(void)
{
	auto b = pc.begin();
	auto e = pc.end();
	cout << "ang vel is " << this->ang_vel << endl;
	Vec3f rotation_axis = this->ang_vel;
	if(rotation_axis.length2()!=0) rotation_axis.normalize();
	float rotation_degree = (this->ang_vel*(1.0f/30.0f)).length();
	this->qt = Vec4f(cos(rotation_degree/2),rotation_axis[0]*sin(rotation_degree/2),rotation_axis[1]*sin(rotation_degree/2),rotation_axis[2]*sin(rotation_degree/2));// ^ this->qt;
	this->qt.normalize();
	float s =  this->qt[0];
	float q1 = this->qt[1];
	float q2 = this->qt[2];
	float q3 = this->qt[3];
	cout << "qt is " << this->qt << endl;
	Mat3f qt_rotate( 	1-2*q2*q2-2*q3*q3,	2*q1*q2+2*s*q3,		2*q1*q3-2*s*q2, 	
				2*q1*q2-2*s*q1,		1-2*q1*q1-2*q3*q3,	2*q2*q3+2*s*q1,		
				2*q1*q3+2*s*q2,		2*q2*q3-2*s*q1,		1-2*q1*q1-2*q2*q2);
	
	while(b!=e){
		b->pos += this->c_vel *(1.0f/30.0f);
		b++;
	}
	b = pc.begin();
	while(b!=e){
		cout << "b pos " << b->pos << endl;
		b->pos = qt_rotate * (b->pos-this->c_pos-Vec3f(0,1,0)) + this->c_pos+Vec3f(0,1,0);
		b++;
	}
	c_pos += this->c_vel*(1.0f/30.0f);
}
