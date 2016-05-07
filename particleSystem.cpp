#pragma warning(disable : 4786)

#include "particleSystem.h"
#include "modelerdraw.h"
#include "modelerapp.h"
#include "modelerui.h"
#include "particle.h"
#include <GL/glu.h>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>

#define PC_INDEX(x,y,z) x+y*5+z*25

using namespace std;

static float prevT;

/***************
 * Constructors
 ***************/

ParticleSystem::ParticleSystem() 
{	
	baked = false;
}




/*************
 * Destructor
 *************/

ParticleSystem::~ParticleSystem() 
{
	bakeframe.clear();
}


/******************
 * Simulation fxns
 ******************/

/** Start the simulation */
void ParticleSystem::startSimulation(float t)
{
	bake_start_time = t;
	po.clear();
	// These values are used by the UI ...
	// negative bake_end_time indicates that simulation
	// is still progressing, and allows the
	// indicator window above the time slider
	// to correctly show the "baked" region
	// in grey.
	bake_end_time = -1;
	simulate = true;
	dirty = true;

}

/** Stop the simulation */
void ParticleSystem::stopSimulation(float t)
{
	bake_end_time = t;
	// These values are used by the UI
	simulate = false;
	dirty = true;

}

/** Reset the simulation */
void ParticleSystem::resetSimulation(float t)
{
	bake_end_time = -1;
	po.clear();
	baked = false;
	// These values are used by the UI
	simulate = false;
	dirty = true;

}

/** Compute forces and update particles **/
void ParticleSystem::computeForcesAndUpdateParticles(float t)
{
	if(po.empty()) return;
	auto po_b = po.begin();
	auto po_e = po.end();
	int stride = 1;
	cout << boolalpha << po_b->has_collide << endl;
	while(po_b!=po_e){
		auto b = po_b->pc.begin();
		auto e = po_b->pc.end();
		auto comp_po = po_b + stride;
		while(comp_po!=po_e){
			while(b!=e){
				auto comp_b = comp_po->pc.begin();
				auto comp_e = comp_po->pc.end();
				while(comp_b!=comp_e){
					double distance = (b->pos - comp_b->pos).length();
					if(distance<2*0.25) {
						Vec3f normal_unitvec = po_b->c_pos-comp_po->c_pos;
						normal_unitvec.normalize();
						Vec3f po_normal = (po_b->c_vel*(-normal_unitvec))*(-normal_unitvec);
						Vec3f po_tangent = po_b->c_vel - po_normal;
						Vec3f comp_normal = (comp_po->c_vel*normal_unitvec)*normal_unitvec;
						Vec3f comp_tangent = comp_po->c_vel - comp_normal;
						Vec3f delta_po = 1.8 * po_normal;
						Vec3f delta_comp = 1.8 * comp_normal;
						po_normal = -0.8 * po_normal;
						comp_normal = -0.8 * comp_normal;
						po_tangent = 0.8 * po_tangent;
						comp_tangent = 0.8 * comp_tangent;
						po_b->c_vel = po_normal + po_tangent;
						comp_po->c_vel = comp_normal + comp_tangent;
						
						Vec3f col_normal = b->normal-comp_b->normal;
						col_normal.normalize();
						Vec3f col_force = 0.25 * 30 * 1.8 * po_normal;
						Vec3f comp_r = comp_po->c_pos - comp_b->pos;
						comp_po->ang_vel += (col_force^comp_r)*(1.0f/30.f);//not evaluate inertia tensor yet, assume 100
						Vec3f po_r = po_b->c_pos - b->pos;
						po_b->ang_vel += ((-col_force)^po_r)*(1.0f/30.f);
						po_b->update_particle();
						comp_po->update_particle();
					}
					comp_b++;
				}
				b++;
			}
			stride++;
			comp_po = po_b + stride;
		}
		po_b++;
		stride = 1;
	}
	//update center velocity
	po_b = po.begin();
	while(po_b!=po_e){
		if(po_b->c_pos[1]-1<0){//ground
			Vec3f normal_unitvec(0,-1,0);
			Vec3f po_normal = (po_b->c_vel*normal_unitvec)*normal_unitvec;
			Vec3f po_tangent = po_b->c_vel - po_normal;
			po_normal = -0.8 * po_normal;
			po_tangent = 0.8 * po_tangent;//friction
			po_b->c_vel = po_normal + po_tangent;
			po_b->ang_vel *= 0.8;
			po_b->update_particle();
		}
		
		po_b->c_vel = po_b->c_vel + Vec3f(0.0,-9.8,0.0) * (t-prevT);
		cout << "vel is " << po_b->c_vel << endl;
		po_b++;
	}
	po_b = po.begin();
	while(po_b!=po_e){
		if((po_b->c_pos[0]>50)||(po_b->c_pos[0]<-50)||(po_b->c_pos[2]>50)||(po_b->c_pos[2]<-50))
			po.erase(po_b);
		po_b++;
	}
	//if(!simulate||baked) return;
	//auto b = p.begin();
	//auto e = p.end();
	//while(b!=e){
	//	b->pos = b->pos + b->vel*(t-prevT);
	//	if(b->type==GRAVITY) {
	//		if(b->pos[1]<0.1&&(b->pos[0]<50)&&(b->pos[0]>-50)&&(b->pos[2]<50)&&(b->pos[2]>-50)){
	//			b->vel[1] = -0.6 * b->vel[1];
	//			b->vel[0] = 0.6 * b->vel[0];
	//			b->vel[2] = 0.6 * b->vel[2];
	//		}
	//		else b->vel = b->vel + Vec3f(0.0,-9.8,0.0) * (t-prevT);
	//	}
	//	if(b->type==SMOKE) {
	//		float air_buoyancy = (3.14 * 0.1*0.1*0.1 * 1.33) * 1.225;//B=V*D
	//		float aclr = air_buoyancy / b->mass;//F=m*a
	//		b->vel = b->vel + Vec3f(0.0,aclr,0.0) * (t-prevT);
	//	}
	//	if(b->pos[1]>16.0||b->pos[1]<(-0.1)||b->vel.length()<1) {
	//		p.erase(b);
	//		//continue;
	//	}
	//	b++;
	//}
	//if(!baked) bakeParticles(t);
	// Debugging info
	if( t - prevT > .04 )
		printf("(!!) Dropped Frame %lf (!!)\n", t-prevT);
	prevT = t;
}


/** Render particles */
void ParticleSystem::drawParticles(float t)
{
	if(po.empty()) return;
	auto po_b = po.begin();
	auto po_e = po.end();
	while(po_b!=po_e){
		setDiffuseColor( 0.4, 0.2, 0.0 );
		setAmbientColor( 0.4, 0.2, 0.0 );
		auto b = po_b->pc.begin();
		auto e = po_b->pc.end();
		//cout << "member size is " << po_b->pc.size() << endl;
		while(b!=e){
			glPushMatrix();
			glTranslatef(b->pos[0],b->pos[1],b->pos[2]);
			gluSphere(gluNewQuadric(), 0.25,10,10);
			glPopMatrix();
			b++;
		}
		po_b++;
	}
	//if(!simulate&&!baked) return;
	//if(!baked){
	//	auto b = p.begin();
	//	auto e = p.end();
	//	while(b!=e){
	//		if(b->type==SMOKE){
	//			setDiffuseColor( 0.85*b->pos[1]/16, 0.85*b->pos[1]/16, 0.95*b->pos[1]/16 );
	//			setAmbientColor( 0.95*b->pos[1]/16, 0.85*b->pos[1]/16, 0.95*b->pos[1]/16 );
	//		}
	//		else {//GRAVITY
	//			setDiffuseColor( 0.4, 0.2, 0.0 );
	//			setAmbientColor( 0.4, 0.2, 0.0 );
	//		}
	//		glPushMatrix();
	//		glTranslatef(b->pos[0],b->pos[1],b->pos[2]);
	//		gluSphere(gluNewQuadric(), 0.2,10,10);
	//		glPopMatrix();
	//		b++;
	//	}
	//}
	//else{
	//	//find matched frame with t
	//	if(t<bake_start_time||t>bake_end_time) return;
	//	auto b_bf = bakeframe.begin();
	//	auto e_bf = bakeframe.end();
	//	if(b_bf==e_bf) return;
	//	while(b_bf+1!=e_bf){
	//		if(t>=b_bf->ts&&t<(b_bf+1)->ts) break;
	//		b_bf++;
	//	}
	//	//draw the frame
	//	auto b = (b_bf->p).begin();
	//	auto e = (b_bf->p).end();
	//	//cout << " run bake " << endl;
	//	//cout << " t is :" << t << " and bf->ts is :" << b_bf->ts << endl;
	//	while(b!=e){
	//		if(b->type==SMOKE){
	//			setDiffuseColor( 0.85, 0.85, 0.95 );
	//			setAmbientColor( 0.95, 0.85, 0.95 );
	//		}
	//		else {//GRAVITY
	//			setDiffuseColor( 0.4, 0.2, 0.0 );
	//			setAmbientColor( 0.4, 0.2, 0.0 );
	//		}
	//		glPushMatrix();
	//		glTranslatef(b->pos[0],b->pos[1],b->pos[2]);
	//		gluSphere(gluNewQuadric(), 0.2,10,10);
	//		glPopMatrix();
	//		b++;
	//	}
	//}
}




/** Adds the current configuration of particles to
  * your data structure for storing baked particles **/
void ParticleSystem::bakeParticles(float t) 
{
	//Particle_Frame pf(p,t);
	//bakeframe.push_back(pf);
}

/** Clears out your data structure of baked particles */
void ParticleSystem::clearBaked()
{
	bakeframe.clear();
	baked = false;
}




