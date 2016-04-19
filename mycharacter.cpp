#include "mymodel.h"




// We need to make a creator function, mostly because of
// nasty API stuff that we'd rather stay away from.
ModelerView* createMyModel(int x, int y, int w, int h, char *label)
{
	return new MyModel(x, y, w, h, label);
}

// We are going to override (is that the right word?) the draw()
// method of ModelerView to draw out MyModel
void MyModel::draw()
{
	// This call takes care of a lot of the nasty projection 
	// matrix stuff.  Unless you want to fudge directly with the 
	// projection matrix, don't bother with this ...

	ModelerView::draw();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ChooseCostume(VAL(COSTUME));

	
		LocateBody(VAL(XPOS), VAL(YPOS), VAL(ZPOS), VAL(ROTATE));
		RotateHead(0, VAL(NECK) + 180, 0);
		int SType = VAL(SLASH_TYPE);
		switch (SType){
		case VERTICAL:
			RotateExcalibur(0.0, 90.0, 2*LOWER_ARM_SLASH*VAL(SLASH) < 40.0 ? 2*LOWER_ARM_SLASH*VAL(SLASH) : 40.0, "yzx");
			RotateRightUpperArm(1.5*VAL(SLASH), 0.0, 0.0);
			RotateRightLowerArm(LOWER_ARM_SLASH*VAL(SLASH), 0.0, 2*LOWER_ARM_SLASH*VAL(SLASH) < 40.0 ? -2*LOWER_ARM_SLASH*VAL(SLASH) : -40.0, "zxy");
			RotateLeftUpperArm(1.5*VAL(SLASH), 0.0, 0.0);
			RotateLeftLowerArm(LOWER_ARM_SLASH*VAL(SLASH), 0.0, 2*LOWER_ARM_SLASH*VAL(SLASH) < 40.0 ? 2*LOWER_ARM_SLASH*VAL(SLASH) : 40.0, "zxy");
			break;
		case STAND:
			RotateExcalibur(VAL(SLASH)>75?-VAL(SLASH)*2+25:-VAL(SLASH)*125.0/75.0, 0.0,VAL(SLASH) < 40.0 ? VAL(SLASH) : 40.0 , "xyz");
			RotateRightUpperArm(1.1*VAL(SLASH), 0.0, 0.0);
			RotateRightLowerArm(0.6*VAL(SLASH), 0.0, VAL(SLASH) < 40.0 ? -VAL(SLASH) : -40.0, "zxy");
			RotateLeftUpperArm(1.1*VAL(SLASH), 0.0, 0.0);
			RotateLeftLowerArm(0.6*VAL(SLASH), 0.0, VAL(SLASH) < 40.0 ? VAL(SLASH) : 40.0, "zxy");
			break;
		case SKEW:
			if (VAL(SLASH) < 45.0){
				RotateExcalibur(0.0, 0.0, 70.0 * VAL(SLASH) / 45.0, "xyz");
				RotateRightUpperArm(90.0*VAL(SLASH) / 45.0, -30.0 * VAL(SLASH) / 45.0, 0.0, "xyz");
				RotateRightLowerArm(110.0*VAL(SLASH) / 45.0, 30.0*VAL(SLASH) / 45.0, 0.0, "xyz");
				RotateLeftUpperArm(90.0*VAL(SLASH) / 45.0, -45.0*VAL(SLASH) / 45.0, 0.0, "xyz");
				RotateLeftLowerArm(65.0*VAL(SLASH) / 45.0, -15.0*VAL(SLASH) / 45.0, 0.0, "xyz");
			}
			else{
				RotateExcalibur(0.0, 90.0*(VAL(SLASH) - 45.0) / 45.0, 70.0 *(1 - (VAL(SLASH) - 45.0) / 45.0), "xyz");
				RotateRightUpperArm(90.0 - 30.0*(VAL(SLASH) - 45.0) / 45.0, -30.0 + 55.0 * (VAL(SLASH) - 45.0) / 45.0, 0.0, "xyz");
				RotateRightLowerArm(110.0 - 110.0*(VAL(SLASH) - 45.0) / 45.0, 30.0 - 30.0*(VAL(SLASH) - 45.0) / 45.0, 0.0, "xyz");
				RotateLeftUpperArm(90.0 - 40.0*(VAL(SLASH) - 45.0) / 45.0, -45.0 + 30.0*(VAL(SLASH) - 45.0) / 45.0, 0.0, "xyz");
				RotateLeftLowerArm(65.0 - 50.0*(VAL(SLASH) - 45.0) / 45.0, -15.0 + 15.0*(VAL(SLASH) - 45.0) / 45.0, 10.0*(VAL(SLASH) - 45.0) / 45.0, "xyz");
				break;
			}
		case SINGLE:
			if (VAL(SLASH) < 45.0){
				RotateExcalibur(0.0, 0.0, 70.0 * VAL(SLASH) / 45.0, "xyz");
				RotateRightUpperArm(90.0*VAL(SLASH) / 45.0, -30.0 * VAL(SLASH) / 45.0, 0.0, "xyz");
				RotateRightLowerArm(110.0*VAL(SLASH) / 45.0, 30.0*VAL(SLASH) / 45.0, 0.0, "xyz");
			}
			else{
				RotateExcalibur(0.0, -90.0*(VAL(SLASH) - 45.0) / 45.0, 70.0 *(1 - (VAL(SLASH) - 45.0) / 45.0), "xyz");
				RotateRightUpperArm(90.0, -30.0 + 75.0 * (VAL(SLASH) - 45.0) / 45.0, 0.0, "xyz");
				RotateRightLowerArm(110.0 - 110.0*(VAL(SLASH) - 45.0) / 45.0, 30.0 + 60.0*(VAL(SLASH) - 45.0) / 45.0, 0.0, "xyz");
			}
				break;
		}
		
		
	treeRoot->RootRender();
	
	if (valid()){
		ParticleSystem *ps = ModelerApplication::Instance()->GetParticleSystem();
		if (ps != NULL) {
			ps->computeForcesAndUpdateParticles(t);
			ps->drawParticles(t);
		}
	}

	endDraw();
}

int main()
{
	// Initialize the controls
	// Constructor is ModelerControl(name, minimumvalue, maximumvalue, 
	// stepsize, defaultvalue)
	ModelerControl controls[NUMCONTROLS];
	controls[XPOS] = ModelerControl("X Position", -5, 5, 0.1f, 0);
	controls[YPOS] = ModelerControl("Y Position", 0, 5, 0.1f, 0);
	controls[ZPOS] = ModelerControl("Z Position", -5, 5, 0.1f, 0);
	controls[SWORD_LENGTH] = ModelerControl("Sword Length", 0.0, 1.0, 0.1f, 1);
	controls[ROTATE] = ModelerControl("Rotate", -180, 180, 1, 0);
	controls[SLASH] = ModelerControl("Slash", 0, 90, 1, 45);
	controls[COSTUME] = ModelerControl("Costume", 1, 3, 1, 1);
	controls[SLASH_TYPE] = ModelerControl("Slash Type", 1, SLASH_TYPE_NUM-1, 1, 1);
	controls[NECK] = ModelerControl("Head direction", -180, 180, 1, 0);
	controls[PARTICLE_GROUND] = ModelerControl("Ground particle enable", 0, 1, 1, 0);
	controls[PARTICLE_PREPARE] = ModelerControl("Excalibur prepare enable", 0, 1, 1, 0);
	controls[PARTICLE_CAST] = ModelerControl("Excalibur enable", 0, 1, 1, 1);
	controls[PARTICLE_BURST] = ModelerControl("Burst particle enable", 0, 1, 1, 0);
	controls[PARTICLE_AIR] = ModelerControl("Invisible air enable", 0, 1, 1, 0);


	srand(time(0));


	ModelerApplication::Instance()->Init(&createMyModel, controls, NUMCONTROLS);

	MyModel::InitializeParticleSystem();

	return ModelerApplication::Instance()->Run();
}
