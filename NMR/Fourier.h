
#pragma once
#include "Complex.h"
#include "Interpolate.h"

struct HarmonicEquation {
	float Sin;
	float Cos;
	float AmplitudeSin;
	float AmplitudeCos;
	float Shift;
};

struct FourierPair {
	float Frequency;
	I Value;
};

bool FourierTransformManual(const std::vector<float>& in, std::vector<I>& out);
bool FourierTransform(std::vector<float>& in, std::vector<I>& out, int interpolatesample, float timesample) {
	//we need to produce new data
	std::vector<float> copy = {};
	float LastSample = 0.f;
	float Time = 0.f;
	bool lastsample = false;
	for (auto& sample : in) {
		if (!lastsample) {
			LastSample = sample;
			lastsample = true;
			continue;
		}

		float Slope = SolveSlope(Time, LastSample, Time + timesample, sample);
		float Shift = SolveShift(Slope, Time, LastSample);
		copy.push_back(LastSample);
		Time += timesample / interpolatesample;
		for (int i = 1; i < interpolatesample-1; i++) {
		
			copy.push_back(InterpolateManual(Time, Slope, Shift));
			Time += timesample / interpolatesample;
		}
		copy.push_back(sample);
		Time += timesample / interpolatesample;
		LastSample = sample;
	}

	in = copy;



	return FourierTransformManual(copy, out);
}

bool FourierNormalize(std::vector<I>& out) {
	for (auto& a : out) {
		if (fabsf(0.f - a.a) <= 0.001f)
			a.a = 0.f;

		if (fabsf(0.f - a.b) <= 0.001f)
			a.b = 0.f;

		a.a /= out.size();// *0.5f;
		a.b /= out.size();// *0.5f;
	}

	return true;
}

bool FourierTransformManual(const std::vector<float>& in, std::vector<I>& out) {

	out.clear();

	float size = (float)in.size();
	for (int freq = 0; freq < in.size(); freq++) {
		I sol(0.f, 0.f);
		I temp(0.f, 0.f);
		for (int i = 0; i < in.size(); i++) {
			float theta = i * freq;
		//	std::cout << "x1: " << std::to_string(theta) << "\n";
			theta /= size;
		//	std::cout << "x2: " << std::to_string(theta) << "\n";
			theta *= PI2;
		//	std::cout << "x3: " << std::to_string(theta) << "\n";
			temp.RExp(theta);
		//	std::cout << "x4: " << std::to_string(temp.b) << "\n";

			temp *= in[i];

			sol += temp;

		}
		out.push_back(sol);
	}

	return true;
}