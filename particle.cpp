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
	Vec3f rotation_axis = this->ang_vel;
	cout << "ang vel is " << this->ang_vel << endl;
	if(rotation_axis.length2()!=0) rotation_axis.normalize();
	float rotation_degree = (this->ang_vel*(1.0f/30.0f)).length();
	this->qt = Vec4f(cos(rotation_degree/2),rotation_axis[0]*sin(rotation_degree/2),rotation_axis[1]*sin(rotation_degree/2),rotation_axis[2]*sin(rotation_degree/2));// ^ this->qt;
	this->qt.normalize();
	float s =  this->qt[0];
	float q1 = this->qt[1];
	float q2 = this->qt[2];
	float q3 = this->qt[3];
	cout << "qt is " << this->qt << endl;
	Mat4f qt_rotate( 	1-2*q2*q2-2*q3*q3,	2*q1*q2-2*s*q3,		2*q1*q3+2*s*q2, 	0,
				2*q1*q2+2*s*q3,		1-2*q1*q1-2*q3*q3,	2*q2*q3-2*s*q1,		0,
				2*q1*q3-2*s*q2,		2*q2*q3+2*s*q1,		1-2*q1*q1-2*q2*q2,	0,
				0,			0,			0,			1);
	
	//while(b!=e){
	//	b->pos += this->c_vel *(1.0f/30.0f);
	//	b++;
	//}
	//b = pc.begin();
	Vec3f delta = this->c_vel *(1.0f/30.0f);
	Mat4f mat_trans = Mat4f::createTranslation(delta[0],delta[1],delta[2]);
	c_pos += this->c_vel*(1.0f/30.0f);
	while(b!=e){
		Vec3f temp1 = b->pos-this->c_pos-Vec3f(0,0,0);
		Vec4f temp2(temp1[0],temp1[1],temp1[2],1);
		Vec4f temp3 = (qt_rotate * mat_trans)*(temp2);
		b->pos = Vec3f(temp3[0],temp3[1],temp3[2]) + this->c_pos+Vec3f(0,0,0);
		b++;
	}
}
