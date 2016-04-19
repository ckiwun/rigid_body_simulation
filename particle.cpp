#include "particle.h"

Particle::Particle(void):pos(Vec3f()),vel(Vec3f())
{

}

Particle::Particle(const Vec3f& x, const Vec3f& v,float m, int t):pos(x),vel(v),mass(m),type(t)
{

}


Particle_Frame::Particle_Frame(void):ts(0),p(std::vector<Particle>{})
{

}

Particle_Frame::Particle_Frame(const std::vector<Particle> new_p,const float& new_ts):ts(new_ts),p(new_p)
{

}
