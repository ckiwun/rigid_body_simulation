// The sample robotarm model.  You should build a file
// very similar to this for when you make your model.
#pragma warning (disable : 4305)
#pragma warning (disable : 4244)
#pragma warning(disable : 4786)
#pragma warning (disable : 4312)


#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include "particleSystem.h"



#include "mat.h"
#include <FL/gl.h>
#include <cstdlib>
#include <iostream>

using namespace std;

#define M_DEFAULT 2.0f
#define M_OFFSET 3.0f
#define P_OFFSET 0.3f
#define MAX_VEL 200
#define MIN_STEP 0.1

#define FPS 30
#define FRAME_PERIOD 1.0f/FPS
#define PCL_UNIT 1.0f

double elapsed_time = 0;
bool particlized = false;

ParticleSystem* ps;
namespace my_util{
	std::default_random_engine random_generator;
	std::function<double(void)> frand = std::bind(std::uniform_real_distribution<double>{}, random_generator);
}
using namespace my_util;


// This is a list of the controls for the RobotArm
// We'll use these constants to access the values 
// of the controls from the user interface.
enum RobotArmControls
{ 
    BOX1_VEL=0, BOX2_VEL, PARTICLE_COUNT, NUMCONTROLS, 
};

void ground(float h);
void base(float h);
void y_box(float h);
Mat4f glGetMatrix(GLenum pname);
Vec3f getWorldPoint(Mat4f matCamXforms);

// To make a RobotArm, we inherit off of ModelerView
class RobotArm : public ModelerView 
{
public:
    RobotArm(int x, int y, int w, int h, char *label) 
        : ModelerView(x,y,w,h,label) {}
    virtual void draw();
};

// We need to make a creator function, mostly because of
// nasty API stuff that we'd rather stay away from.
ModelerView* createRobotArm(int x, int y, int w, int h, char *label)
{ 
    return new RobotArm(x,y,w,h,label); 
}

// We'll be getting the instance of the application a lot; 
// might as well have it as a macro.
#define VAL(x) (ModelerApplication::Instance()->GetControlValue(x))


// Utility function.  Use glGetMatrix(GL_MODELVIEW_MATRIX) to retrieve
//  the current ModelView matrix.
Mat4f glGetMatrix(GLenum pname)
{
    GLfloat m[16];
    glGetFloatv(pname, m);
    Mat4f matCam(m[0],  m[1],  m[2],  m[3],
                            m[4],  m[5],  m[6],  m[7],
                            m[8],  m[9],  m[10], m[11],
                            m[12], m[13], m[14], m[15] );

    // because the matrix GL returns is column major...
    return matCam.transpose();
}





// We are going to override (is that the right word?) the draw()
// method of ModelerView to draw out RobotArm
void RobotArm::draw()
{
	/* pick up the slider values */

	float v1 = VAL( BOX1_VEL );
	float v2 = VAL( BOX2_VEL );
	float pc = VAL( PARTICLE_COUNT );

    // This call takes care of a lot of the nasty projection 
    // matrix stuff
    ModelerView::draw();

    // Save the camera transform that was applied by
    // ModelerView::draw() above.
    // While we're at it, save an inverted copy of this matrix.  We'll
    // need it later.
    Mat4f matCam = glGetMatrix( GL_MODELVIEW_MATRIX );
    //Mat4f matCamInverse = matCam.inverse();



	static GLfloat lmodel_ambient[] = {0.4,0.4,0.4,1.0};

	// define the model
	cout << "elapsed time is " << elapsed_time << endl;

	ground(-0.2);
	glPushMatrix();
		//if(particlized) {
		//	glTranslatef(ps->po[0].c_vel[0]*elapsed_time,ps->po[0].c_vel[1]*elapsed_time,ps->po[0].c_vel[2]*elapsed_time);
		//	glTranslatef(ps->po[0].c_pos[0],ps->po[0].c_pos[1],ps->po[0].c_pos[2]);
		//}
		//base(2.0);
		if(!particlized){ //push particleobject
			std::vector<Particle> pc;
			for(float z=-1.0;z<=1.0;z+=0.5)
			for(float y=-1.0;y<=1.0;y+=0.5)
			for(float x=-1.0;x<=1.0;x+=0.5){
				Vec3f normal;
				if(x==-1) normal += Vec3f(-1,0,0);
				if(x== 1) normal += Vec3f( 1,0,0);
				if(y==-1) normal += Vec3f(0,-1,0);
				if(y== 1) normal += Vec3f(0, 1,0);
				if(z==-1) normal += Vec3f(0,0,-1);
				if(z== 1) normal += Vec3f(0,0, 1);
				normal.normalize();
				pc.emplace_back(Vec3f(x,y+1.0+10,z+5.0),0.002,normal,OBJECT);
			}
			//cout << "box 1 has " << pc.size() << "particles" << endl;
			ps->po.emplace_back(pc,OBJECT,Vec3f(0,1+10,5),Vec3f(0,0,-v1),Vec4f(1,0,0,0),Vec3f(0.4,0.6,-0.5));
		}
	glPopMatrix();
	glPushMatrix();
		//if(particlized) {
		//	glTranslatef(ps->po[1].c_vel[0]*elapsed_time,ps->po[1].c_vel[1]*elapsed_time,ps->po[1].c_vel[2]*elapsed_time);
		//	glTranslatef(ps->po[1].c_pos[0],ps->po[1].c_pos[1],ps->po[1].c_pos[2]);
		//}
		//base(2.0);
		if(!particlized){ //push particleobject
			std::vector<Particle> pc;
			for(float z=-1.0;z<=1.0;z+=0.5)
			for(float y=-1.0;y<=1.0;y+=0.5)
			for(float x=-1.0;x<=1.0;x+=0.5){
				Vec3f normal;
				if(x==-1) normal += Vec3f(-1,0,0);
				if(x== 1) normal += Vec3f( 1,0,0);
				if(y==-1) normal += Vec3f(0,-1,0);
				if(y== 1) normal += Vec3f(0, 1,0);
				if(z==-1) normal += Vec3f(0,0,-1);
				if(z== 1) normal += Vec3f(0,0, 1);
				normal.normalize();
				pc.emplace_back(Vec3f(x,y+1.0+10,z-5.0),0.002,normal,OBJECT);
			}
			//cout << "box 2 has " << pc.size() << "particles" << endl;
			ps->po.emplace_back(pc,OBJECT,Vec3f(0,1+10,-5),Vec3f(0,0,v2),Vec4f(1,0,0,0),Vec3f(-0.4,1.2,-0.2));
		}
	glPopMatrix();

	if(!particlized) particlized = true;//set flag to true
	else {
		auto b = ps->po.begin();
		auto e = ps->po.end();
		while(b!=e){
			b->update_particle();
			b++;
		}
	}


	elapsed_time += FRAME_PERIOD;
	//*** DON'T FORGET TO PUT THIS IN YOUR OWN CODE **/
	//endDraw();
}

void Particlize(){
	//create and push ParticleObject to std::vector<ParticleObject>
}

void ground(float h) 
{
	glDisable(GL_LIGHTING);
	glColor3f(0.65,0.45,0.2);
	glPushMatrix();
	glScalef(40,2,40);
	y_box(h);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void base(float h) {
	setDiffuseColor( 0.25, 0.25, 0.25 );
	setAmbientColor( 0.25, 0.25, 0.25 );
	glPushMatrix();
	glScalef(4.0f, h, 4.0f);
	y_box(1.0f);
	glPopMatrix();
}

void y_box(float h) {

	glBegin( GL_QUADS );

	glNormal3d( 1.0 ,0.0, 0.0);			// +x side
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d( 0.25,0.0,-0.25);
	glVertex3d( 0.25,  h,-0.25);
	glVertex3d( 0.25,  h, 0.25);

	glNormal3d( 0.0 ,0.0, -1.0);		// -z side
	glVertex3d( 0.25,0.0,-0.25);
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d(-0.25,  h,-0.25);
	glVertex3d( 0.25,  h,-0.25);

	glNormal3d(-1.0, 0.0, 0.0);			// -x side
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d(-0.25,  h, 0.25);
	glVertex3d(-0.25,  h,-0.25);

	glNormal3d( 0.0, 0.0, 1.0);			// +z side
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d( 0.25,  h, 0.25);
	glVertex3d(-0.25,  h, 0.25);

	glNormal3d( 0.0, 1.0, 0.0);			// top (+y)
	glVertex3d( 0.25,  h, 0.25);
	glVertex3d( 0.25,  h,-0.25);
	glVertex3d(-0.25,  h,-0.25);
	glVertex3d(-0.25,  h, 0.25);

	glNormal3d( 0.0,-1.0, 0.0);			// bottom (-y)
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d( 0.25,0.0,-0.25);

	glEnd();
}

int main()
{
	ModelerControl controls[NUMCONTROLS ];
	
	controls[BOX1_VEL]	= ModelerControl("box1 initial velocity (v1)", 0, 5, 0.1, 2.0 );
	controls[BOX2_VEL]	= ModelerControl("box2 initial velocity (v2)", 0, 5, 0.1, 2.0 );
	controls[PARTICLE_COUNT]= ModelerControl("particle count (pc)", 0.0, 5.0, 0.1, 5.0 );
    

	// You should create a ParticleSystem object ps here and then
	// call ModelerApplication::Instance()->SetParticleSystem(ps)
	// to hook it up to the animator interface.

	ps = new ParticleSystem();
	ModelerApplication::Instance()->SetParticleSystem(ps);
    ModelerApplication::Instance()->Init(&createRobotArm, controls, NUMCONTROLS);
    return ModelerApplication::Instance()->Run();
}
