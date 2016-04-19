#include "bsplinecurveevaluator.h"
#include <cassert>

void BsplineCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts, 
										 std::vector<Point>& ptvEvaluatedCurvePts, 
										 const float& fAniLength, 
										 const bool& bWrap) const
{
	std::vector<Point> my_ptvCtrlPts;
	auto b = ptvCtrlPts.begin();
	my_ptvCtrlPts.push_back(*b);
	my_ptvCtrlPts.push_back(*b);
	my_ptvCtrlPts.push_back(*b);
	while(b!=ptvCtrlPts.end()){
		my_ptvCtrlPts.push_back(*b);
		b++;
	}
	b--;
	my_ptvCtrlPts.push_back(*b);
	my_ptvCtrlPts.push_back(*b);
	my_ptvCtrlPts.push_back(*b);

	int iCtrlPtCount = my_ptvCtrlPts.size();

	ptvEvaluatedCurvePts.clear();

	float x = 0.0;
	float y1;

	if (bWrap) {
		// if wrapping is on, interpolate the y value at xmin and
		// xmax so that the slopes of the lines adjacent to the
		// wraparound are equal.

		if ((my_ptvCtrlPts[0].x + fAniLength) - my_ptvCtrlPts[iCtrlPtCount - 1].x > 0.0f) {
			y1 = (my_ptvCtrlPts[0].y * (fAniLength - my_ptvCtrlPts[iCtrlPtCount - 1].x) + 
				  my_ptvCtrlPts[iCtrlPtCount - 1].y * my_ptvCtrlPts[0].x) /
				 (my_ptvCtrlPts[0].x + fAniLength - my_ptvCtrlPts[iCtrlPtCount - 1].x);
		}
		else 
			y1 = my_ptvCtrlPts[0].y;
	}
	else {
		// if wrapping is off, make the first and last segments of
		// the curve horizontal.

		y1 = my_ptvCtrlPts[0].y;
    }

	ptvEvaluatedCurvePts.push_back(Point(x, y1));

	/// set the endpoint based on the wrap flag.
	float y2;
    x = fAniLength;
    if (bWrap)
		y2 = y1;
    else
		y2 = my_ptvCtrlPts[iCtrlPtCount - 1].y;

	ptvEvaluatedCurvePts.push_back(Point(x, y2));
	auto b2 = my_ptvCtrlPts.begin();
	while(iCtrlPtCount>=4){
		//calculate the bezier equation
		auto p0 = *b2;
		b2++;
		auto p1 = *b2;
		b2++;
		auto p2 = *b2;
		b2++;
		auto p3 = *b2;
		b2-=2;
		//uniform sampling
		int i = 0;
		const int n = 100;
		while(i!=n){
			double u = (double)i/(const double)n;
			Point result =	p0*(-(1.0/6.0)*pow(u,3)+(1.0/2.0)*pow(u,2)-(1.0/2.0)*u+(1.0/6.0))+
					p1*((1.0/2.0)*pow(u,3)-pow(u,2)+(2.0/3.0))+
					p2*(-(1.0/2.0)*pow(u,3)+(1.0/2.0)*pow(u,2)+(1.0/2.0)*u+(1.0/6.0))+
					p3*((1.0/6.0)*pow(u,3));
			ptvEvaluatedCurvePts.push_back(result);
			i++;
		}
		iCtrlPtCount--;
	}
	b2+=2;
	while(b2!=my_ptvCtrlPts.end()){
		ptvEvaluatedCurvePts.push_back(*b);
		b2++;
	}
}
