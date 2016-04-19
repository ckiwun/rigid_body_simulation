#include "mymodel.h"

Mat4f getModelViewMatrix(){
	GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	Mat4f matMV(m[0], m[1], m[2], m[3],
		m[4], m[5], m[6], m[7],
		m[8], m[9], m[10], m[11],
		m[12], m[13], m[14], m[15]);
	return matMV.transpose(); // because the matrix GL returns is column major
}

int hairRand(bool resetSign){
	static int ret = 0;
	int A = 3, B = 23;
	if (resetSign)ret = 0;
	ret = (ret * A + B) % 100;
	return ret;
}

void setDiffuseColorAlpha(float r, float g, float b, float alpha)
{
	ModelerDrawState *mds = ModelerDrawState::Instance();

	mds->m_diffuseColor[0] = (GLfloat)r;
	mds->m_diffuseColor[1] = (GLfloat)g;
	mds->m_diffuseColor[2] = (GLfloat)b;
	mds->m_diffuseColor[3] = (GLfloat)alpha;

	if (mds->m_drawMode == NORMAL)
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mds->m_diffuseColor);
	else
		glColor3f(r, g, b);
}

extern void _dump_current_material(void);
extern void _dump_current_modelview(void);
extern void _setupOpenGl();

void drawCylinderWithoutDisk(double h, double r1, double r2)
{
	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		GLUquadricObj* gluq;

		/* GLU will again do the work.  draw the sides of the cylinder. */
		gluq = gluNewQuadric();
		gluQuadricDrawStyle(gluq, GLU_FILL);
		gluQuadricTexture(gluq, GL_TRUE);
		gluCylinder(gluq, r1, r2, h, divisions, divisions);
		gluDeleteQuadric(gluq);
	}

}

void QuadFunction(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, GLdouble x3, GLdouble y3,GLdouble &A,GLdouble &B,GLdouble &C){
	GLdouble a0, a1, a2;
	if (abs(x1 - x2) < esp || abs(x1 - x3) < esp || abs(x2 - x3) < esp)return;
	a0 = y1 / (x1 - x2) / (x1 - x3);
	a1 = y2 / (x2 - x1) / (x2 - x3);
	a2 = y3 / (x3 - x1) / (x3 - x2);
	A = a0 + a1 + a2;
	B = -(a0*(x2 + x3) + a1*(x1 + x3) + a2*(x1 + x2));
	C = a0*x2*x3 + a1*x1*x3 + a2*x1*x2;

}

const GLdouble DIVIDE_POINT = 1.0/4.0;

void drawTorsoHalfLinear(GLdouble h,GLdouble r1,GLdouble r2,GLdouble rm,GLdouble mratio,GLdouble ACME_DELTA){
	GLdouble hup, hlow;
	hlow = mratio * h;
	hup = h - hlow;
	GLdouble A, B, C;

	QuadFunction(0+hlow*(1-DIVIDE_POINT), r1*(DIVIDE_POINT)+rm*(1-DIVIDE_POINT), hlow, rm*(1-ACME_DELTA), hup*DIVIDE_POINT+hlow, r2*DIVIDE_POINT+rm*(1-DIVIDE_POINT),A,B,C);

	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		GLUquadricObj* gluq;

		/* GLU will again do the work.  draw the sides of the cylinder. */
		gluq = gluNewQuadric();
		gluQuadricDrawStyle(gluq, GLU_FILL);
		gluQuadricTexture(gluq, GL_TRUE);
		glPushMatrix();
		gluCylinder(gluq, r1, r1*(DIVIDE_POINT)+rm*(1 - DIVIDE_POINT), hlow*(1 - DIVIDE_POINT), divisions, divisions);
		glTranslated(0.0, 0.0, hlow*(1 - DIVIDE_POINT));
		for (int i = 0; i < divisions; i++){
			GLdouble rfirst, rsecond, hfirst, hsecond;
			hfirst = i*h*DIVIDE_POINT / divisions + hlow*(1-DIVIDE_POINT);
			hsecond = (i + 1)*h*DIVIDE_POINT / divisions + hlow*(1 - DIVIDE_POINT);
			rfirst = hfirst*hfirst*A + hfirst*B + C;
			rsecond = hsecond*hsecond*A + hsecond*B + C;
			gluCylinder(gluq, rfirst, rsecond, hsecond-hfirst, divisions, 3);
			glTranslated(0.0, 0.0, hsecond - hfirst);
		}
		gluCylinder(gluq, r2*DIVIDE_POINT + rm*(1 - DIVIDE_POINT), r2, hup*(1-DIVIDE_POINT), divisions, divisions);
		glPopMatrix();
		gluDeleteQuadric(gluq);

		if (r1 > 0.0)
		{
			/* if the r1 end does not come to a point, draw a flat disk to
			cover it up. */

			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_INSIDE);
			gluDisk(gluq, 0.0, r1, divisions, divisions);
			gluDeleteQuadric(gluq);
		}

		if (r2 > 0.0)
		{
			/* if the r2 end does not come to a point, draw a flat disk to
			cover it up. */

			/* save the current matrix mode. */
			int savemode;
			glGetIntegerv(GL_MATRIX_MODE, &savemode);

			/* translate the origin to the other end of the cylinder. */
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslated(0.0, 0.0, h);

			/* draw a disk centered at the new origin. */
			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_OUTSIDE);
			gluDisk(gluq, 0.0, r2, divisions, divisions);
			gluDeleteQuadric(gluq);

			/* restore the matrix stack and mode. */
			glPopMatrix();
			glMatrixMode(savemode);
		}
	}

	
}


void drawTorso(GLdouble h, GLdouble r1, GLdouble r2, GLdouble rm, GLdouble mratio){
	GLdouble hup, hlow;
	hlow = mratio * h;
	hup = h - hlow;
	GLdouble A, B, C;

	QuadFunction(0, r1, hlow, rm, hup + hlow, r2, A, B, C);

	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		GLUquadricObj* gluq;

		/* GLU will again do the work.  draw the sides of the cylinder. */
		gluq = gluNewQuadric();
		gluQuadricDrawStyle(gluq, GLU_FILL);
		gluQuadricTexture(gluq, GL_TRUE);
		glPushMatrix();
		for (int i = 0; i < divisions; i++){
			GLdouble rfirst, rsecond, hfirst, hsecond;
			hfirst = i*h / divisions;
			hsecond = (i + 1)*h / divisions;
			rfirst = hfirst*hfirst*A + hfirst*B + C;
			rsecond = hsecond*hsecond*A + hsecond*B + C;
			gluCylinder(gluq, rfirst, rsecond, hsecond - hfirst, divisions, 3);
			glTranslated(0.0, 0.0, hsecond - hfirst);
		}
		glPopMatrix();
		gluDeleteQuadric(gluq);

		if (r1 > 0.0)
		{
			/* if the r1 end does not come to a point, draw a flat disk to
			cover it up. */

			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_INSIDE);
			gluDisk(gluq, 0.0, r1, divisions, divisions);
			gluDeleteQuadric(gluq);
		}

		if (r2 > 0.0)
		{
			/* if the r2 end does not come to a point, draw a flat disk to
			cover it up. */

			/* save the current matrix mode. */
			int savemode;
			glGetIntegerv(GL_MATRIX_MODE, &savemode);

			/* translate the origin to the other end of the cylinder. */
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslated(0.0, 0.0, h);

			/* draw a disk centered at the new origin. */
			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_OUTSIDE);
			gluDisk(gluq, 0.0, r2, divisions, divisions);
			gluDeleteQuadric(gluq);

			/* restore the matrix stack and mode. */
			glPopMatrix();
			glMatrixMode(savemode);
		}
	}


}

void drawTorsoLinear(GLdouble h, GLdouble r1, GLdouble r2, GLdouble rm, GLdouble mratio){
	GLdouble hup, hlow;
	hlow = mratio * h;
	hup = h - hlow;

	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		GLUquadricObj* gluq;

		/* GLU will again do the work.  draw the sides of the cylinder. */
		gluq = gluNewQuadric();
		gluQuadricDrawStyle(gluq, GLU_FILL);
		gluQuadricTexture(gluq, GL_TRUE);
		glPushMatrix();
			gluCylinder(gluq, r1, rm, hlow, divisions, divisions);
			glTranslated(0.0, 0.0, hlow);
			gluCylinder(gluq, rm, r2, hup, divisions, divisions);
		glPopMatrix();
		gluDeleteQuadric(gluq);

		if (r1 > 0.0)
		{
			/* if the r1 end does not come to a point, draw a flat disk to
			cover it up. */

			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_INSIDE);
			gluDisk(gluq, 0.0, r1, divisions, divisions);
			gluDeleteQuadric(gluq);
		}

		if (r2 > 0.0)
		{
			/* if the r2 end does not come to a point, draw a flat disk to
			cover it up. */

			/* save the current matrix mode. */
			int savemode;
			glGetIntegerv(GL_MATRIX_MODE, &savemode);

			/* translate the origin to the other end of the cylinder. */
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslated(0.0, 0.0, h);

			/* draw a disk centered at the new origin. */
			gluq = gluNewQuadric();
			gluQuadricDrawStyle(gluq, GLU_FILL);
			gluQuadricTexture(gluq, GL_TRUE);
			gluQuadricOrientation(gluq, GLU_OUTSIDE);
			gluDisk(gluq, 0.0, r2, divisions, divisions);
			gluDeleteQuadric(gluq);

			/* restore the matrix stack and mode. */
			glPopMatrix();
			glMatrixMode(savemode);
		}
	}


}


void drawPartialCylinder(GLdouble h, GLdouble r1, GLdouble r2, GLdouble startAngle, GLdouble endAngle){
	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;
	startAngle *= 3.1415926 / 180.0;
	endAngle *= 3.1415926 / 180.0;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{	
		divisions *= 3;
		GLdouble rangeAngle = endAngle - startAngle;
		GLdouble upthis[2], uplast[2], downthis[2], downlast[2];
		upthis[0] = r1*cos(startAngle);
		upthis[1] = r1*sin(startAngle);
		downthis[0] = r2*cos(startAngle);
		downthis[1] = r2*sin(startAngle);
		for (int i = 1; i <= divisions; i++){
			GLdouble curAngle = (rangeAngle / divisions)*i + startAngle;
			uplast[0] = upthis[0];
			uplast[1] = upthis[1];
			downlast[0] = downthis[0];
			downlast[1] = downthis[1];
			upthis[0] = r1*cos(curAngle);
			upthis[1] = r1*sin(curAngle);
			downthis[0] = r2*cos(curAngle);
			downthis[1] = r2*sin(curAngle);
			drawTriangle(uplast[0],uplast[1],0.0, downlast[0],downlast[1],h, upthis[0],upthis[1],0.0);
			drawTriangle(upthis[0],upthis[1],0.0, downlast[0],downlast[1],h, downthis[0],downthis[1],h);
		}


	}

}

static GLuint texName,texWind,texWhite,texOrange,texYellow;
void TextureInit(){
	
	int width, height;
	unsigned char *data = readBMP("kendo.bmp", width, height);

	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	data = readBMP("sakura.bmp", width, height);

	glGenTextures(1, &texWind);
	glBindTexture(GL_TEXTURE_2D, texWind);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	data = readBMP("sakura1.bmp", width, height);

	glGenTextures(1, &texWhite);
	glBindTexture(GL_TEXTURE_2D, texWhite);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	data = readBMP("sakura.bmp", width, height);

	glGenTextures(1, &texOrange);
	glBindTexture(GL_TEXTURE_2D, texOrange);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	data = readBMP("sakura.bmp", width, height);

	glGenTextures(1, &texYellow);
	glBindTexture(GL_TEXTURE_2D, texYellow);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}


void drawTriangleTexture(double x1, double y1, double z1,
	double x2, double y2, double z2,
	double x3, double y3, double z3,
	double texRatio,double unitRatio, int ty)
{
	ModelerDrawState *mds = ModelerDrawState::Instance();

	_setupOpenGl();

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"polymesh { points=((%f,%f,%f),(%f,%f,%f),(%f,%f,%f)); faces=((0,1,2));\n", x1, y1, z1, x2, y2, z2, x3, y3, z3);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		double a, b, c, d, e, f;

		/* the normal to the triangle is the cross product of two of its edges. */
		a = x2 - x1;
		b = y2 - y1;
		c = z2 - z1;

		d = x3 - x1;
		e = y3 - y1;
		f = z3 - z1;

		
		glBegin(GL_TRIANGLES);
		glNormal3d(b*f - c*e, c*d - a*f, a*e - b*d);
		if (ty == 1){
			glTexCoord2d(texRatio, 1.0);	glVertex3d(x1, y1, z1);
			glTexCoord2d(texRatio, 0.0);	glVertex3d(x2, y2, z2);
			glTexCoord2d(texRatio+unitRatio, 1.0);	glVertex3d(x3, y3, z3);
		}
		else {
			glTexCoord2d(texRatio, 1.0);	glVertex3d(x1, y1, z1);
			glTexCoord2d(texRatio-unitRatio, 0.0);	glVertex3d(x2, y2, z2);
			glTexCoord2d(texRatio, 0.0);	glVertex3d(x3, y3, z3);
		}
		glEnd();
	}
}

void drawPartialCylinderTexture(GLdouble h, GLdouble r1, GLdouble r2, GLdouble startAngle, GLdouble endAngle){
	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;
	startAngle *= 3.1415926 / 180.0;
	endAngle *= 3.1415926 / 180.0;

	_setupOpenGl();

	if (!texName)TextureInit();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texName);

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		divisions *= 3;
		GLdouble rangeAngle = endAngle - startAngle;
		GLdouble upthis[2], uplast[2], downthis[2], downlast[2];
		upthis[0] = r1*cos(startAngle);
		upthis[1] = r1*sin(startAngle);
		downthis[0] = r2*cos(startAngle);
		downthis[1] = r2*sin(startAngle);
		for (int i = 1; i <= divisions; i++){
			GLdouble curAngle = (rangeAngle / divisions)*i + startAngle;
			uplast[0] = upthis[0];
			uplast[1] = upthis[1];
			downlast[0] = downthis[0];
			downlast[1] = downthis[1];
			upthis[0] = r1*cos(curAngle);
			upthis[1] = r1*sin(curAngle);
			downthis[0] = r2*cos(curAngle);
			downthis[1] = r2*sin(curAngle);
			drawTriangleTexture(uplast[0], uplast[1], 0.0, downlast[0], downlast[1], h, upthis[0], upthis[1], 0.0, (double)i / divisions,1.0/divisions, 1);
			drawTriangleTexture(upthis[0], upthis[1], 0.0, downlast[0], downlast[1], h, downthis[0], downthis[1], h, (double)(i+1) / divisions,1.0/divisions,2 );
		}


	}

	glDisable(GL_TEXTURE_2D);
}

void drawPartialTorso(GLdouble h, GLdouble r1, GLdouble r2, GLdouble rm,GLdouble mratio,GLdouble startAngle, GLdouble endAngle){
	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;
	
	GLdouble hup, hlow;
	hlow = mratio * h;
	hup = h - hlow;
	GLdouble A, B, C;

	QuadFunction(0, r1, hlow, rm, hup + hlow, r2, A, B, C);

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { height=%f; bottom_radius=%f; top_radius=%f;\n", h, r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		glPushMatrix();
		for (int i = 0; i < divisions; i++){
			GLdouble rfirst, rsecond, hfirst, hsecond;
			hfirst = i*h / divisions;
			hsecond = (i + 1)*h / divisions;
			rfirst = hfirst*hfirst*A + hfirst*B + C;
			rsecond = hsecond*hsecond*A + hsecond*B + C;
			drawPartialCylinder(hsecond - hfirst, rfirst, rsecond,startAngle,endAngle);
			glTranslated(0.0, 0.0, hsecond - hfirst);
		}
		glPopMatrix();

	}

}

void drawTorus(GLdouble r1, GLdouble r2){
	ModelerDrawState *mds = ModelerDrawState::Instance();
	int divisions;

	_setupOpenGl();

	switch (mds->m_quality)
	{
	case HIGH:
		divisions = 32; break;
	case MEDIUM:
		divisions = 20; break;
	case LOW:
		divisions = 12; break;
	case POOR:
		divisions = 8; break;
	}

	if (mds->m_rayFile)
	{
		_dump_current_modelview();
		fprintf(mds->m_rayFile,
			"cone { bottom_radius=%f; top_radius=%f;\n", r1, r2);
		_dump_current_material();
		fprintf(mds->m_rayFile, "})\n");
	}
	else
	{
		divisions *= 5;
		
		for (int i = 0; i < divisions; i++){
			GLdouble curAngle = 3.1415926 * 2.0 / divisions * i;
			GLdouble curOx, curOz;
			curOx = r1 * cos(curAngle);
			curOz = r1 * sin(curAngle);
			
			glPushMatrix();
			glTranslated(curOx, 0.0, curOz);
			glRotated(-curAngle / 3.1415926*180.0, 0.0, 1.0, 0.0);
			drawCylinderWithoutDisk(3.1415926 * 2.0 / divisions * (r1+r2)*1.02 , r2, r2);
			glPopMatrix();
		}

	}
}

void drawBlade(int swordType){
	if(swordType==TYPE_EXCALIBUR_MORGAN)setDiffuseColor(USE_COLOR_DARK);
	else setDiffuseColor(USE_COLOR_EXCALIBUR);
	//setAmbientColor(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glScaled(1.0/8.0, 1.0 / 320.0, 1.0);
	//Blade top:(0,320,0)
	//First part:blade
	//Left upper
	//drawTriangle(0.0, 280.0, 1.0, 0.0, 320.0, 0.0, -9.0, 300.0, 0.0);
	//drawTriangle(0.0, 280.0, 1.0, -9.0, 300.0, 0.0, -4.0,275.0,1.0);
	//drawTriangle(-4.0, 275.0, 1.0, -9.0, 300.0, 0.0, -13.5, 23.0, 0.0);
	//drawTriangle(-4.0, 275.0, 1.0, -13.5, 23.0, 0.0, -8.0, 37.0, 1.0);
	//drawTriangle(-8.0, 37.0, 1.0, -13.5, 23.0, 0.0, -8.0, 18.0, 1.0);
	////Right upper
	//drawTriangle(0.0, 280.0, 1.0, 9.0, 300.0, 0.0, 0.0, 320.0, 0.0);
	//drawTriangle(0.0, 280.0, 1.0, 4.0, 275.0, 1.0, 9.0, 300.0, 0.0);
	//drawTriangle(4.0, 275.0, 1.0, 13.5, 23.0, 0.0, 9.0, 300.0, 0.0);
	//drawTriangle(4.0, 275.0, 1.0, 8.0, 37.0, 1.0, 13.5, 23.0, 0.0);
	//drawTriangle(8.0, 37.0, 1.0, 8.0, 18.0, 1.0, 13.5, 23.0, 0.0);
	////Left lower
	//drawTriangle(0.0, 280.0, -1.0, -9.0, 300.0, 0.0, 0.0, 320.0, 0.0);
	//drawTriangle(0.0, 280.0, -1.0, -4.0, 275.0, -1.0, -9.0, 300.0, 0.0);
	//drawTriangle(-4.0, 275.0, -1.0, -13.5, 23.0, 0.0, -9.0, 300.0, 0.0);
	//drawTriangle(-4.0, 275.0, -1.0, -8.0, 37.0, -1.0, -13.5, 23.0, 0.0);
	//drawTriangle(-8.0, 37.0, -1.0, -8.0, 18.0, -1.0, -13.5, 23.0, 0.0);
	////Right lower
	//drawTriangle(0.0, 280.0, -1.0, 0.0, 320.0, 0.0, 9.0, 300.0, 0.0);
	//drawTriangle(0.0, 280.0, -1.0, 9.0, 300.0, 0.0, 4.0, 275.0, -1.0);
	//drawTriangle(4.0, 275.0, -1.0, 9.0, 300.0, 0.0, 13.5, 23.0, 0.0);
	//drawTriangle(4.0, 275.0, -1.0, 13.5, 23.0, 0.0, 8.0, 37.0, -1.0);
	//drawTriangle(8.0, 37.0, -1.0, 13.5, 23.0, 0.0, 8.0, 18.0, -1.0);

	

	//Second part:spine
	switch (swordType){
	case TYPE_EXCALIBUR:
		setDiffuseColor(USE_COLOR_EXCALIBUR);
		
		break;
	case TYPE_EXCALIBUR_MORGAN:
		setDiffuseColor(USE_COLOR_BLACK);
		break;
	case TYPE_CALIBURN:
		setDiffuseColor(USE_COLOR_GOLD);
		break;
	}
	////Up
	//drawTriangle(-4.0, 275.0, 1.0, 4.0, 275.0, 1.0, 0.0, 280.0, 1.0);
	//drawTriangle(0.0, 55.0, 1.0, 4.0, 275.0, 1.0, 0.5, 275.0, 1.0);
	//drawTriangle(0.0, 55.0, 1.0, -0.5, 275.0, 1.0, -4.0, 275.0, 1.0);
	//drawTriangle(0.0, 55.0, 1.0, 8.0, 46.0, 1.0, 4.0, 275.0, 1.0);
	//drawTriangle(0.0, 55.0, 1.0, -4.0, 275.0, 1.0, -8.0, 46.0, 1.0);
	
	glPushMatrix();
	glRotated(-90.0, 1.0, 0.0, 0.0);
	drawCylinder(300.0, 3.0, 4.0);
	glPopMatrix();

	//Down
	//drawTriangle(-4.0, 275.0, -1.0, 0.0, 280.0, -1.0, 4.0, 275.0, -1.0);
	//drawTriangle(0.0, 55.0, -1.0, -4.0, 275.0, -1.0, -0.5, 275.0, -1.0);
	//drawTriangle(0.0, 55.0, -1.0, 0.5, 275.0, -1.0, 4.0, 275.0, -1.0);
	//drawTriangle(0.0, 55.0, -1.0, 4.0, 275.0, -1.0, 8.0, 46.0, -1.0);
	//drawTriangle(0.0, 55.0, -1.0, -8.0, 46.0, -1.0, -4.0, 275.0, -1.0);
	//if (swordType == TYPE_EXCALIBUR_MORGAN)setDiffuseColor(USE_COLOR_RED);
	//drawTriangle(0.0, 55.0, 1.0, 0.5, 275.0, 1.0, -0.5, 275.0, 1.0);
	//drawTriangle(0.0, 55.0, -1.0, -0.5, 275.0, -1.0, 0.5, 275.0, -1.0);

	//Third part:decoration
	//switch (swordType){
	//case TYPE_EXCALIBUR:
	//	setDiffuseColor(USE_COLOR_BLUE);
	//	break;
	//case TYPE_EXCALIBUR_MORGAN:
	//	setDiffuseColor(USE_COLOR_BLACK);
	//	break;
	//case TYPE_CALIBURN:
	//	setDiffuseColor(USE_COLOR_GREEN);
	//	break;
	//}
	////up
	//drawTriangle(-8.0, 46.0, 1.0, 8.0, 46.0, 1.0, 0.0, 51.0, 1.0);
	//
	//drawTriangle(-8.0, 0.0, 1.0, 8.0, 46.0, 1.0, -8.0, 46.0, 1.0);
	//drawTriangle(-8.0, 0.0, 1.0, 8.0, 0.0, 1.0, 8.0, 46.0, 1.0);
	////down
	//drawTriangle(-8.0, 46.0, -1.0, 0.0, 51.0, -1.0, 8.0, 46.0, -1.0);

	//drawTriangle(-8.0, 0.0, -1.0, -8.0, 46.0, -1.0, 8.0, 46.0, -1.0);
	//drawTriangle(-8.0, 0.0, -1.0, 8.0, 46.0, -1.0, 8.0, 0.0, -1.0);
	//switch (swordType){
	//case TYPE_EXCALIBUR:
	//	setDiffuseColor(USE_COLOR_GOLD);
	//	break;
	//case TYPE_EXCALIBUR_MORGAN:
	//	setDiffuseColor(USE_COLOR_RED);
	//	break;
	//case TYPE_CALIBURN:
	//	setDiffuseColor(USE_COLOR_GOLD);
	//	break;
	//}
	//drawTriangle(-8.0, 46.0, 1.0, 0.0, 51.0, 1.0, 0.0, 55.0, 1.0);
	//drawTriangle(0.0, 55.0, 1.0, 0.0, 51.0, 1.0, 8.0, 46.0, 1.0);

	//drawTriangle(-8.0, 46.0, -1.0, 0.0, 55.0, -1.0, 0.0, 51.0, -1.0);
	//drawTriangle(0.0, 55.0, -1.0, 8.0, 46.0, -1.0, 0.0, 51.0, -1.0);

	////Fourth part:Side
	//switch (swordType){
	//case TYPE_EXCALIBUR:
	//	setDiffuseColor(USE_COLOR_GOLD);
	//	break;
	//case TYPE_EXCALIBUR_MORGAN:
	//	setDiffuseColor(USE_COLOR_DARK);
	//	break;
	//case TYPE_CALIBURN:
	//	setDiffuseColor(USE_COLOR_BLUE);
	//	break;
	//}
	////left up
	//drawTriangle(-8.0, 18.0, 1.0, -13.5, 23.0, 0.0, -13.5, 0.0, 0.0);
	//drawTriangle(-8.0, 18.0, 1.0, -13.5, 0.0, 0.0, -8.0, 0.0, 1.0);

	////right up
	//drawTriangle(8.0, 18.0, 1.0, 13.5, 0.0, 0.0, 13.5, 23.0, 0.0);
	//drawTriangle(8.0, 18.0, 1.0, 8.0, 0.0, 1.0, 13.5, 0.0, 0.0);
	////left down

	//drawTriangle(-8.0, 18.0, -1.0, -13.5, 0.0, 0.0, -13.5, 23.0, 0.0);
	//drawTriangle(-8.0, 18.0, -1.0, -8.0, 0.0, -1.0, -13.5, 0.0, 0.0);
	////right down

	//drawTriangle(8.0, 18.0, -1.0, 13.5, 23.0, 0.0, 13.5, 0.0, 0.0);
	//drawTriangle(8.0, 18.0, -1.0, 13.5, 0.0, 0.0, 8.0, 0.0, -1.0);

	glPopMatrix();
	//setAmbientColor(.1f, .1f, .1f);
}

void drawBladeTexture(int swordType){
		setDiffuseColorAlpha(0, 0, 0, 0.8 * VAL(PARTICLE_PREPARE));
		if (!texYellow)TextureInit();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texYellow);
		glPushMatrix();
	glScaled(1.01 / 13.5, 1.0 / 320.0, 1.01);
	//Blade top:(0,320,0)
	//First part:blade
	//Left upper
	drawTriangle(0.0, 280.0, 1.0, 0.0, 320.0, 0.0, -9.0, 300.0, 0.0);
	drawTriangle(0.0, 280.0, 1.0, -9.0, 300.0, 0.0, -4.0, 275.0, 1.0);
	drawTriangle(-4.0, 275.0, 1.0, -9.0, 300.0, 0.0, -13.5, 23.0, 0.0);
	drawTriangle(-4.0, 275.0, 1.0, -13.5, 23.0, 0.0, -8.0, 37.0, 1.0);
	drawTriangle(-8.0, 37.0, 1.0, -13.5, 23.0, 0.0, -8.0, 18.0, 1.0);
	//Right upper
	drawTriangle(0.0, 280.0, 1.0, 9.0, 300.0, 0.0, 0.0, 320.0, 0.0);
	drawTriangle(0.0, 280.0, 1.0, 4.0, 275.0, 1.0, 9.0, 300.0, 0.0);
	drawTriangle(4.0, 275.0, 1.0, 13.5, 23.0, 0.0, 9.0, 300.0, 0.0);
	drawTriangle(4.0, 275.0, 1.0, 8.0, 37.0, 1.0, 13.5, 23.0, 0.0);
	drawTriangle(8.0, 37.0, 1.0, 8.0, 18.0, 1.0, 13.5, 23.0, 0.0);
	//Left lower
	drawTriangle(0.0, 280.0, -1.0, -9.0, 300.0, 0.0, 0.0, 320.0, 0.0);
	drawTriangle(0.0, 280.0, -1.0, -4.0, 275.0, -1.0, -9.0, 300.0, 0.0);
	drawTriangle(-4.0, 275.0, -1.0, -13.5, 23.0, 0.0, -9.0, 300.0, 0.0);
	drawTriangle(-4.0, 275.0, -1.0, -8.0, 37.0, -1.0, -13.5, 23.0, 0.0);
	drawTriangle(-8.0, 37.0, -1.0, -8.0, 18.0, -1.0, -13.5, 23.0, 0.0);
	//Right lower
	drawTriangle(0.0, 280.0, -1.0, 0.0, 320.0, 0.0, 9.0, 300.0, 0.0);
	drawTriangle(0.0, 280.0, -1.0, 9.0, 300.0, 0.0, 4.0, 275.0, -1.0);
	drawTriangle(4.0, 275.0, -1.0, 9.0, 300.0, 0.0, 13.5, 23.0, 0.0);
	drawTriangle(4.0, 275.0, -1.0, 13.5, 23.0, 0.0, 8.0, 37.0, -1.0);
	drawTriangle(8.0, 37.0, -1.0, 13.5, 23.0, 0.0, 8.0, 18.0, -1.0);

	//Second part:spine
	//Up
	drawTriangle(-4.0, 275.0, 1.0, 4.0, 275.0, 1.0, 0.0, 280.0, 1.0);
	drawTriangle(0.0, 55.0, 1.0, 4.0, 275.0, 1.0, 0.5, 275.0, 1.0);
	drawTriangle(0.0, 55.0, 1.0, -0.5, 275.0, 1.0, -4.0, 275.0, 1.0);
	drawTriangle(0.0, 55.0, 1.0, 8.0, 46.0, 1.0, 4.0, 275.0, 1.0);
	drawTriangle(0.0, 55.0, 1.0, -4.0, 275.0, 1.0, -8.0, 46.0, 1.0);
	//Down
	drawTriangle(-4.0, 275.0, -1.0, 0.0, 280.0, -1.0, 4.0, 275.0, -1.0);
	drawTriangle(0.0, 55.0, -1.0, -4.0, 275.0, -1.0, -0.5, 275.0, -1.0);
	drawTriangle(0.0, 55.0, -1.0, 0.5, 275.0, -1.0, 4.0, 275.0, -1.0);
	drawTriangle(0.0, 55.0, -1.0, 4.0, 275.0, -1.0, 8.0, 46.0, -1.0);
	drawTriangle(0.0, 55.0, -1.0, -8.0, 46.0, -1.0, -4.0, 275.0, -1.0);
	drawTriangle(0.0, 55.0, 1.0, 0.5, 275.0, 1.0, -0.5, 275.0, 1.0);
	drawTriangle(0.0, 55.0, -1.0, -0.5, 275.0, -1.0, 0.5, 275.0, -1.0);

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void drawFoot(){
	
	glPushMatrix();
	setDiffuseColor(USE_COLOR_WHITE);
	glScaled(1.0, 1.5, 1.0);
	drawPartialCylinder(1.5, 0.4, 0.2, 0, 180);
	glPopMatrix();
	
	drawTriangle(-0.5, 0.0, 0.0, 0.5, 0.0, 0.0, -0.2, 0.0, 1.5);
	drawTriangle(-0.2, 0.0, 1.5, 0.5, 0.0, 0.0, 0.2, 0.0, 1.5);

	
	glPushMatrix();
	setDiffuseColor(USE_COLOR_WHITE);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	drawTorso(1.5, 0.52, 0.35, 0.4, 0.4);
	glPopMatrix();
}

void drawEye(int li,int ty){
	setDiffuseColor(USE_COLOR_WHITE);
	drawTriangle(4.0, 3.0, 0.0, 3.0, 3.0, 0.0, 3.0, -1.0, 0.0);
	drawTriangle(-3.0, 3.0, 0.0, -4.0, 3.0, 0.0, -3.0, -1.0, 0.0);

	if(ty!=COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_EYE_DARK_BLACK);
	else setDiffuseColor(USE_COLOR_EYE_DARK_YELLOW);
	drawTriangle(3.0, 3.0, 0.0, -3.0, 3.0, 0.0, -3.0, 1.0, 0.0);
	drawTriangle(3.0, 3.0, 0.0, -3.0, 1.0, 0.0, 3.0, 1.0, 0.0);
	drawTriangle(1.0, 1.0, 0.0, -1.0, 1.0, 0.0, -1.0, -1.0, 0.0);
	drawTriangle(1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0);

	setDiffuseColor(USE_COLOR_WHITE);
	drawTriangle(-1.0, 1.0, 0.0, -3.0, 1.0, 0.0, -3.0, 0.0, 0.0);
	drawTriangle(-1.0, 1.0, 0.0, -3.0, 0.0, 0.0, -1.0, 0.0, 0.0);

	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_EYE_WHITE);
	else setDiffuseColor(USE_COLOR_EYE_YELLOW);
	drawTriangle(1.0, 1.0, 0.0, 3.0, 0.0, 0.0, 3.0, 1.0, 0.0);
	drawTriangle(1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 3.0, 0.0, 0.0);
	drawTriangle(-1.0, 0.0, 0.0, -3.0, 0.0, 0.0, -3.0, -1.0, 0.0);
	drawTriangle(-1.0, 0.0, 0.0, -3.0, -1.0, 0.0, -1.0, -1.0, 0.0);
	drawTriangle(1.0, 0.0, 0.0, 3.0, -1.0, 0.0, 3.0, 0.0, 0.0);
	drawTriangle(1.0, 0.0, 0.0, 1.0, -1.0, 0.0, 3.0, -1.0, 0.0);
	drawTriangle(2.0, -3.0, 0.0, -3.0, -1.0, 0.0, -2.0, -3.0, 0.0);
	drawTriangle(2.0, -3.0, 0.0, 3.0,-1.0, 0.0, -3.0, -1.0, 0.0);
	
	setDiffuseColor(USE_COLOR_BLACK);
	if (li){
		drawTriangle(-4.0, 4.5, 0.0, 4.0, 4.0, 0.0, 4.0, 4.2, 0.0);
		drawTriangle(-4.0, 4.8, 0.0, -4.0, 4.5, 0.0, 4.0, 4.0, 0.0);
	}
	else {
		drawTriangle(-4.0, 4.0, 0.0, 4.0, 4.5, 0.0, 4.0, 4.8, 0.0);
		drawTriangle(-4.0, 4.2, 0.0, -4.0, 4.0, 0.0, 4.0, 4.5, 0.0);
	}

}

void drawNose(){
	setDiffuseColor(USE_COLOR_BODY);
	drawTriangle(0.0, 3.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
	drawTriangle(0.0, 3.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
	drawTriangle(0.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
}
void drawMouth(){
	setDiffuseColor(USE_COLOR_BODY);
	drawTriangle(0.0, 1.0, 0.0, -3.0, 0.0, 0.0, 0.0, 0.2, 1.0);
	drawTriangle(0.0, 1.0, 0.0, 0.0, 0.2, 1.0, 3.0, 0.0, 0.0);
	drawTriangle(0.0, 0.2, 1.0, -3.0, 0.0, 0.0, 3.0, 0.0, 0.0);
	drawTriangle(0.0, -1.0, 0.0, 0.0, -0.2, 1.0, -3.0, 0.0, 0.0);
	drawTriangle(0.0, -1.0, 0.0, 3.0, 0.0, 0.0, 0.0, -0.2, 1.0);
	drawTriangle(0.0, -0.2, 1.0, 3.0, 0.0, 0.0, -3.0, 0.0, 0.0);
}

void drawHair(GLdouble mp,GLdouble ep){
	drawTriangle(0.5, 0.0, 0.0, mp - 0.25, -0.5, 0.0, -0.5, 0.0, 0.0);
	drawTriangle(0.5, 0.0, 0.0, mp + 0.25, -0.5, 0.0, mp - 0.25, -0.5, 0.0);
	drawTriangle(mp + 0.25, -0.5, 0.0, mp + ep, -1.0, 0.0, mp - 0.25, -0.5, 0.0);
}

#define UP_LINE_HEIGHT(x) (x*x/5.0)
#define LOW_LINE_HEIGHT(x) (x*x/10.0)*0.3
#define FACE_START_POINT 0.5
#define FACE_INDENT 0.2
#define FACE_BOTTOM 0.0, -0.05, 0.6
#define LOWER_RADIUS 0.6
#define LOWER_HEIGHT 0.25
#define BOTTOM_RADIUS 0.1
void drawHead(int ty){
	hairRand(1);
	glPushMatrix();
	if(ty!=COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_HAIR_BROWN);
	else setDiffuseColor(USE_COLOR_HAIR_DARK_GOLD);
	glTranslated(0.0, 1.0, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	drawCylinder(1 - LOWER_HEIGHT, 1.0, LOWER_RADIUS);
	glPopMatrix();
	glPushMatrix();
	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_BODY);
	else setDiffuseColor(USE_COLOR_PALE);
	glTranslated(0.0, LOWER_HEIGHT, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	drawCylinder(LOWER_HEIGHT+0.01, LOWER_RADIUS, BOTTOM_RADIUS);
	glPopMatrix();

	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_BODY);
	else setDiffuseColor(USE_COLOR_PALE);

	drawTriangle(cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), cos(FACE_START_POINT) - FACE_INDENT, 1.0, sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	drawTriangle(cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS);
	drawTriangle(-cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), -LOWER_RADIUS*cos(FACE_START_POINT) + FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), -cos(FACE_START_POINT) + FACE_INDENT, 1.0, sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	drawTriangle(-cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), -cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS, -LOWER_RADIUS*cos(FACE_START_POINT) + FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	
	
	drawTriangle(cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), cos(FACE_START_POINT) - FACE_INDENT, 1.0, sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	drawTriangle(cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS);
	
	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_BODY);
	else setDiffuseColor(USE_COLOR_DARK_ARMOR);
	drawTriangle(0.0, 0.0, 0.0, FACE_BOTTOM, BOTTOM_RADIUS*cos(FACE_START_POINT), 0.0, BOTTOM_RADIUS*sin(FACE_START_POINT));
	drawTriangle(FACE_BOTTOM, BOTTOM_RADIUS*cos(FACE_START_POINT), 0.0, BOTTOM_RADIUS*sin(FACE_START_POINT), LOWER_RADIUS*cos(FACE_START_POINT), LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT));
	drawTriangle(cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS, LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), FACE_BOTTOM);
	drawTriangle(0.0, 0.0, 0.0, -BOTTOM_RADIUS*cos(FACE_START_POINT), 0.0, BOTTOM_RADIUS*sin(FACE_START_POINT), FACE_BOTTOM);
	drawTriangle(FACE_BOTTOM, -LOWER_RADIUS*cos(FACE_START_POINT), LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT), -BOTTOM_RADIUS*cos(FACE_START_POINT), 0.0, BOTTOM_RADIUS*sin(FACE_START_POINT));
	drawTriangle(-cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS, FACE_BOTTOM, -LOWER_RADIUS*cos(FACE_START_POINT) + FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));

	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_BODY);
	else setDiffuseColor(USE_COLOR_PALE);
	drawTriangle(-cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), -LOWER_RADIUS*cos(FACE_START_POINT) + FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT), -cos(FACE_START_POINT) + FACE_INDENT, 1.0, sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	drawTriangle(-cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), -cos(FACE_START_POINT)* LOWER_RADIUS, LOWER_HEIGHT, sin(FACE_START_POINT)*LOWER_RADIUS, -LOWER_RADIUS*cos(FACE_START_POINT) + FACE_INDENT, LOWER_HEIGHT, LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT));
	
	
	
	GLdouble upidZ = sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT);
	GLdouble lowidZ = LOWER_RADIUS*sin(FACE_START_POINT) + FACE_INDENT / sin(FACE_START_POINT) * cos(FACE_START_POINT);
	int divisions = 30;
	GLdouble upthis[2], lowthis[2], uplast[2], lowlast[2];
	upthis[0]=0.0;
	upthis[1] =  upidZ+ 0.2;
	lowthis[0] = 0.0;
	lowthis[1] = lowidZ+0.03 ;
	for (int i = 1; i <= divisions; i++){
		uplast[0] = upthis[0];
		uplast[1] = upthis[1];
		lowlast[0] = lowthis[0];
		lowlast[1] = lowthis[1];
		upthis[0] = (cos(FACE_START_POINT) - FACE_INDENT) / divisions*i;
		upthis[1] = upidZ + 0.2 - UP_LINE_HEIGHT(upthis[0] / (cos(FACE_START_POINT) - FACE_INDENT));
		lowthis[0] = (LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT) / divisions*i;
		lowthis[1] = lowidZ  +0.03 - LOW_LINE_HEIGHT(lowthis[0] / (LOWER_RADIUS*cos(FACE_START_POINT) - FACE_INDENT));

		if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_BODY);
		else setDiffuseColor(USE_COLOR_PALE);
		drawTriangle(uplast[0], 1.0, uplast[1], lowlast[0], LOWER_HEIGHT, lowlast[1], upthis[0], 1.0, upthis[1]);
		drawTriangle(upthis[0], 1.0, upthis[1], lowlast[0], LOWER_HEIGHT, lowlast[1], lowthis[0], LOWER_HEIGHT, lowthis[1]);
		drawTriangle(lowlast[0], LOWER_HEIGHT, lowlast[1], lowthis[0], LOWER_HEIGHT, lowthis[1], FACE_BOTTOM);
		drawTriangle(-uplast[0], 1.0, uplast[1], -upthis[0], 1.0, upthis[1], -lowlast[0], LOWER_HEIGHT, lowlast[1]);
		drawTriangle(-upthis[0], 1.0, upthis[1], -lowthis[0], LOWER_HEIGHT, lowthis[1], -lowlast[0], LOWER_HEIGHT, lowlast[1]);
		drawTriangle(-lowlast[0], LOWER_HEIGHT, lowlast[1], FACE_BOTTOM, -lowthis[0], LOWER_HEIGHT, lowthis[1]);

		if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_HAIR_BROWN);
		else setDiffuseColor(USE_COLOR_HAIR_DARK_GOLD);
		drawTriangle(-uplast[0], 1.0, uplast[1], 0.0, 1.0, 0.0, -upthis[0], 1.0, upthis[1]);
		drawTriangle(uplast[0], 1.0, uplast[1], upthis[0], 1.0, upthis[1], 0.0, 1.0, 0.0);

		if (!(i % 2)){
			double mp = (double)hairRand(0) / 100 *2.0 - 1.0;
			double ep = (double)hairRand(0) / 100 *4.0 - 2.0;
			if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_HAIR_BROWN);
			else setDiffuseColor(USE_COLOR_HAIR_DARK_GOLD);
			glPushMatrix();
			glTranslated(upthis[0], 1.05, upthis[1]+0.02);
			glScaled(-0.15, 0.4, 0.0);
			drawHair(mp, ep);
			glPopMatrix();
			mp = (double)hairRand(0) / 100 *2.0 - 1.0;
			ep = (double)hairRand(0) / 100 *4.0 - 2.0;
			glPushMatrix();
			glTranslated(-upthis[0], 1.05, upthis[1] + 0.02);
			glScaled(-0.1, 0.4, 0.1);
			drawHair(mp, ep);
			glPopMatrix();
		}
	}
	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_HAIR_BROWN);
	else setDiffuseColor(USE_COLOR_HAIR_DARK_GOLD);
	drawTriangle(upthis[0], 1.0, upthis[1], cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT), 0.0, 1.0, 0.0);
	drawTriangle(-upthis[0], 1.0, upthis[1], 0.0, 1.0, 0.0, -cos(FACE_START_POINT), 1.0, sin(FACE_START_POINT));

	if (ty != COSTUME_SABER_ALTER)setDiffuseColor(USE_COLOR_HAIR_BROWN);
	else setDiffuseColor(USE_COLOR_HAIR_DARK_GOLD);
	divisions = 80;
	for (int i = -divisions/10; i <= divisions+divisions/10; i++){
		GLdouble curAngle = (3.14159 + FACE_START_POINT * 2.0) / divisions * i + 3.14159 - FACE_START_POINT;
		GLdouble curX, curZ;
		curX = cos(curAngle);
		curZ = sin(curAngle);
		GLdouble rotY = (-curAngle*180.0 / 3.14159 + 90.0);
		glPushMatrix();
		glTranslated(curX*1.05, 1.05, curZ*1.05);
		glRotated(rotY, 0.0, 1.0, 0.0);
		if(curZ<0.3&&curZ>esp)glScaled(-0.15, 1.2, 0.1);
		else glScaled(-0.1, 0.7, 0.1);
		double mp = (double)hairRand(0) / 100 *1.0 - 0.5;
		double ep = (double)hairRand(0) / 100 *2.0 - 1.0;
		drawHair(mp, ep);
		glPopMatrix();
	}
	//Top hair
	divisions = 80;
	for (int i = 0; i <= divisions; i++){
		GLdouble curAngle = (3.14159 *2) / divisions * i;
		glPushMatrix(); 
		GLdouble curX, curZ;
		curX = cos(curAngle);
		curZ = sin(curAngle);
		glTranslated(curZ*1.05, 1.05, curX*1.05);
		glRotated(curAngle*180.0 / 3.14159, 0.0, 1.0, 0.0);
		glRotated(105.0, 1.0, 0.0, 0.0);
		glScaled(0.1, 1.2, 0.1);
		double mp = (double)hairRand(0) / 100 *1.0 - 0.5;
		double ep = (double)hairRand(0) / 100 *2.0 - 1.0;
		drawHair(mp, ep);
		glPopMatrix();
	}
	
	glPushMatrix();
	glTranslated(0.0, 1.0, 0.0);
	glRotated(-90, 1.0, 0.0, 0.0);
	drawCylinder(0.3, 1.0, 0);
	glPopMatrix();


	glPushMatrix();
	glTranslated(-0.23, 0.5, 0.87);
	glRotated(10.0, 1.0, 0.0, 0.0);
	glRotated(-10.0,0.0, 1.0, 0.0);
	glScaled(0.04, 0.03, 1.0);
	drawEye(1,ty);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.23, 0.5, 0.87);
	glRotated(10.0, 1.0, 0.0, 0.0);
	glRotated(10.0, 0.0, 1.0, 0.0);
	glScaled(0.04, 0.03, 1.0);
	drawEye(0,ty);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.3, 0.77);
	glScaled(0.07, 0.05, 0.1);
	glRotated(10.0, 1.0, 0.0, 0.0);
	drawNose();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.2, 0.67);
	glScaled(0.05, 0.01, 0.05);
	drawMouth();
	glPopMatrix();

	//VERY IMPORTANT!
	if (ty != COSTUME_SABER_ALTER){
		setDiffuseColor(USE_COLOR_HAIR_BROWN);
		double mp = (double)hairRand(0) / 100 *1.0 - 0.5;
		double ep = (double)hairRand(0) / 100 *2.0 - 1.0;
		glPushMatrix();
		glTranslated(0.0, 1.01, 0.8);
		glRotated(120, 0.0, 0.0, 1.0);
		glScaled(-0.1, 0.8, 0.1);
		drawHair(mp,ep);
		glPopMatrix();
	}
}

void drawSwordGuard(){
	//glPushMatrix();
	//glTranslated(-1.0, 0.0, -1.0);
	//glRotated(-30.0, 0.0, 0.0, 1.0);
	//drawBox(-2.0, 1.0, 2.0);
	//glPopMatrix();

	//glPushMatrix();
	//glTranslated(1.0, 0.0, -1.0);
	//glRotated(30.0, 0.0, 0.0, 1.0);
	//drawBox(2.0, 1.0, 2.0);
	//glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 1.0, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glScaled(1.0, 5.0, 1.0);
	drawCylinder(0.6, 2.0, 2.0);
	glPopMatrix();

	//drawTriangle(-1.0, 0.0, -1.0, -0.5, 0.866, -1.0, 1.0, 0.0, -1.0);
	//drawTriangle(1.0, 0.0, -1.0, -0.5, 0.866, -1.0, 0.5, 0.866, -1.0);

	//drawTriangle(-1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -0.5, 0.866, 1.0);
	//drawTriangle(1.0, 0.0, 1.0, 0.5, 0.866, 1.0, -0.5, 0.866, 1.0);

	//drawTriangle(0.5, 0.866, 1.0, -0.5, 0.866, -1.0, -0.5, 0.866, 1.0);
	//drawTriangle(0.5, 0.866, 1.0, 0.5, 0.866, -1.0, -0.5, 0.866, -1.0);

	//drawTriangle(1.0, 0.0, 1.0, -1.0, 0.0, 1.0, -1.0, 0.0, -1.0);
	//drawTriangle(1.0, 0.0, 1.0, -1.0, 0.0, -1.0, 1.0, 0.0, -1.0);
}

ModelNode::ModelNode(){
	childHead = brotherNext = NULL;
	disabled = false;
	texAvailable = false;
	clip = 1.0;
}
void ModelNode::nodeCreate(ModelNode *father, int thePrimitiveType){
	primitiveType = thePrimitiveType;
	if (father){
		if (father->childHead)brotherNext = father->childHead;
		father->childHead = this;
	}
}

void ModelNode::setAngle(GLdouble XAngle, GLdouble YAngle, GLdouble ZAngle, char theRotateOrder[]){
	xAngle = XAngle;
	yAngle = YAngle;
	zAngle = ZAngle;
	rotateOrder = string(theRotateOrder);
}
void ModelNode::setStartPos(GLdouble X, GLdouble Y, GLdouble Z){
	startPosX = X;
	startPosY = Y;
	startPosZ = Z;
}
void ModelNode::setScale(GLdouble X, GLdouble Y, GLdouble Z){
	xScale = X;
	yScale = Y;
	zScale = Z;
}
void ModelNode::setColor(GLdouble r, GLdouble g, GLdouble b){
	colorRed = r;
	colorGreen = g;
	colorBlue = b;
	colorAlpha = 1.0;
}

void ModelNode::setColorAlpha(GLdouble r, GLdouble g, GLdouble b, GLdouble alpha){
	colorRed = r;
	colorGreen = g;
	colorBlue = b;
	colorAlpha = alpha;
}

void ModelNode::setTrans(GLdouble X, GLdouble Y, GLdouble Z){
	transX = X;
	transY = Y;
	transZ = Z;
}

void ModelNode::cylinderScale(GLdouble theUpperScale, GLdouble theMiddleScale, GLdouble theMiddleRatio){
	upperScale = theUpperScale;
	middleScale = theMiddleScale;
	middleRatio = theMiddleRatio;
}

void ModelNode::setSwordType(int ty){
	swordType = ty;
}

void ModelNode::setHeadType(int ty){
	headType = ty;
}

void ModelNode::setClip(double c){
	clip = c;
}

void ModelNode::setStartAndEndAngle(GLdouble theStartAngle, GLdouble theEndAngle){
	startAngle = theStartAngle;
	endAngle = theEndAngle;
}
void ModelNode::enableNode(){
	disabled = false;
}
void ModelNode::disableNode(){
	disabled = true;
}

void ModelNode::enableTexture(){
	texAvailable = true;
}

void ModelNode::disableTexture(){
	texAvailable = false;
}

void ModelNode::Render(){

	GLdouble vec[4];
	GLdouble *eq;
	if (disabled)return;
	setAmbientColor(.1f, .1f, .1f);
	if (abs(colorAlpha - 1.0) < esp){
		setDiffuseColor(colorRed, colorGreen, colorBlue);//set color
	}
	else {
		setDiffuseColorAlpha(colorRed, colorGreen, colorBlue, colorAlpha);
	}
	glPushMatrix();
	glTranslated(startPosX, startPosY, startPosZ);
	for (int i = 2; i>=0 ; i--){
		if (rotateOrder[i] == 'x')glRotated(xAngle, 1.0, 0.0, 0.0); 
		if (rotateOrder[i] == 'y')glRotated(yAngle, 0.0, 1.0, 0.0);
		if (rotateOrder[i] == 'z')glRotated(zAngle, 0.0, 0.0, 1.0);
	}
	glPushMatrix();
	static int delta = 0;

		switch (primitiveType){
		case PRIMITIVE_BOX:
			glTranslated(transX, transY, transZ);
			drawBox(xScale, yScale, zScale);
			break;
		case PRIMITIVE_SPHERE:
			glTranslated(transX, transY, transZ);
			glScaled(xScale, yScale, zScale);
			drawSphere(1.0);
			break;
		case PRIMITIVE_CYLINDER:
			glRotated(90.0, 1.0, 0.0, 0.0);
			glTranslated(transX, transZ, transY);
			glScaled(xScale, zScale, yScale / abs(yScale));
			drawCylinder(abs(yScale), 1.0, upperScale);
			break;
		case PRIMITIVE_CYLINDER_NO_DISK:
			glRotated(90.0, 1.0, 0.0, 0.0);
			glTranslated(transX, transZ, transY);
			glScaled(xScale, zScale, yScale / abs(yScale));
			drawCylinderWithoutDisk(abs(yScale), 1.0, upperScale);
			break;
		case SHAPE_TORSO:
			glRotated(90.0, 1.0, 0.0, 0.0);
			glTranslated(transX, transZ, transY);
			glScaled(xScale, zScale, yScale / abs(yScale));
			drawTorso(abs(yScale), 1.0, upperScale, middleScale, middleRatio);

			break;
		case SHAPE_TORSO_LINEAR:
			glRotated(90.0, 1.0, 0.0, 0.0);
			glTranslated(transX, transZ, transY);
			glScaled(xScale, zScale, yScale / abs(yScale));
			drawTorsoLinear(abs(yScale), 1.0, upperScale, middleScale, middleRatio);
			break;
		case SHAPE_TORSO_HALF_LINEAR:
			glRotated(90.0, 1.0, 0.0, 0.0);
			glTranslated(transX, transZ, transY);
			glScaled(xScale, zScale, yScale / abs(yScale));
			drawTorsoHalfLinear(abs(yScale), 1.0, upperScale, middleScale, middleRatio, 0.1);
			break;
		case SHAPE_BLADE:
		glTranslated(transX, transY, transZ);
		glScaled(xScale, yScale, zScale);

		vec[0] = 0;
		vec[1] = -1;
		vec[2] = 0;
		vec[3] = VAL(SWORD_LENGTH)*1.1 - 0.1;
		eq = vec;

		delta += 2;

		glClipPlane(GL_CLIP_PLANE0, eq);
		glEnable(GL_CLIP_PLANE0);

		//particle
		for (int i = 0; i < 1; i++) {
			float vx = rand() % 100 / 100.0;
			float vy = rand() % 100 / 100.0;
			float vz = rand() % 100 / 100.0;
			spawnParticle(Vec3f(0, vec[3], 0), Vec3f(vx, vy, vz), 55, 10, 0.08, FIREWORKS);
		}

		for (int i = 0; i < 4; i++) {
			addFireWorks(Vec3f(0, vec[3], 0), i * 90 + delta);
			//float vx = rand() % 100 / 50.0;
			//float vy = rand() % 100 / 10.0 + 3.0;
			//float vz = rand() % 100 / 50.0;
			//spawnParticle(Vec3f(0, vec[3], 0), Vec3f(vx, vy, vz), 1, 2, 0.05, FIREWORKS);
		}

		if (VAL(PARTICLE_PREPARE) > 1e-6){
			for (int i = 0; i < 10; i++){
				float po = rand() % 100 / 100.0;
				float ao = rand() % 100 / 100.0;
				spawnParticle(Vec3f(1.1 - po * 0.3, 0.95 *po, 0), Vec3f(0.0, 0.0, 0.0), 1, ao, 0.03, EXCALIBUR_PREPARE);
				spawnParticle(Vec3f(-1.1 + po * 0.3, 0.95* po, 0), Vec3f(0.0, 0.0, 0.0), 1, ao, 0.03, BURST);
			}
		}

		if (VAL(PARTICLE_CAST) > 1 - 1e-6){
			modifyAxis(excaliburCast, Vec3f(0, 0, 0), Vec3f(0, 1, 0));
			for (int i = 0; i < 5; i++){
				float vx = rand() % 100 / 10.0 - 5.0;
				float vz = rand() % 100 - 50; 
				spawnParticle(Vec3f(0, 0.8, 0), Vec3f(vx, 0, vz), 1, 2, 0.03, EXCALIBUR_CAST);
			}
			spawnParticle(Vec3f(0, 0.8, 0), Vec3f(0, 0, 0), 5, 2, 0.5, EXCALIBUR_CAST);
		}

		drawBlade(swordType);
		drawBladeTexture(swordType);

		glDisable(GL_CLIP_PLANE0);

		break;
	case SHAPE_PARTIAL_CYLINDER:
		glRotated(90.0, 1.0, 0.0, 0.0);
		glTranslated(transX, transZ, transY);
		glScaled(xScale, zScale, yScale / abs(yScale));
		if (texAvailable){
			drawPartialCylinderTexture(abs(yScale), 1.0, upperScale, startAngle, endAngle);
		}
		else drawPartialCylinder(abs(yScale), 1.0, upperScale, startAngle, endAngle);
		break;
	case SHAPE_FOOT:
		glTranslated(transX, transY, transZ);
		glScaled(xScale, yScale, zScale);

		//particle
		for (int i = 0; i < 3; i++) {
			float vx = rand() % 100 / 50.0;
			float vy = rand() % 100 / 60.0;
			float vz = -(rand() % 100 / 10.0);
			float sz = rand() % 100 / 100.0;
			spawnParticle(Vec3f(0, 0, 0), Vec3f(vx, vy, vz), 1, 2, 0.5*sz, DUST);
		}

		drawFoot();
		break;
	case SHAPE_HEAD:
		glTranslated(transX, transY, transZ);
		glScaled(xScale, yScale, zScale);
		drawHead(headType);
		break;
	case SHAPE_GUARD:
		glTranslated(transX, transY, transZ);
		glScaled(xScale, yScale, zScale);

		vec[0] = 0;
		vec[1] = -1;
		vec[2] = 0;
		vec[3] = VAL(SWORD_LENGTH) * 10 - 0.1;
		eq = vec;
		glClipPlane(GL_CLIP_PLANE1, eq);
		glEnable(GL_CLIP_PLANE1);

		drawSwordGuard();

		glDisable(GL_CLIP_PLANE1);

		break;
	}
	glPopMatrix();
	//recursive draw children
	for (ModelNode* mn = childHead; mn; mn = mn->brotherNext){
		mn->Render();
	}
	glPopMatrix();
}



void ModelNode::RootRender(){
	cameraMatrix = getModelViewMatrix();
	Render();

	//Ground particles
	addGroundParticle();
	
}

AxisForce* ModelNode::invisibleAirStorm = NULL;
AxisForce* ModelNode::excaliburCast = NULL;

Mat4f ModelNode::cameraMatrix;

MyModel* ModelNode::caller = NULL;

MyModel* MyModel::instance = NULL;
//Manually initialize tree nodes
void MyModel::InitializeTree(){
	upperTorso.nodeCreate(NULL, SHAPE_TORSO_HALF_LINEAR);
	rightUpperArm.nodeCreate(&upperTorso, SHAPE_TORSO);
	rightLowerArm.nodeCreate(&rightUpperArm, PRIMITIVE_CYLINDER);
	rightShoulder.nodeCreate(&rightUpperArm, SHAPE_TORSO_LINEAR);
	leftUpperArm.nodeCreate(&upperTorso, SHAPE_TORSO);
	leftLowerArm.nodeCreate(&leftUpperArm, PRIMITIVE_CYLINDER);
	leftShoulder.nodeCreate(&leftUpperArm, SHAPE_TORSO_LINEAR);
	excaliburGrip.nodeCreate(&rightHand, SHAPE_TORSO);
	excaliburGuard.nodeCreate(&excaliburGrip, SHAPE_GUARD);
	excaliburBlade.nodeCreate(&excaliburGuard, SHAPE_BLADE);
	lowerTorso.nodeCreate(&upperTorso, PRIMITIVE_CYLINDER_NO_DISK);
	head.nodeCreate(&upperTorso, SHAPE_HEAD);
	leftUpperLeg.nodeCreate(&lowerTorso, SHAPE_TORSO);
	leftLowerLeg.nodeCreate(&leftUpperLeg, SHAPE_TORSO);
	rightUpperLeg.nodeCreate(&lowerTorso, SHAPE_TORSO);
	rightLowerLeg.nodeCreate(&rightUpperLeg, SHAPE_TORSO);
	leftFoot.nodeCreate(&leftLowerLeg, SHAPE_FOOT);
	rightFoot.nodeCreate(&rightLowerLeg, SHAPE_FOOT);
	leftHand.nodeCreate(&leftLowerArm, SHAPE_TORSO);
	rightHand.nodeCreate(&rightLowerArm, SHAPE_TORSO);

	//lowerArmor[0].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	//lowerArmor[1].nodeCreate(&lowerArmor[0], SHAPE_PARTIAL_CYLINDER);
	//lowerArmor[2].nodeCreate(&lowerArmor[1], SHAPE_PARTIAL_CYLINDER); 
	//lowerArmor[3].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	//lowerArmor[4].nodeCreate(&lowerArmor[3], SHAPE_PARTIAL_CYLINDER);
	//lowerArmor[5].nodeCreate(&lowerArmor[4], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[0].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerArmor[1].nodeCreate(&lowerArmor[0], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[2].nodeCreate(&lowerArmor[1], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[3].nodeCreate(&lowerArmor[2], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[4].nodeCreate(&lowerArmor[3], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[5].nodeCreate(&lowerArmor[4], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[6].nodeCreate(&lowerArmor[3], SHAPE_PARTIAL_CYLINDER);
	lowerArmor[7].nodeCreate(&lowerArmor[4], SHAPE_PARTIAL_CYLINDER);

	lowerFront[0].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerFront[1].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[0].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[1].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[2].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[3].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[4].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	lowerOuter[5].nodeCreate(&lowerTorso, SHAPE_PARTIAL_CYLINDER);
	treeRoot = &upperTorso;

	ModelNode::caller = this;

}

void MyModel::CostumeKendo(){

	leftShoulder.nodeCreate(NULL, SHAPE_TORSO_LINEAR);
	rightShoulder.nodeCreate(NULL, SHAPE_TORSO_LINEAR);

	// Do
	upperTorso.setAngle(0.0, 0.0, 0.0);
	upperTorso.setColor(USE_COLOR_DO_BLUE);
	upperTorso.setScale(0.32, 2.5, 0.2);
	upperTorso.setStartPos(0.0, 0.0, 0.0);
	upperTorso.setTrans(0.0, 0, 0.0);//center at neck
	upperTorso.cylinderScale(3.2,4.3,0.5);


	lowerTorso.setAngle(0.0, 0.0, 0.0);
	lowerTorso.setColor(USE_COLOR_BLACK);
	lowerTorso.setScale(0.75, 4.5, 0.47);
	lowerTorso.setStartPos(0.0, -2.5, 0.0);
	lowerTorso.setTrans(0.0, 0.0, 0.0);//center at weist
	lowerTorso.cylinderScale(3.0, 1.0, 0.5);

	leftUpperArm.setAngle(0.0, 0.0, 0.0);
	leftUpperArm.setColor(USE_COLOR_WHITE);
	leftUpperArm.setScale(0.35, 2.2, 0.35);
	leftUpperArm.setStartPos(-1.5, -1.0, 0.0);
	leftUpperArm.setTrans(0.0,0.0,0.0);//center at shoulder
	leftUpperArm.cylinderScale(0.8, 1.2, 0.5);

	leftLowerArm.setAngle(0.0, 0.0, 0.0);
	leftLowerArm.setColor(USE_COLOR_ROYAL_BLUE);
	leftLowerArm.setScale(0.5, 1.8, 0.5);
	leftLowerArm.setStartPos(0.0, -2.0, 0.0);
	leftLowerArm.setTrans(0.0, 0.0, 0.0);
	leftLowerArm.cylinderScale(0.6, 1.0, 0.5);

	leftHand.setAngle(0.0, 0.0, 0.0);
	leftHand.setColor(USE_COLOR_PURPLE);
	leftHand.setScale(0.3, 0.5, 0.3);
	leftHand.setStartPos(0.0, -1.8, 0.0);
	leftHand.setTrans(0.0, 0.0, 0.0);
	leftHand.cylinderScale(0.6, 0.8, 0.7);

	leftShoulder.setAngle(0.0, 0.0, -15.0);
	leftShoulder.setColor(USE_COLOR_WHITE);
	leftShoulder.setScale(0.4, 1.0, 0.4);
	leftShoulder.setStartPos(0.0, 0.0, 0.0);
	leftShoulder.setTrans(0.0, -0.8, 0.0);
	leftShoulder.cylinderScale(1.0, 1.5, 0.4);

	rightUpperArm.setAngle(0.0, 0.0, 0.0);
	rightUpperArm.setColor(USE_COLOR_WHITE);
	rightUpperArm.setScale(0.35, 2.2, 0.35);
	rightUpperArm.setStartPos(1.5, -1.0, 0.0);
	rightUpperArm.setTrans(0.0, 0.0, 0.0);//center at shoulder
	rightUpperArm.cylinderScale(0.8, 1.2, 0.5);

	rightLowerArm.setAngle(0.0, 0.0, 0.0);
	rightLowerArm.setColor(USE_COLOR_ROYAL_BLUE);
	rightLowerArm.setScale(0.5, 1.8, 0.5);
	rightLowerArm.setStartPos(0.0, -2.0, 0.0);
	rightLowerArm.setTrans(0.0, 0.0, 0.0);
	rightLowerArm.cylinderScale(0.6, 1.0, 0.5); 
	
	rightHand.setAngle(0.0, 0.0, 0.0);
	rightHand.setColor(USE_COLOR_PURPLE);
	rightHand.setScale(0.3, 0.5, 0.3);
	rightHand.setStartPos(0.0, -1.8, 0.0);
	rightHand.setTrans(0.0, 0.0, 0.0);
	rightHand.cylinderScale(0.6, 0.8, 0.7);


	rightShoulder.setAngle(0.0, 0.0, 15.0);
	rightShoulder.setColor(USE_COLOR_WHITE);
	rightShoulder.setScale(0.4, 1.0, 0.4);
	rightShoulder.setStartPos(0.0, 0.0, 0.0);
	rightShoulder.setTrans(0.0, -0.8, 0.0);
	rightShoulder.cylinderScale(1.0, 1.5, 0.4);

	excaliburGuard.setAngle(0.0, 0.0, 0.0);
	excaliburGuard.setColor(USE_COLOR_GOLD);
	excaliburGuard.setScale(0.3, -0.2, 0.06);
	excaliburGuard.setStartPos(0.0, 0.0, 0.0);
	excaliburGuard.setTrans(0.0, 0.0, 0.0);

	excaliburBlade.setAngle(0.0, 0.0, 0.0);
	excaliburBlade.setColor(USE_COLOR_EXCALIBUR);
	excaliburBlade.setScale(0.3, -5.0, 0.04);
	excaliburBlade.setStartPos(0.0, -0.17, 0.0);
	excaliburBlade.setTrans(0.0, 0.0, 0.0);
	excaliburBlade.setSwordType(TYPE_EXCALIBUR);

	excaliburGrip.setAngle(0.0, 0.0, 0.0);
	excaliburGrip.setColor(USE_COLOR_BLUE);
	excaliburGrip.setScale(0.07, -1.0, 0.07);
	excaliburGrip.setStartPos(0.0, -0.5, 0.0);
	excaliburGrip.setTrans(0.0, 0.0, 0.0);
	excaliburGrip.cylinderScale(0.8, 0.5, 0.5);

	head.setAngle(0.0, 180.0, 0.0);
	head.setColor(USE_COLOR_BODY);
	head.setScale(0.8, 1.4, 0.8);
	head.setStartPos(0.0, 0.0, 0.0);
	head.setTrans(0.0, 0.1, 0.0);
	head.setHeadType(COSTUME_SABER);

	leftUpperLeg.setAngle(0.0, 0.0, -10.0);
	leftUpperLeg.setColor(USE_COLOR_BLUE);
	leftUpperLeg.setScale(0.4, 2.8, 0.4);
	leftUpperLeg.setStartPos(-0.4, -0.4, 0.0);
	leftUpperLeg.setTrans(0.0, 0.0, 0.0);
	leftUpperLeg.cylinderScale(0.75, 1.0, 0.6);

	rightUpperLeg.setAngle(0.0, 0.0, 10.0);
	rightUpperLeg.setColor(USE_COLOR_BLUE);
	rightUpperLeg.setScale(0.4, 2.8, 0.4);
	rightUpperLeg.setStartPos(0.4, -0.4, 0.0);
	rightUpperLeg.setTrans(0.0, 0.0, 0.0);
	rightUpperLeg.cylinderScale(0.75, 1.0, 0.6);

	leftLowerLeg.setAngle(0.0, 0.0, 10.0);
	leftLowerLeg.setColor(USE_COLOR_WHITE);
	leftLowerLeg.setScale(0.35, 1.5, 0.35);
	leftLowerLeg.setStartPos(0.0, -2.8, 0.0);
	leftLowerLeg.setTrans(0.0, 0.0, 0.0);
	leftLowerLeg.cylinderScale(0.8, 0.9, 0.6);

	rightLowerLeg.setAngle(0.0, 0.0, -10.0);
	rightLowerLeg.setColor(USE_COLOR_WHITE);
	rightLowerLeg.setScale(0.35, 1.5, 0.35);
	rightLowerLeg.setStartPos(0.0, -2.8, 0.0);
	rightLowerLeg.setTrans(0.0, 0.0, 0.0);
	rightLowerLeg.cylinderScale(0.8, 0.9, 0.6);

	// ankles
	leftFoot.setAngle(0.0, 180.0, 0.0);
	leftFoot.setColor(USE_COLOR_WHITE);
	leftFoot.setScale(0.8, 0.7, 0.8);
	leftFoot.setStartPos(0.0, -2.5, 0.0);
	leftFoot.setTrans(0.0, 0.0, 0.0);

	rightFoot.setAngle(0.0, 180.0, 0.0);
	rightFoot.setColor(USE_COLOR_WHITE);
	rightFoot.setScale(0.8, 0.7, 0.8);
	rightFoot.setStartPos(0.0, -2.5, 0.0);
	rightFoot.setTrans(0.0, 0.0, 0.0);


	// Tare
	lowerArmor[0].setAngle(0.0, 0.0, 0.0);
	lowerArmor[0].setColor(USE_COLOR_DARK_BLUE);
	lowerArmor[0].setScale(0.8, 1.0, 0.5);
	lowerArmor[0].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[0].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[0].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[0].setStartAndEndAngle(-30, 30);

	lowerArmor[1].setAngle(0.0, 0.0, 0.0);
	lowerArmor[1].setColor(USE_COLOR_BLACK);
	lowerArmor[1].setScale(0.8, 0.8, 0.5);
	lowerArmor[1].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[1].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[1].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[1].setStartAndEndAngle(30, 60);

	lowerArmor[2].setAngle(0.0, 0.0, 0.0);
	lowerArmor[2].setColor(USE_COLOR_DARK_BLUE);
	lowerArmor[2].setScale(0.8, 1.0, 0.5);
	lowerArmor[2].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[2].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[2].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[2].setStartAndEndAngle(60, 120);

	lowerArmor[3].setAngle(0.0, 0.0, 0.0);
	lowerArmor[3].setColor(USE_COLOR_BLACK);
	lowerArmor[3].setScale(0.8, 0.8, 0.5);
	lowerArmor[3].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[3].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[3].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[3].setStartAndEndAngle(120, 150);

	lowerArmor[4].setAngle(0.0, 0.0, 0.0);
	lowerArmor[4].setColor(USE_COLOR_DARK_BLUE);
	lowerArmor[4].setScale(0.8, 1.0, 0.5);
	lowerArmor[4].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[4].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[4].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[4].setStartAndEndAngle(150, 210);

	lowerArmor[5].setAngle(0.0, 0.0, 0.0);
	lowerArmor[5].setColor(USE_COLOR_BLACK);
	lowerArmor[5].setScale(0.8, 0.8, 0.5);
	lowerArmor[5].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[5].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[5].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[5].setStartAndEndAngle(210, 240);

	lowerArmor[6].setAngle(0.0, 0.0, 0.0);
	lowerArmor[6].setColor(USE_COLOR_DARK_BLUE);
	lowerArmor[6].setScale(0.8, 1.0, 0.5);
	lowerArmor[6].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[6].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[6].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[6].setStartAndEndAngle(240, 300);

	lowerArmor[7].setAngle(0.0, 0.0, 0.0);
	lowerArmor[7].setColor(USE_COLOR_BLACK);
	lowerArmor[7].setScale(0.8, 0.8, 0.5);
	lowerArmor[7].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[7].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[7].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[7].setStartAndEndAngle(300, 330);

	//lowerArmor[1].setAngle(0.0, 0.0, 0.0);
	//lowerArmor[1].setColor(USE_COLOR_DARK_BLUE);
	//lowerArmor[1].setScale(1.5, 1.0, 0.9);
	//lowerArmor[1].setStartPos(0.0, -1.05, 0.0);
	//lowerArmor[1].setTrans(0.0, 0.0, 0.0);
	//lowerArmor[1].cylinderScale(1.4, 1.0, 0.5);
	//lowerArmor[1].setStartAndEndAngle(-70, 70);

	//lowerArmor[2].setAngle(0.0, 0.0, 0.0);
	//lowerArmor[2].setColor(USE_COLOR_DARK_BLUE);
	//lowerArmor[2].setScale(2.1, 1.0, 1.26);
	//lowerArmor[2].setStartPos(0.0, -1.05, 0.0);
	//lowerArmor[2].setTrans(0.0, 0.0, 0.0);
	//lowerArmor[2].cylinderScale(1.3, 1.0, 0.5);
	//lowerArmor[2].setStartAndEndAngle(-70, 70);

	//lowerArmor[3].setAngle(0.0, 0.0, 0.0);
	//lowerArmor[3].setColor(USE_COLOR_DARK_BLUE);
	//lowerArmor[3].setScale(0.8, 1.0, 0.5);
	//lowerArmor[3].setStartPos(0.0, 0.0, 0.0);
	//lowerArmor[3].setTrans(0.0, 0.0, 0.0);//center at weist
	//lowerArmor[3].cylinderScale(1.8, 1.0, 0.5);
	//lowerArmor[3].setStartAndEndAngle(110, 250);

	//lowerArmor[4].setAngle(0.0, 0.0, 0.0);
	//lowerArmor[4].setColor(USE_COLOR_DARK_BLUE);
	//lowerArmor[4].setScale(1.5, 1.0, 0.9);
	//lowerArmor[4].setStartPos(0.0, -1.05, 0.0);
	//lowerArmor[4].setTrans(0.0, 0.0, 0.0);
	//lowerArmor[4].cylinderScale(1.4, 1.0, 0.5);
	//lowerArmor[4].setStartAndEndAngle(110, 250);

	//lowerArmor[5].setAngle(0.0, 0.0, 0.0);
	//lowerArmor[5].setColor(USE_COLOR_DARK_BLUE);
	//lowerArmor[5].setScale(2.1, 1.0, 1.26);
	//lowerArmor[5].setStartPos(0.0, -1.05, 0.0);
	//lowerArmor[5].setTrans(0.0, 0.0, 0.0);
	//lowerArmor[5].cylinderScale(1.3, 1.0, 0.5);
	//lowerArmor[5].setStartAndEndAngle(110, 250);

	// costume: lower front with text
	lowerFront[0].enableNode();


	lowerFront[0].setAngle(0.0, 0.0, 0.0);
	lowerFront[0].setColor(USE_COLOR_BLACK_CLOTHES);
	lowerFront[0].setScale(0.8, 3.5, 0.5);
	lowerFront[0].setStartPos(0.0, 0.0, 0.0);
	lowerFront[0].setTrans(0.0, 0.0, 0.0);
	lowerFront[0].cylinderScale(3.5, 1.0, 0.5);
	lowerFront[0].setStartAndEndAngle(250, 290);
	lowerFront[0].enableTexture();
	
	lowerFront[1].disableNode();

	lowerOuter[0].setAngle(0.0, 0.0, 0.0);
	lowerOuter[0].setColor(USE_COLOR_WHITE);
	lowerOuter[0].setScale(0.8, 4.3, 0.5);
	lowerOuter[0].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[0].setTrans(0.0, 0.0, 0.0);
	lowerOuter[0].cylinderScale(3.7, 1.0, 0.5);
	lowerOuter[0].setStartAndEndAngle(-75, 85);

	lowerOuter[1].enableNode();
	lowerOuter[2].enableNode();

	lowerOuter[1].setAngle(0.0, 0.0, 0.0);
	lowerOuter[1].setColor(USE_COLOR_WHITE);
	lowerOuter[1].setScale(0.8, 3.8, 0.5);
	lowerOuter[1].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[1].setTrans(0.0, 0.0, 0.0);
	lowerOuter[1].cylinderScale(3.4, 1.0, 0.5);
	lowerOuter[1].setStartAndEndAngle(80,100);

	lowerOuter[2].setAngle(0.0, 0.0, 0.0);
	lowerOuter[2].setColor(USE_COLOR_WHITE);
	lowerOuter[2].setScale(0.8, 4.3, 0.5);
	lowerOuter[2].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[2].setTrans(0.0, 0.0, 0.0);
	lowerOuter[2].cylinderScale(3.7, 1.0, 0.5);
	lowerOuter[2].setStartAndEndAngle(95, 255);

	lowerOuter[3].disableNode();
	lowerOuter[4].disableNode();
	lowerOuter[5].disableNode();
	
}
void MyModel::CostumeKendoAlter(){
	CostumeKendo();
	excaliburBlade.setSwordType(TYPE_EXCALIBUR_MORGAN);
	head.setHeadType(COSTUME_SABER_ALTER);

	upperTorso.setColor(USE_COLOR_DARK_ARMOR);
	leftUpperArm.setColor(USE_COLOR_DARK_CLOTHES);
	leftLowerArm.setColor(USE_COLOR_DARK_ARMOR);
	rightUpperArm.setColor(USE_COLOR_DARK_CLOTHES);
	rightLowerArm.setColor(USE_COLOR_DARK_ARMOR);
	leftUpperLeg.setColor(USE_COLOR_DARK_CLOTHES);
	leftLowerLeg.setColor(USE_COLOR_DARK_ARMOR);
	rightUpperLeg.setColor(USE_COLOR_DARK_CLOTHES);
	rightLowerLeg.setColor(USE_COLOR_DARK_ARMOR);
	leftShoulder.setColor(USE_COLOR_DARK_CLOTHES);
	rightShoulder.setColor(USE_COLOR_DARK_CLOTHES);
	leftHand.setColor(USE_COLOR_DARK_ARMOR);
	rightHand.setColor(USE_COLOR_DARK_ARMOR);
	leftFoot.setColor(USE_COLOR_DARK_ARMOR);
	rightFoot.setColor(USE_COLOR_DARK_ARMOR);
	excaliburGrip.setColor(USE_COLOR_DARK);
	excaliburGuard.setColor(USE_COLOR_DARK);
	leftShoulder.cylinderScale(1.0, 1.8, 0.4);
	rightShoulder.cylinderScale(1.0, 1.8, 0.4);

	lowerTorso.setAngle(0.0, 0.0, 0.0);
	lowerTorso.setColor(USE_COLOR_BLACK_CLOTHES);
	lowerTorso.setScale(0.75, 4.0, 0.47);
	lowerTorso.setStartPos(0.0, -2.5, 0.0);
	lowerTorso.setTrans(0.0, 0.0, 0.0);//center at weist
	lowerTorso.cylinderScale(2.7, 1.0, 0.5);

	lowerArmor[0].setAngle(0.0, 0.0, 0.0);
	lowerArmor[0].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[0].setScale(0.8, 0.7, 0.5);
	lowerArmor[0].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[0].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[0].cylinderScale(1.4, 1.0, 0.5);
	lowerArmor[0].setStartAndEndAngle(-70, 70);

	lowerArmor[1].setAngle(0.0, 0.0, 0.0);
	lowerArmor[1].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[1].setScale(1.15, 0.7, 0.7);
	lowerArmor[1].setStartPos(0.0, -0.75, 0.0);
	lowerArmor[1].setTrans(0.0, 0.0, 0.0);
	lowerArmor[1].cylinderScale(1.3, 1.0, 0.5);
	lowerArmor[1].setStartAndEndAngle(-70, 70);

	lowerArmor[2].setAngle(0.0, 0.0, 0.0);
	lowerArmor[2].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[2].setScale(1.5, 0.7, 0.9);
	lowerArmor[2].setStartPos(0.0, -0.75, 0.0);
	lowerArmor[2].setTrans(0.0, 0.0, 0.0);
	lowerArmor[2].cylinderScale(1.25, 1.0, 0.5);
	lowerArmor[2].setStartAndEndAngle(-70, 70);

	lowerArmor[3].setAngle(0.0, 0.0, 0.0);
	lowerArmor[3].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[3].setScale(0.8, 0.7, 0.5);
	lowerArmor[3].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[3].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[3].cylinderScale(1.4, 1.0, 0.5);
	lowerArmor[3].setStartAndEndAngle(110, 250);

	lowerArmor[4].setAngle(0.0, 0.0, 0.0);
	lowerArmor[4].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[4].setScale(1.15, 0.7, 0.7);
	lowerArmor[4].setStartPos(0.0, -0.75, 0.0);
	lowerArmor[4].setTrans(0.0, 0.0, 0.0);
	lowerArmor[4].cylinderScale(1.3, 1.0, 0.5);
	lowerArmor[4].setStartAndEndAngle(110, 250);

	lowerArmor[5].setAngle(0.0, 0.0, 0.0);
	lowerArmor[5].setColor(USE_COLOR_DARK_ARMOR);
	lowerArmor[5].setScale(1.5, 0.7, 0.9);
	lowerArmor[5].setStartPos(0.0, -0.75, 0.0);
	lowerArmor[5].setTrans(0.0, 0.0, 0.0);
	lowerArmor[5].cylinderScale(1.25, 1.0, 0.5);
	lowerArmor[5].setStartAndEndAngle(110, 250);

	lowerFront[0].disableNode();

	lowerOuter[0].setAngle(0.0, 0.0, 0.0);
	lowerOuter[0].setColor(USE_COLOR_DARK_CLOTHES);
	lowerOuter[0].setScale(0.8, 4.5, 0.5);
	lowerOuter[0].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[0].setTrans(0.0, 0.0, 0.0);
	lowerOuter[0].cylinderScale(3.3, 1.0, 0.5);
	lowerOuter[0].setStartAndEndAngle(-75, 255);


	lowerFront[1].disableNode();

	lowerOuter[1].disableNode();
	lowerOuter[2].disableNode();
	lowerOuter[3].disableNode();
	lowerOuter[4].disableNode();
	lowerOuter[5].disableNode();
}
void MyModel::CostumeKendoLily(){
	CostumeKendo();
	excaliburBlade.setSwordType(TYPE_CALIBURN);
	head.setHeadType(COSTUME_SABER_LILY);
	
	leftShoulder.nodeCreate(NULL, SHAPE_TORSO);
	rightShoulder.nodeCreate(NULL, SHAPE_TORSO);

	upperTorso.setColor(USE_COLOR_SILVER);
	lowerTorso.setColor(USE_COLOR_WHITE);
	leftUpperArm.setColor(USE_COLOR_WHITE);
	leftLowerArm.setColor(USE_COLOR_SILVER);
	rightUpperArm.setColor(USE_COLOR_WHITE);
	rightLowerArm.setColor(USE_COLOR_SILVER);
	leftUpperLeg.setColor(USE_COLOR_WHITE);
	leftLowerLeg.setColor(USE_COLOR_SILVER);
	rightUpperLeg.setColor(USE_COLOR_WHITE);
	rightLowerLeg.setColor(USE_COLOR_SILVER);
	leftShoulder.setColor(USE_COLOR_BODY);
	rightShoulder.setColor(USE_COLOR_BODY);
	leftHand.setColor(USE_COLOR_SILVER);
	rightHand.setColor(USE_COLOR_SILVER);
	leftFoot.setColor(USE_COLOR_SILVER);
	rightFoot.setColor(USE_COLOR_SILVER);
	excaliburGuard.setColor(USE_COLOR_DARK_GOLD);
	leftShoulder.setScale(0.3, 1.0, 0.3);
	rightShoulder.setScale(0.3, 1.0, 0.3);
	leftShoulder.cylinderScale(1.05, 1.2, 0.4);
	rightShoulder.cylinderScale(1.05, 1.2, 0.4);

	lowerTorso.setAngle(0.0, 0.0, 0.0);
	lowerTorso.setColor(USE_COLOR_WHITE);
	lowerTorso.setScale(0.75, 4.5, 0.47);
	lowerTorso.setStartPos(0.0, -2.5, 0.0);
	lowerTorso.setTrans(0.0, 0.0, 0.0);//center at weist
	lowerTorso.cylinderScale(4.0, 1.0, 0.5);


	lowerFront[0].enableNode();
	lowerFront[1].enableNode();

	lowerOuter[1].enableNode();
	lowerOuter[2].enableNode();
	lowerOuter[3].enableNode();
	lowerOuter[4].enableNode();
	lowerOuter[5].enableNode();

	lowerArmor[0].setAngle(0.0, 0.0, 0.0);
	lowerArmor[0].setColor(USE_COLOR_SILVER);
	lowerArmor[0].setScale(0.8, 0.8, 0.5);
	lowerArmor[0].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[0].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[0].cylinderScale(2.3, 1.0, 0.5);
	lowerArmor[0].setStartAndEndAngle(-50, 50);

	lowerArmor[1].setAngle(0.0, 0.0, 0.0);
	lowerArmor[1].setColor(USE_COLOR_SILVER);
	lowerArmor[1].setScale(1.0, 0.8, 0.6);
	lowerArmor[1].setStartPos(0.0, -0.5, 0.0);
	lowerArmor[1].setTrans(0.0, 0.0, 0.0);
	lowerArmor[1].cylinderScale(2.0, 1.0, 0.5);
	lowerArmor[1].setStartAndEndAngle(-50, 50);

	lowerArmor[2].setAngle(0.0, 0.0, 0.0);
	lowerArmor[2].setColor(USE_COLOR_SILVER);
	lowerArmor[2].setScale(1.3, 0.8, 0.7);
	lowerArmor[2].setStartPos(0.0, -0.5, 0.0);
	lowerArmor[2].setTrans(0.0, 0.0, 0.0);
	lowerArmor[2].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[2].setStartAndEndAngle(-50, 50);

	lowerArmor[3].setAngle(0.0, 0.0, 0.0);
	lowerArmor[3].setColor(USE_COLOR_SILVER);
	lowerArmor[3].setScale(0.8, 0.8, 0.5);
	lowerArmor[3].setStartPos(0.0, 0.0, 0.0);
	lowerArmor[3].setTrans(0.0, 0.0, 0.0);//center at weist
	lowerArmor[3].cylinderScale(2.3, 1.0, 0.5);
	lowerArmor[3].setStartAndEndAngle(130, 230);

	lowerArmor[4].setAngle(0.0, 0.0, 0.0);
	lowerArmor[4].setColor(USE_COLOR_SILVER);
	lowerArmor[4].setScale(1.0, 0.8, 0.6);
	lowerArmor[4].setStartPos(0.0, -0.5, 0.0);
	lowerArmor[4].setTrans(0.0, 0.0, 0.0);
	lowerArmor[4].cylinderScale(2.0, 1.0, 0.5);
	lowerArmor[4].setStartAndEndAngle(130, 230);

	lowerArmor[5].setAngle(0.0, 0.0, 0.0);
	lowerArmor[5].setColor(USE_COLOR_SILVER);
	lowerArmor[5].setScale(1.3, 0.8, 0.7);
	lowerArmor[5].setStartPos(0.0, -0.5, 0.0);
	lowerArmor[5].setTrans(0.0, 0.0, 0.0);
	lowerArmor[5].cylinderScale(1.8, 1.0, 0.5);
	lowerArmor[5].setStartAndEndAngle(130, 230);

	lowerFront[0].setAngle(0.0, 0.0, 0.0);
	lowerFront[0].setColor(USE_COLOR_BLACK);
	lowerFront[0].setScale(0.8, 1.8, 0.5);
	lowerFront[0].setStartPos(0.0, 0.0, 0.0);
	lowerFront[0].setTrans(0.0, 0.0, 0.0);
	lowerFront[0].cylinderScale(2.4, 1.0, 0.5);
	lowerFront[0].setStartAndEndAngle(250, 265);
	//lowerFront[0].disableTexture();

	lowerFront[1].setAngle(0.0, 0.0, 0.0);
	lowerFront[1].setColor(USE_COLOR_BLACK);
	lowerFront[1].setScale(0.8, 1.8, 0.5);
	lowerFront[1].setStartPos(0.0, 0.0, 0.0);
	lowerFront[1].setTrans(0.0, 0.0, 0.0);
	lowerFront[1].cylinderScale(2.4, 1.0, 0.5);
	lowerFront[1].setStartAndEndAngle(275, 290);

	lowerOuter[0].setAngle(0.0, 0.0, 0.0);
	lowerOuter[0].setColor(USE_COLOR_SILVER);
	lowerOuter[0].setScale(0.8, 3.8, 0.5);
	lowerOuter[0].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[0].setTrans(0.0, 0.0, 0.0);
	lowerOuter[0].cylinderScale(3.6, 1.0, 0.5);
	lowerOuter[0].setStartAndEndAngle(-75, -45);

	lowerOuter[1].setAngle(0.0, 0.0, 0.0);
	lowerOuter[1].setColor(USE_COLOR_SILVER);
	lowerOuter[1].setScale(0.8, 4.2, 0.5);
	lowerOuter[1].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[1].setTrans(0.0, 0.0, 0.0);
	lowerOuter[1].cylinderScale(3.8, 1.0, 0.5);
	lowerOuter[1].setStartAndEndAngle(-35, 35);

	lowerOuter[2].setAngle(0.0, 0.0, 0.0);
	lowerOuter[2].setColor(USE_COLOR_SILVER);
	lowerOuter[2].setScale(0.8, 3.8, 0.5);
	lowerOuter[2].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[2].setTrans(0.0, 0.0, 0.0);
	lowerOuter[2].cylinderScale(3.6, 1.0, 0.5);
	lowerOuter[2].setStartAndEndAngle(45,75);

	lowerOuter[3].setAngle(0.0, 0.0, 0.0);
	lowerOuter[3].setColor(USE_COLOR_SILVER);
	lowerOuter[3].setScale(0.8, 3.8, 0.5);
	lowerOuter[3].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[3].setTrans(0.0, 0.0, 0.0);
	lowerOuter[3].cylinderScale(3.6, 1.0, 0.5);
	lowerOuter[3].setStartAndEndAngle(105, 135);

	lowerOuter[4].setAngle(0.0, 0.0, 0.0);
	lowerOuter[4].setColor(USE_COLOR_SILVER);
	lowerOuter[4].setScale(0.8, 4.2, 0.5);
	lowerOuter[4].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[4].setTrans(0.0, 0.0, 0.0);
	lowerOuter[4].cylinderScale(3.8, 1.0, 0.5);
	lowerOuter[4].setStartAndEndAngle(145, 215);

	lowerOuter[5].setAngle(0.0, 0.0, 0.0);
	lowerOuter[5].setColor(USE_COLOR_SILVER);
	lowerOuter[5].setScale(0.8, 3.8, 0.5);
	lowerOuter[5].setStartPos(0.0, 0.0, 0.0);
	lowerOuter[5].setTrans(0.0, 0.0, 0.0);
	lowerOuter[5].cylinderScale(3.6, 1.0, 0.5);
	lowerOuter[5].setStartAndEndAngle(225, 255);

}

void MyModel::ChooseCostume(int cost){
	switch (cost){
	case COSTUME_SABER:
		CostumeKendo();
		break;
	case COSTUME_SABER_ALTER:
		CostumeKendoAlter();
		break;
	case COSTUME_SABER_LILY:
		CostumeKendoLily();
		break;
	}
}


void MyModel::LocateBody(GLdouble xPos, GLdouble yPos, GLdouble zPos, GLdouble rotateAngle){
	upperTorso.setStartPos(xPos, yPos, zPos);
	upperTorso.setAngle(0.0, rotateAngle, 0.0);
}

void MyModel::RotateLeftUpperArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	leftUpperArm.setAngle(X, Y, Z, theRotateOrder);
}
void MyModel::RotateLeftLowerArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	leftLowerArm.setAngle(X, Y, Z, theRotateOrder);
}
void MyModel::RotateRightUpperArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	rightUpperArm.setAngle(X, Y, Z, theRotateOrder);
}
void MyModel::RotateRightLowerArm(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	rightLowerArm.setAngle(X, Y, Z, theRotateOrder);
}

void MyModel::RotateExcalibur(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	excaliburGrip.setAngle(X, Y, Z, theRotateOrder);
}

void MyModel::setExcaliburTransparency(GLdouble alpha){
	excaliburBlade.setClip(alpha);
}

void MyModel::RotateHead(GLdouble X, GLdouble Y, GLdouble Z, char theRotateOrder[]){
	head.setAngle(X, Y, Z, theRotateOrder);
}

void billBoard(float size, Vec3f v, GLuint texn){
	Mat4f ca = getModelViewMatrix();
	Vec3f cX = ca.inverse()*Vec3f(1, 0, 0) - ca.inverse()*Vec3f(0,0,0);
	cX.normalize();
	Vec3f cY = ca.inverse()*Vec3f(0, 1, 0) - ca.inverse()*Vec3f(0, 0, 0);
	cY.normalize();
	Vec3f cZ = ca.inverse()*Vec3f(0, 0, 1) - ca.inverse()*Vec3f(0, 0, 0);
	cZ.normalize();
	v = v - (v * cZ) *cZ;
	float len = v.length() ;
	v.normalize();
	Vec3f vY;
	vY = Vec3f(v[1] * cZ[2] - v[2] * cZ[1], v[2] * cZ[0] - v[0] * cZ[2], v[0] * cZ[1] - v[1] * cZ[0]);

	v *= size ;
	vY *= size;
	Vec3f ld, lu, rd, ru;
	ld = -v - vY;
	rd = v - vY;
	lu = -v + vY;
	ru = v + vY;

	if (!texn)TextureInit();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texn);

	drawTriangle(ld[0], ld[1], ld[2], rd[0], rd[1], rd[2], lu[0], lu[1], lu[2]);
	drawTriangle(rd[0], rd[1], rd[2], ru[0], ru[1], ru[2], lu[0], lu[1], lu[2]);
	drawTriangle(ld[0], ld[1], ld[2], lu[0], lu[1], lu[2], rd[0], rd[1], rd[2]);
	drawTriangle(rd[0], rd[1], rd[2], lu[0], lu[1], lu[2], ru[0], ru[1], ru[2]);

	glDisable(GL_TEXTURE_2D);
}


void Particle::render(){
	switch (type){
	case GROUND:
		setDiffuseColor(USE_COLOR_GOLD);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		drawSphere(size*(1 - age*0.5 / ageLimit));
		glPopMatrix();
		break;
	case DUST:
		setDiffuseColorAlpha(USE_COLOR_DUST, 0.7 - age / ageLimit * 0.7);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		drawSphere(size*(1 - age*0.5 / ageLimit));
		glPopMatrix();
		break;
	case FIREWORKS:
		if ( rand() % 5 == 0 )
			setDiffuseColorAlpha(USE_COLOR_RED, 0.7 - age / ageLimit * 0.7);
		else if (rand() % 5 == 1 )
			setDiffuseColorAlpha(USE_COLOR_SILVER, 0.7 - age / ageLimit * 0.7);
		else if (rand() % 5 == 2)
			setDiffuseColorAlpha(USE_COLOR_GREEN, 0.7 - age / ageLimit * 0.7);
		else if (rand() % 5 == 3)
			setDiffuseColorAlpha(USE_COLOR_GOLD, 0.7 - age / ageLimit * 0.7);
		else if (rand() % 5 == 4)
			setDiffuseColorAlpha(USE_COLOR_BLUE, 0.7 - age / ageLimit * 0.7);
		
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		drawSphere(size);
		glPopMatrix();
		break;
	case INVISIBLE_AIR:
		setDiffuseColorAlpha(USE_COLOR_WHITE,0.7 - age/ageLimit * 0.7);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		billBoard(size,velocity, texWind);
		glPopMatrix();
	case EXCALIBUR_PREPARE:
		setDiffuseColorAlpha(USE_COLOR_WHITE, 1 - age / ageLimit);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		billBoard(size, velocity, texOrange);
		glPopMatrix();
	case BURST:
		setDiffuseColorAlpha(USE_COLOR_WHITE, 1 - age / ageLimit);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		billBoard(size, velocity, texOrange);
		glPopMatrix();
	case EXCALIBUR_CAST:
		setDiffuseColorAlpha(USE_COLOR_WHITE, 0.8);
		glPushMatrix();
		glTranslated(position[0], position[1], position[2]);
		billBoard(size, velocity, texYellow);
		glPopMatrix();
	}
}


void ModelNode::addGroundParticle(){
	if (VAL(PARTICLE_GROUND)>1-1e-6){
		ParticleSystem *ps = ModelerApplication::Instance()->GetParticleSystem();
		double xp, zp;
		xp = (rand() % 100 - 50) / 50.0 * GroundSize;
		zp = (rand() % 100 - 50) / 50.0 * GroundSize;
		if (ps->isSimulate())ps->addParticle(Vec3f(xp, -8, zp), Vec3f(0, 0, 0), 0.1, 15, 0.08 ,GROUND);
	}
}

void ModelNode::modifyAxis(AxisForce* f, Vec3f AxisStart, Vec3f AxisEnd){
	Mat4f curM = getModelViewMatrix();
	Mat4f modelM = cameraMatrix.inverse() * curM;
	AxisStart = modelM * AxisStart;
	AxisEnd = modelM * AxisEnd;
	f->changeAxis(AxisStart, AxisEnd);
}

void ModelNode::addFireWorks(Vec3f origin, int deg){

		double xp, zp;
		double ang =  (deg) / 180.0 * 3.1415926;
		
		double desireRadius;
		
		for (int i = 0; i < 1; i++){
			desireRadius = i / 10.0 * (INVISIBLE_END_RADIUS - INVISIBLE_START_RADIUS) + INVISIBLE_START_RADIUS;
			xp = cos(ang) * desireRadius;
			zp = sin(ang) * desireRadius;
			if (VAL(PARTICLE_AIR) > 1 - 1e-6)spawnParticle(origin, Vec3f(8 * zp, -2, -xp * 8), 1.0, 5, 0.05, INVISIBLE_AIR);
		}

		//Update axis
	modifyAxis(invisibleAirStorm,Vec3f(0,0,0),Vec3f(0,-5,0));
}

void ModelNode::spawnParticle(Vec3f POSITION, Vec3f VELOCITY, float MASS, float AGE_LIMIT, float SIZE, ParticleType t){
	Mat4f curM = getModelViewMatrix();
	Mat4f modelM = cameraMatrix.inverse() * curM;
	POSITION = modelM * POSITION;
	VELOCITY = modelM * VELOCITY - modelM * Vec3f(0,0,0);


	ParticleSystem *ps = ModelerApplication::Instance()->GetParticleSystem();
	if (ps->isSimulate()){
		ps->addParticle(POSITION, VELOCITY, MASS, AGE_LIMIT, SIZE, t);
	}
}

void MyModel::InitializeParticleSystem(){
	ParticleSystem *ps = new ParticleSystem;

	Force *f = new Gravity(GROUND, -9.8, -1);
	ps->addForce(f);
	f = new Wind(GROUND, 0, Vec3f(1, 0, 0), 1.0, Vec3f(1, 0, 0));
	ps->addForce(f);
	f = new Wind(GROUND, 0, Vec3f(1, 0, 0), 1.0, Vec3f(0, 0, 1));
	ps->addForce(f);
	f = new Drag(GROUND, 0.05, 0.005);
	ps->addForce(f);

	f = new Gravity(FIREWORKS, 9.8, -10);
	ps->addForce(f);

	f = new Wind(DUST, 0, Vec3f(1, 0, 0), 1.0, Vec3f(0, 0, 1));
	ps->addForce(f);

	f = new Storm(INVISIBLE_AIR, Vec3f(0, 0, 0), Vec3f(1, 0, 0), 0.4, INVISIBLE_START_RADIUS, INVISIBLE_END_RADIUS, 0.0);
	ps->addForce(f);
	ModelNode::invisibleAirStorm = (AxisForce*)f;
	f = new Drag(INVISIBLE_AIR, 0.1, 0.02);
	ps->addForce(f);

	f = new Gravity(EXCALIBUR_PREPARE, -10, -10);
	ps->addForce(f);
	f = new Drag(EXCALIBUR_PREPARE,100, 0.1);
	ps->addForce(f);

	f = new Gravity(BURST, 10, 10);
	ps->addForce(f);
	f = new Drag(BURST, 100, 0.1);
	ps->addForce(f);

	// f = new Storm(EXCALIBUR_CAST,); // original source code (not compiling)
	f = new Storm(EXCALIBUR_CAST, Vec3f(0, 0, 0), Vec3f(0, 1, 0), 0, 1.0, 1.0, 0.0);
	ps->addForce(f);
	ModelNode::excaliburCast = (AxisForce*)f;
	f = new Drag(EXCALIBUR_CAST, 1, 0.01);
	ps->addForce(f);

	ModelerApplication::Instance()->SetParticleSystem(ps);
}
