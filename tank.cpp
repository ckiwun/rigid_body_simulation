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

using namespace std;

#define M_DEFAULT 2.0f
#define M_OFFSET 3.0f
#define P_OFFSET 0.3f
#define MAX_VEL 200
#define MIN_STEP 0.1



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
    TANK_POSITION=0, BASE_ROTATION, UPPER_LENGTH, FORE_LENGTH, PARTICLE_COUNT, NUMCONTROLS, 
};

void ground(float h);
void base(void);
void tire(float size);
void pipe(float pc,float pos);
void headlight(void);
void canon_base(void);
void upper_arm(void);
void forearm(void);
void barrel_cover(void);
void cover(void);
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

	float pos = VAL(TANK_POSITION);
	float theta = VAL(BASE_ROTATION);
	float l1 = VAL( UPPER_LENGTH );
	float l2 = VAL( FORE_LENGTH );
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
	ground(-0.2);
	float tire_size = 0.5;
	glTranslatef(0.0,0.0,pos); //tank move
	glScalef(1.5,1.5,1.5);
	glTranslatef(0.0,0.2,0.0);
	base();
	glPushMatrix();
		glTranslatef(1.0,0.3,0.0);
		tire(tire_size);//left tire
		Mat4f xform = glGetMatrix(GL_MODELVIEW_MATRIX);
		Vec3f dust_pos = Vec3f(1.4+xform[3][0],0.5+xform[3][1],3.0+pos+xform[3][2]);
		Vec3f dust_vel(-0.5+1*frand(),4+2*frand(),4+2*frand());
		float dust_mass = 0.002; //kg, 2 gram per dust particle
		float rand = 30.0*frand();//fps = 30Hz
		if(rand<pc) ps->p.emplace_back(dust_pos,dust_vel,dust_mass,GRAVITY);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-1.0-tire_size,0.3,0.0);
		tire(tire_size);//right tire
		dust_pos = Vec3f(-1.4-tire_size*1.5+xform[3][0],0.5+xform[3][1],3.0+pos+xform[3][2]);
		dust_vel = Vec3f(-0.5+1*frand(),4+2*frand(),4+2*frand());
		rand = 30.0*frand();//fps = 30Hz
		if(rand<pc) ps->p.emplace_back(dust_pos,dust_vel,dust_mass,GRAVITY);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-0.5,1.0,1.0);
		pipe(pc,pos);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-0.5,0.8,-1.85);
		headlight();//left headlight
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.5,0.8,-1.85);
		headlight();//right headlight
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0,1.0,-0.1);
		canon_base();//canon base
		glPushMatrix();
			glTranslatef(0.0,0.2,-0.5);
			upper_arm();
			glPushMatrix();
				glTranslatef(0.0,0.0,1.0);
				forearm();
				glPushMatrix();
					glTranslatef(0.0,0.0,1.5);
					//glTranslatef(-0.08,0.0,0.1);
					//glRotatef(-90.0,0.0,1.0,0.0);
					barrel_cover();
					//glTranslatef(-0.04,0.0,0.0);
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
		glPushMatrix();
			glTranslatef(0.0,0.55,0.0);
			glRotatef(90,1.0,0.0,0.0);
			cover();
		glPopMatrix();
	glPopMatrix();
	//*** DON'T FORGET TO PUT THIS IN YOUR OWN CODE **/
	endDraw();
}

void ground(float h) 
{
	glDisable(GL_LIGHTING);
	glColor3f(0.41,0.61,0.46);
	glPushMatrix();
	glScalef(50,0,50);
	y_box(h);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void base(void){
	setDiffuseColor( 0.37, 0.49, 0.30 );
	setAmbientColor( 0.37, 0.49, 0.30 );
	glScalef(4.0, 1.0, 7.0);
	y_box(1.0f);
	glScalef(0.25,1.0, 1.0/7.0);
}

void tire(float tire_size){
	setDiffuseColor( 0.4, 0.2, 0.0 );
	setAmbientColor( 0.4, 0.2, 0.0 );
	glPushMatrix();
		glTranslatef(0.0, 0.0 , 1.5);
		glRotatef(90.0,0.0,1.0,0.0);
		drawCylinder(tire_size, 0.5, 0.5);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0 , 0.5);
		glRotatef(90.0,0.0,1.0,0.0);
		drawCylinder(tire_size, 0.5, 0.5);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0 , -0.5);
		glRotatef(90.0,0.0,1.0,0.0);
		drawCylinder(tire_size, 0.5, 0.5);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0, 0.0 , -1.5);
		glRotatef(90.0,0.0,1.0,0.0);
		drawCylinder(tire_size, 0.5, 0.5);
	glPopMatrix();
}

void pipe(float pc,float pos){
	glRotatef( -90.0, 1.0, 0.0, 0.0 );
	drawCylinder( 0.6, 0.05, 0.05 );
	glTranslatef(0.6,0.0,0.0);
	Mat4f xform = glGetMatrix(GL_MODELVIEW_MATRIX);
	Vec3f smoke_pos = Vec3f(-0.75+xform[3][0],2.7+xform[3][1],1.5+pos+xform[3][2]);
	Vec3f smoke_vel(-0.1+0.2*frand(),4*frand(),-0.1+0.2*frand());
	float smoke_mass = 0.001; //kg, 1 gram per smoke particle
	static float fps = 30.0;
	float rand = fps*frand();
	if(rand<pc) ps->p.emplace_back(smoke_pos,smoke_vel,smoke_mass,SMOKE);
}

void headlight(void){
	setDiffuseColor( 0.85, 0.85, 0.55 );
	setAmbientColor( 0.95, 0.85, 0.55 );
	setSpecularColor( 1.0,1.0,1.0);
	drawCylinder(0.2,0.2,0.1);
	setSpecularColor(0.0,0.0,0.0);
}

void canon_base(void){
	setDiffuseColor( 0.48, 0.32, 0.23 );
	setAmbientColor( 0.48, 0.32, 0.23 );
	glScalef(2.0, 0.5, 2.0);
	y_box(1.0f); // the rotation base
	glScalef(0.5, 2.0, 0.5);
}

void upper_arm(void){
	setDiffuseColor( 0.22, 0.33, 0.2 );
	setAmbientColor( 0.22, 0.33, 0.2 );
	glRotatef(180.0,0.0,1.0,0.0);
	drawCylinder(1.0,0.12,0.12);
}

void forearm(void){
	setDiffuseColor( 0.22, 0.33, 0.2 );
	setAmbientColor( 0.22, 0.33, 0.2 );
	drawCylinder(1.5,0.08,0.08);
}

void barrel_cover(void){
	setDiffuseColor( 0.38, 0.27, 0.18 );
	setAmbientColor( 0.38, 0.27, 0.18 );
	drawCylinder(0.05,0.08,0.08);
}

void cover(void){
	setDiffuseColor( 0.38, 0.27, 0.18 );
	setAmbientColor( 0.38, 0.27, 0.18 );
	drawCylinder(0.2,0.2,0.2);
}

void y_box(float h) {

	glBegin( GL_QUADS );

	glNormal3d( 1.0 ,0.0, 0.0);			// +x side
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d( 0.25,0.0,-0.25);
	glVertex3d( 0.25,  h,-0.25);
	glVertex3d( 0.25,  h, 0.25);

	glNormal3d( 0.0 ,0.0, -1.0);			// -z side
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

    controls[TANK_POSITION] = ModelerControl("tank position (x)", -10.0, 10.0, 0.5, 0.0 );
    controls[BASE_ROTATION] = ModelerControl("base rotation (theta)", -180.0, 180.0, 0.1, 0.0 );
    controls[UPPER_LENGTH] = ModelerControl("upper arm length (l1)", 1, 10.0, 0.1, 3.0 );
    controls[FORE_LENGTH] = ModelerControl("fore arm length (l2)", 1, 10.0, 0.1, 2.5 );
    controls[PARTICLE_COUNT] = ModelerControl("particle count (pc)", 0.0, 15.0, 0.1, 10.0 );
	
	/*controls[BASE_ROTATION] = ModelerControl("base rotation (theta)", -180.0, 180.0, 0.1, 0.0 );
    controls[LOWER_TILT] = ModelerControl("lower arm tilt (phi)", 15.0, 95.0, 0.1, 55.0 );
    controls[UPPER_TILT] = ModelerControl("upper arm tilt (psi)", 0.0, 135.0, 0.1, 30.0 );
	controls[CLAW_ROTATION] = ModelerControl("claw rotation (cr)", -30.0, 180.0, 0.1, 0.0 );
    controls[BASE_LENGTH] = ModelerControl("base height (h1)", 0.5, 10.0, 0.1, 0.8 );
    controls[LOWER_LENGTH] = ModelerControl("lower arm length (h2)", 1, 10.0, 0.1, 3.0 );
    controls[UPPER_LENGTH] = ModelerControl("upper arm length (h3)", 1, 10.0, 0.1, 2.5 );
    controls[PARTICLE_COUNT] = ModelerControl("particle count (pc)", 0.0, 5.0, 0.1, 5.0 );*/
    

	// You should create a ParticleSystem object ps here and then
	// call ModelerApplication::Instance()->SetParticleSystem(ps)
	// to hook it up to the animator interface.
	ps = new ParticleSystem();
	//ps->p.push_back(Particle(Vec3f(4.0,4.0,4.0),Vec3f(0.0,1.0,0.0)));
	//ps->p.push_back(Particle(Vec3f(4.0,5.0,4.0),Vec3f(0.0,0.0,0.0)));
	ModelerApplication::Instance()->SetParticleSystem(ps);


    ModelerApplication::Instance()->Init(&createRobotArm, controls, NUMCONTROLS);
    return ModelerApplication::Instance()->Run();
}
