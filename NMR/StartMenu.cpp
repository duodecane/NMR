#include "StartMenu.h"

void RenderInformation(float a) {
	Vec2 S = App->MP + Vec2(6, 6);;

	Render::Blur(S.x, S.y, 376, 94, Col(), false,a);
	Render::Blur(S.x, S.y, 376, 94, Col(), false,a);
	Render::FilledRoundedRect(S.x, S.y, 376, 94, Col(0, 0, 0, a * 100),8.f);
	Render::DrawString(S.x + 7, S.y + 6, Col(255, 255, 255, 255 * a), F::ICS, 0, "E");
	R::DrawString(S.x + 376 * 0.5, S.y+5, Col(255, 255, 255, 255 * a), Fonts::TX, Render::centered_x, "NMR Studio");
	R::DrawString(S.x + 8, S.y + 25, Col(255, 255, 255, 255 * a), Fonts::TXB, 0, "Version:");
	R::DrawString(S.x + 368 - R::TextSize(Fonts::TX, "1.0a").x, S.y + 25, Col(170, 170, 255, 255 * a), Fonts::TX, 0, "1.0a");
	R::DrawString(S.x + 8, S.y + 45, Col(255, 255, 255, 255 * a), Fonts::TXB, 0, "Build Date:");
	R::DrawString(S.x + 368 - R::TextSize(Fonts::TX, __DATE__).x, S.y + 45, Col(170, 170, 255, 255 * a), Fonts::TX, 0, __DATE__);
	R::DrawString(S.x + 8, S.y + 65, Col(255, 255, 255, 255 * a), Fonts::TXB, 0, "Built by:");
	R::DrawString(S.x + 368 - R::TextSize(Fonts::TX, "Alkazar").x, S.y + 65, Col(170, 170, 255, 255 * a), Fonts::TX, 0, "Alkazar");
}

Hoverable Info(Vec2(10, 10), Vec2(220, 65), RenderInformation);

void RenderNewFile(float a) {
	Vec2 S = App->MP + Vec2(6,6);
	Render::Blur(S.x, S.y, 316, 74, Col(), false, a);
	Render::Blur(S.x, S.y, 316, 74, Col(), false, a);
	Render::FilledRoundedRect(S.x, S.y, 316, 74, Col(0, 0, 0, a * 100), 8.f);
	Render::DrawString(S.x + 88, S.y + 8, Col(255, 255, 255, 255 * a), F::ICS, 0, "E");
	R::DrawString(S.x + 316 * 0.5 + 13, S.y + 7, Col(180, 180, 255, 255 * a), Fonts::TX, Render::centered_x, "New Project");
	R::DrawString(S.x + 316 * 0.5, S.y + 33, Col(255, 255, 255, 255 * a), Fonts::TX, Render::centered_x, "Creates a new NMR Project File");

	if (App->Mouse1Click)
		SC->CurrentPage = 1;
}
Hoverable New(Vec2(70, 240), Vec2(220, 150), RenderNewFile);
struct ProjectList {
	std::string Directory;
	std::string Project;
	std::string DisplayedName;
};
Vec2 PL(70, 240);
void RenderProjectList() {

	New.p[0] = PL;
	
	Render::FilledRoundedRect(PL.x, PL.y, 220, 150, Col(0, 0, 3, 70 + 40 * New.a), 8.f);
	Render::DrawString(PL.x + 110, PL.y + 75, Col(255, 255, 255, 255), F::ICL, Render::centered_xy, "B");

	New.Update();
}

void SMRender() {
	//start with loading screen yes
//	DBVisualizeHov(&Info);
	
	Render::DrawString(30, 30, Col(255, 255, 255, 255), Fonts::H, 0, "NMR Studio");
	Render::DrawString(70, 180, Col(255, 255, 255, 255), Fonts::SH, 0, "NMR Project Selection");
	RenderProjectList();

	Info.Update();
}

void SMStart() {
	PL = Vec2(70, 240);
	Info.a = 0.f;
	New.a = 0.f;
}