#pragma once
#include "App.h"
struct AnimCache {
	float Modifier;
	//bool Enable;
};

struct _SceneContext {
	int CurrentPage = 0; //0 = start menu, 1 = new file
	bool DoLoadAnimation = false;
	AnimCache BasicAnimationCache[10]; //for basic functions
	//float Scale = 1.f;
	Vec2 ScreenSize;
	bool DisableComplexAnimations;
	float AnimationModifier;
	Col Bg = Col(31, 31, 34, 255);
	Col Tx = Col(255,255,255, 255);
};
#define SC SceneContext
#define R Render
#define F Fonts
#define MC App->Mouse1Click
extern _SceneContext* SceneContext;


float Ease(const float& a);
float DEase(const float& a, const float& Delay);

inline void Anim(float& f, bool Enable);
inline void EAnim(float& f, bool Enable, float speed);


class Hoverable {
public:
	Hoverable() {}
	Hoverable(Vec2 Position, Vec2 Bounds, void(*render)(float alpha)) {
		r = render;
		p[0] = Position;
		p[1] = Bounds;
	}
	void Update() {
		bool hovered = App->MP >= p[0] && App->MP <= (p[0] + p[1]);
		Anim(a, hovered);
		if (a > 0)
			r(a);
	}
	Vec2 p[2];
	void(*r)(float alpha);
	float a;
};

class Clickable {
public:
	Clickable() {}
	Clickable(Vec2 Position, Vec2 Bounds, void(*onclick)(), AnimCache* opsional = nullptr) {
		Event = onclick;
		cache = opsional;
		p[0] = Position;
		p[1] = Bounds;
	}
	void Update() {
		bool hovered = App->MP >= p[0] && App->MP <= (p[0] + p[1]);

		if (cache) {
			Anim(cache->Modifier, hovered);
		}

		if (hovered && App->Mouse1Click)
			Event();
	}
	Vec2 p[2];
	void(*Event)();
	AnimCache* cache;
};

 void DBVisualizeHov(Hoverable* A);
 void DBVisualizeCl(Hoverable* A);
