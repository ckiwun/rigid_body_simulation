// The sample model.  You should build a file
// very similar to this for when you make your model.

#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include "bitmap.h"
#include "mat.h"
#include "particleSystem.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <string>
#include <ctime>
using namespace std;

#include "modelerdraw.h"

#define USE_COLOR_BLUE 37.0/255.0, 69.0/255.0, 130.0/255.0
#define USE_COLOR_ROYAL_BLUE 72.0/255.0, 118.0/255.0, 255.0/255.0
#define USE_COLOR_DO_BLUE 0.0/255.0, 34.0/255.0, 102.0/255.0
#define USE_COLOR_PURPLE 106.0/255.0, 90.0/255.0, 205.0/255.0
#define USE_COLOR_DARK_BLUE 25.0/255.0, 25.0/255.0, 112.0/255.0
#define USE_COLOR_LIGHT_BLUE 71.0/255.0,66.0/255.0,157.0/255.0
#define USE_COLOR_SILVER 236.0/255.0, 236.0 / 255.0, 234.0/255.0
#define USE_COLOR_GOLD 252.0/255.0,247.0/255.0,135.0/255.0
#define USE_COLOR_EXCALIBUR 210.0/255.0,180.0/255.0,140.0/255.0
#define USE_COLOR_DARK 50.0/255.0,56.0/255.0,73.0/255.0
#define USE_COLOR_BLACK 12.0/255.0,11.0/255.0,18.0/255.0
#define USE_COLOR_BLACK_CLOTHES 46.0/255.0, 43.0/255.0, 30.0/255.0
#define USE_COLOR_DARK_CLOTHES 52.0/255.0,44.0/255.0,65.0/255.0
#define USE_COLOR_DARK_ARMOR 83.0/255.0, 89.0/255.0, 124.0/255.0
#define USE_COLOR_RED 234.0/255.0,65.0/255.0,43.0/255.0
#define USE_COLOR_WHITE 254.0/255.0,254.0/255.0,254.0/255.0
#define USE_COLOR_BODY 255.0/255.0, 238.0/255.0, 212.0/255.0
#define USE_COLOR_DARK_GOLD 211.0/255.0,197.0/255.0,162.0/255.0
#define USE_COLOR_HAIR_BROWN 139.0 / 255.0, 69.0 / 255.0, 19.0 / 255.0
#define USE_COLOR_HAIR_DARK_GOLD 243.0/255.0, 233.0/255.0, 210.0/255.0
#define USE_COLOR_EYE_WHITE 250.0/255.0,250.0/255.0,250.0/255.0
#define USE_COLOR_EYE_DARK_BLACK 0.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0
#define USE_COLOR_EYE_YELLOW 242.0/255.0, 240.0/255.0, 200.0/255.0
#define USE_COLOR_EYE_DARK_YELLOW 156 / 255.0, 126 / 255.0, 71 / 255.0
#define USE_COLOR_PALE 244.0/255.0, 236.0/255.0, 246.0/255.0
#define USE_COLOR_GREEN 150.0/255.0, 255.0/255.0, 10.0/255.0

#define USE_COLOR_DUST 236.0/255.0, 236.0 / 255.0, 234.0/255.0

const double esp = 1e-6;
const int STEPMAX = 50;
enum{
	PRIMITIVE_BOX = 1,
	PRIMITIVE_SPHERE,
	PRIMITIVE_CYLINDER,
	PRIMITIVE_CYLINDER_NO_DISK,
	SHAPE_PARTIAL_CYLINDER,
	SHAPE_TORSO,
	SHAPE_TORSO_LINEAR,
	SHAPE_TORSO_HALF_LINEAR,
	SHAPE_PARTIAL_TORSO,
	PRIMITIVE_TRIANGLE,
	SHAPE_BLADE,
	SHAPE_FOOT,
	SHAPE_HEAD,
	SHAPE_GUARD,
	PRIMITIVE_TYPES
};

enum{
	TYPE_EXCALIBUR = 1,
	TYPE_CALIBURN,
	TYPE_EXCALIBUR_MORGAN,
	TYPE_SWORDS
};

enum{
	COSTUME_SABER = 1,
	COSTUME_SABER_ALTER,
	COSTUME_SABER_LILY
};

enum{
	VERTICAL = 1,
	STAND,
	SKEW,
	SINGLE,
	SLASH_TYPE_NUM
};

enum MyModelControls
{
	XPOS = 0, YPOS, ZPOS, SWORD_LENGTH,
	ROTATE, SLASH, COSTUME, SLASH_TYPE, NECK, 
	PARTICLE_GROUND, PARTICLE_PREPARE, PARTICLE_CAST, PARTICLE_BURST, PARTICLE_AIR,
	NUMCONTROLS
};

#define VAL(x) (ModelerApplication::Instance()->GetControlValue(x))

const GLdouble LOWER_ARM_SLASH=0.8;

const float INVISIBLE_START_RADIUS = 0.4, INVISIBLE_END_RADIUS = 1.5;

class MyModel;

class ModelNode{
private:
	ModelNode *childHead, *brotherNext;
	int primitiveType;
	int swordType;
	int headType;
	GLdouble xAngle, yAngle, zAngle, startPosX, startPosY, startPosZ, xScale, yScale, zScale;
	GLdouble upperScale,middleScale,middleRatio;
	GLdouble colorRed, colorGreen, colorBlue;
	GLdouble colorAlpha;
	GLdouble transX, transY, transZ;
	GLdouble startAngle, endAngle;
	bool disabled;
	bool texAvailable;
	string rotateOrder;
	double clip;
public:
	ModelNode();
	void nodeCreate(ModelNode *father, int thePrimitiveType);
	void setAngle(GLdouble XAngle, GLdouble YAngle, GLdouble ZAngle, char theRotateOrder[] = "xyz");
	void setStartPos(GLdouble X, GLdouble Y, GLdouble Z);
	void setScale(GLdouble X, GLdouble Y, GLdouble Z);
	void setColor(GLdouble r, GLdouble g, GLdouble b);
	void setColorAlpha(GLdouble r, GLdouble g, GLdouble b,GLdouble alpha);
	void setSwordType(int ty);
	void setHeadType(int ty);
	void setTrans(GLdouble X, GLdouble Y, GLdouble Z);
	void cylinderScale(GLdouble theUpperScale, GLdouble theMiddleScale, GLdouble theMiddleRatio);
	void setStartAndEndAngle(GLdouble theStartAngle, GLdouble theEndAngle);
	void setClip(double c);
	void enableNode();
	void disableNode();
	void Render();
	void RootRender();
	void enableTexture();
	void disableTexture();
	static void addGroundParticle();
	static void addFireWorks(Vec3f origin, int deg);
	static AxisForce* invisibleAirStorm;
	static AxisForce* excaliburCast;
	static void spawnParticle(Vec3f POSITION, Vec3f VELOCITY, float MASS, float AGE_LIMIT, float SIZE, ParticleType t);
	static Mat4f cameraMatrix;
	static void modifyAxis(AxisForce* f, Vec3f AxisStart, Vec3f AxisEnd);
	static MyModel *caller;
};

// To make a MyModel, we inherit off of ModelerView
class MyModel : public ModelerView
{
public:
	MyModel(int x, int y, int w, int h, char *label)
		: ModelerView(x, y, w, h, label)
		{
		InitializeTree();
		CostumeKendo();
		instance = this;
	}
	void InitializeTree();
	void CostumeKendo();
	void CostumeKendoAlter();
	void CostumeKendoLily();
	void ChooseCostume(int cost);
	virtual void draw();
	void LocateBody(GLdouble xPos, GLdouble yPos, GLdouble zPos, GLdouble rotateAngle);
	void RotateLeftUpperArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void RotateLeftLowerArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void RotateRightUpperArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void RotateRightLowerArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void RotateExcalibur(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void RotateHead(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[] = "xyz");
	void setExcaliburTransparency(GLdouble alpha);
	static void InitializeParticleSystem();
	static MyModel* instance;
private:
	ModelNode *treeRoot;
	ModelNode upperTorso, lowerTorso, leftUpperArm, leftLowerArm, rightUpperArm, rightLowerArm;
	ModelNode excaliburGrip, excaliburGuard, excaliburBlade, head, leftUpperLeg, leftLowerLeg, rightUpperLeg, rightLowerLeg;
	ModelNode leftShoulder, rightShoulder;
	ModelNode lowerArmor[8],lowerFront[2],lowerOuter[6];
	ModelNode leftFoot, rightFoot;
	ModelNode leftHand, rightHand;
};

Mat4f getModelViewMatrix();

const float GroundSize = 10.0;
