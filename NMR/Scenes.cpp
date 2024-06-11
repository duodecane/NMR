#include "Scenes.h"
#include "SceneContext.h"


float Ease(const float& a) {
	if (SceneContext->DisableComplexAnimations) {
		return a * a * a;
	}
	if (a == 0.f || a == 1.f) //avoid wasting resources
		return a;

	float sqt = a * a;
	return sqt / (2.0f * (sqt - a) + 1.0f);

}
float DEase(const float& a, const float& Delay) { //delay ease


	if (a == 1.f)
		return a;

	if (a <= Delay)
		return 0.f;

	return ((a - Delay) / (1.f - Delay));
}

inline void Anim(float& f, bool Enable) {
	if ((Enable && f < 1.f) || (!Enable && f > 0.f))
		f = Math::Clamp(f + ((Enable ? 1.f : -1.f) * 0.007f * SceneContext->AnimationModifier), 0.f, 1.f);
}
inline void EAnim(float& f, bool Enable, float speed) { //ease

	if (SceneContext->DisableComplexAnimations) {
		Anim(f, Enable);
	}

	f = (f + ((Enable ? 1.f : 0.f) - f) * speed * SceneContext->AnimationModifier);
	if (f > 0.993f)
		f = 1.f;

	if (f < 0.013f)
		f = 0.f;
}



void OnUpdateScene(int NewScene) {
	switch (NewScene) {
	case 0:
		SMStart();
		break;
	}
}

AnimCache CloseButton;
Clickable CloseButtonEvent(Vec2(1200 - 30, 15), Vec2(15, 15), []() {App->Exit(); },&CloseButton );
AnimCache Minimize;
Clickable MinimizeEvent(Vec2(1200 - 70, 15), Vec2(15, 15), []() {ShowWindow(App->Window, SW_MINIMIZE); }, & Minimize);

void GRender() {
	CloseButtonEvent.Update();
	MinimizeEvent.Update();
	Render::Line(MinimizeEvent.p[0].x, MinimizeEvent.p[0].y + MinimizeEvent.p[1].y * 0.5f, MinimizeEvent.p[0].x + MinimizeEvent.p[1].x, MinimizeEvent.p[0].y + MinimizeEvent.p[1].y * 0.5f, Col(255, 255, 255, 100 + 155 * Minimize.Modifier), 4.f);
	Render::Line(CloseButtonEvent.p[0].x, CloseButtonEvent.p[0].y, CloseButtonEvent.p[0].x + CloseButtonEvent.p[1].x, CloseButtonEvent.p[0].y + CloseButtonEvent.p[1].y, Col(255, 0, 0, 100 + 155 * CloseButton.Modifier), 4.f);
	Render::Line(CloseButtonEvent.p[0].x, CloseButtonEvent.p[0].y + CloseButtonEvent.p[1].y, CloseButtonEvent.p[0].x + CloseButtonEvent.p[1].x, CloseButtonEvent.p[0].y, Col(255, 0, 0, 100 + 155 * CloseButton.Modifier), 4.f);

}
_SceneContext* SceneContext = new _SceneContext{};



//debug funbctions


 void DBVisualizeHov(Hoverable* A) {
	 Render::FilledRect(A->p[0].x, A->p[0].y, A->p[1].x,A->p[1].y, Col());
}
 void DBVisualizeCl(Hoverable* A) {
	 Render::FilledRect(A->p[0].x, A->p[0].y, A->p[1].x, A->p[1].y, Col());
}
