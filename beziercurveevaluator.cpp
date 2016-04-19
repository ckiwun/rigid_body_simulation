#include "beziercurveevaluator.h"
#include <cassert>
#include <cmath>

void BezierCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts, 
										 std::vector<Point>& ptvEvaluatedCurvePts, 
										 const float& fAniLength, 
										 const bool& bWrap) const
{
	int iCtrlPtCount = ptvCtrlPts.size();

	ptvEvaluatedCurvePts.clear();

	float x = 0.0;
	float y1;

	if (bWrap) {
		// if wrapping is on, interpolate the y value at xmin and
		// xmax so that the slopes of the lines adjacent to the
		// wraparound are equal.

		if ((ptvCtrlPts[0].x + fAniLength) - ptvCtrlPts[iCtrlPtCount - 1].x > 0.0f) {
			y1 = (ptvCtrlPts[0].y * (fAniLength - ptvCtrlPts[iCtrlPtCount - 1].x) + 
				  ptvCtrlPts[iCtrlPtCount - 1].y * ptvCtrlPts[0].x) /
				 (ptvCtrlPts[0].x + fAniLength - ptvCtrlPts[iCtrlPtCount - 1].x);
		}
		else 
			y1 = ptvCtrlPts[0].y;
	}
	else {
		// if wrapping is off, make the first and last segments of
		// the curve horizontal.

		y1 = ptvCtrlPts[0].y;
    }

	ptvEvaluatedCurvePts.push_back(Point(x, y1));

	/// set the endpoint based on the wrap flag.
	float y2;
    x = fAniLength;
    if (bWrap)
		y2 = y1;
    else
		y2 = ptvCtrlPts[iCtrlPtCount - 1].y;

	ptvEvaluatedCurvePts.push_back(Point(x, y2));

	auto b = ptvCtrlPts.begin();
	while(iCtrlPtCount>=4){
		//calculate the bezier equation
		auto p0 = *(b++);
		auto p1 = *(b++);
		auto p2 = *(b++);
		auto p3 = *b;
		//uniform sampling
		int i = 0;
		const int n = 100;
		while(i!=n){
			double u = (double)i/(const double)n;
			Point result =	p0*(-pow(u,3)+3*pow(u,2)-3*u+1)+
					p1*(3*pow(u,3)-6*pow(u,2)+3*u)+
					p2*(-3*pow(u,3)+3*pow(u,2))+
					p3*(pow(u,3));
			ptvEvaluatedCurvePts.push_back(result);
			i++;
		}
		iCtrlPtCount -= 3;
	}
	while(b!=ptvCtrlPts.end()){
		ptvEvaluatedCurvePts.push_back(*b);
		b++;
	}
}
