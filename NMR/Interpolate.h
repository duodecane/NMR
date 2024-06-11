#pragma once
#include "Complex.h"
#include "Math.h"

//linear interpolate

float SolveSlope(float x1, float y1, float x2, float y2) { //m = dy/dx
	return (y2 - y1) / (x2 - x1);
}

float SolveShift(float slope, float x, float y) { //y=mx+c, c=y-mx
	return y - (slope * x);
}


float InterpolateManual(float x, float slope, float c) {
	return slope * x + c; //return mx+c
}

float Interpolate(float x1, float y1, float x2, float y2, float locx) {
	float slope = SolveSlope(x1, y1, x2, y2);
	return InterpolateManual(locx, slope, SolveShift(slope, x1, y1));
}


