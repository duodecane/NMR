#pragma once
#include "Col.h"
#include "Math.h"
#include "Matrix.h"
#include "Vectors.h"


struct Vertex {
	float x;
	float y;
	unsigned int Color;
};


namespace Render {
	enum DrawStringFlags {
		none = 0,
		outline = 1 << 0,
		centered_x = 1 << 1,
		centered_y = 1 << 2,
		centered_xy = centered_x | centered_y,
		dropshadow = 1 << 3,
		outlineshadow = dropshadow | outline
	};
	void DoRender();
	void GradientCircle(float x, float y, float radius, Col inner, Col outer, bool antialiased = false);
	void FilledTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Col clr);
	void Line(float x, float y, float endx, float endy, Col clr, float thick);
	void DrawTexture(float x, float y, float l, float w, void* text, Col color = Col(255, 255, 255, 255));
	void DrawRoundedTexture(float x, float y, float l, float w, void* text, float rounding, Col color = Col(255, 255, 255, 255));
	void FilledRect(float x, float y, float l, float w, Col color);
	void FilledRoundedRect(float x, float y, float l, float w, Col color, float rounding);
	void FilledCircle(float x, float y, float r, Col color, int count);
	void FilledRoundedRectCustom(float x, float y, float l, float w, Col color, float rounding, int flags);
	void GradientFilledRect(float x, float y, float l, float w, Col left, Col right, Col bl, Col br);
	void Rect(float x, float y, float l, float w, Col color, float thickness);
	void RoundedRect(float x, float y, float l, float w, Col color, float thickness, float rounding);
	void DrawStringFmt(float x, float y, Col color, ImFont* font, unsigned int flags, const char* message, ...);
	void DrawString(float x, float y, Col color, ImFont* font, unsigned int flags, const char* message);
	void DrawVertexes(Vertex* Vertices, int Count, bool antialiased = false);
	void DrawVertexesAsLine(Vertex* Vertices, int Count, float thickness = 1.f);
	void PushClipRect(float x, float y, float w, float h, bool IntersectWithCurrentClipRect);
	void PopClipRect();
	Vec2 TextSizeFmt(ImFont* font, const char* message, ...);
	Vec2 TextSize(ImFont* font, const char* message);
	void Blur(float x, float y, float w, float h, Col col, bool drawrect = false, float alpha = 1.f);
	//D3DXCreateTextureFromFileInMemory(csgo->render_device, (void*)infinite_new_dark_png, infinite_new_dark_png_len, &csgo->Logos[0]);
	//LPDIRECT3DTEXTURE9
	LPDIRECT3DTEXTURE9 CreateTexture(void* Data, int Size);
	void DrawFullscreenBlur();
	void Initialize();
	static bool Initialized = false;
	static constexpr auto SinCosPoints = 64;
	static std::vector<Vec2> SinCosTable;
	static ImDrawList* DrawList;
	static Mat4x4 ProjectionMatrix;
};

namespace Fonts {

	extern ImFont* H;
	extern ImFont* SH;
	extern ImFont* TX;
	extern ImFont* TXB;

	extern ImFont* ICL ;
	extern ImFont* ICM ;
	extern ImFont* ICS ;
	extern ImFont* MenuThin80;
	extern ImFont* MenuThin100;
	extern ImFont* MenuMain100;
	extern ImFont* MenuMain140;
};