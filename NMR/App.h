#pragma once
#include "Includes.h"
#include "Rendering.h"

class CRendering {
public:
	LPDIRECT3DDEVICE9        RenderDevice;
	D3DPRESENT_PARAMETERS    DirectXParams;
	LPDIRECT3D9              DirectX;
	bool CreateDeviceD3D(HWND hWnd);

	void Release();

	void Reset();
};


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class CApp {
public:
	HWND Window;
	CRendering* Render = new CRendering();
	void Init();
	void UpdateKeyStates();
	void RenderScenes();
	void Exit();
	bool ShouldExit;
	Vec2 ScreenSize;
	Vec2 MP; //mousepos
	Vec2 PrevMP; //mousepos old
	float Scroll;
	float Interval;
	float LastLoopTime;
	float Curtime;
	float AcceptTimeAnimation;
	bool KeyStates[256];
	bool OldKeyStates[256];
	bool Mouse1Click;
	std::string DefaultSearchFolder;
	bool Mouse1Press;
	bool KeyPressed(int VKEY) {
		if (VKEY < 0 || VKEY > 256)
			return false;
		return KeyStates[VKEY];
	}
	bool KeyToggled(int VKEY) {
		if (VKEY < 0 || VKEY > 256)
			return false;
		return KeyStates[VKEY] && !OldKeyStates[VKEY];
	}
	std::vector<int> GetInputCharacters();
};

extern CApp* App;