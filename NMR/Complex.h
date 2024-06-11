#pragma once

#include <vector>
#include <deque>
#include <cmath>


const float PI2 = 6.28318530718f;
template<typename A>
A __forceinline NormDeg(A a) {
	while (a >= PI2)
		a -= PI2;

	while (a <= 0)
		a += PI2;

	return a;
}
template<typename A>
A __forceinline FPow(A a, int B) {
	while (B > 0)
	{
		B--;
		a *= a;
	}
	return a;
}


template<typename A>
A FSin(A a) {
#ifndef OPTIMIZE
	return (A)(sin(a));
#endif
	a = NormDeg(a);

	a = a
		- (FPow(a, 3) * 0.16666666666)
		+ (FPow(a, 5) * 0.00833333333)
		- (FPow(a, 7) * 0.00019841269)
		+ (FPow(a, 9) * 0.00000275573);


	return a;
}

template<typename A>
A FCos(A a) {
#ifndef OPTIMIZE
	return (A)(cos(a));
#endif
	return FSin(0.5 * PI - a);
}
struct I {
public:
	I(float a, float b) {
		this->a = a;
		this->b = b;
	}

	I(float a) {
		this->a = cos(a); //e^itheta
		this->b = sin(a);
	}
	I() {
		a = 0;
		b = 0;
	}

	void __forceinline Exp(float a) { //e^itheta
		this->a = cos(a);
		this->b = sin(a);
	}
	void __forceinline RExp(float a) { //e^-itheta
		this->a = cos(a);
		this->b = -sin(a);
	}
	void __forceinline clear() {
		a = 0;
		b = 0;
	}
	const I& operator+(const I& o) {
		this->a += o.a;
		this->b += o.b;
		return *this;
	}

	const I& operator*(float o) {
		this->a *= o;
		this->b *= o;
		return *this;
	}
	const I& operator*(const I& o) {  //(a+bi)(c+di)
		this->a = this->a * o.a - this->b * o.b;
		this->b = this->a * o.b + o.a * this->b;
		return *this;
	}
	const I& operator-(const I& o) {
		this->a -= o.a;
		this->b -= o.b;
		return *this;
	}

	void __forceinline operator*=(const float& o) {
		*this = this->operator*(o);
	}
	void __forceinline operator*=(const I& o) {
		*this = this->operator*(o);
	}
	void __forceinline operator+=(const I& o) {
		*this = this->operator+(o);
	}
	void __forceinline operator=(const I& o) {
		this->a = o.a;
		this->b = o.b;
	}
	void __forceinline operator-=(const I& o) {
		*this = this->operator-(o);
	}
	float a;
	float b;
};

class Equation {
public:
	
};