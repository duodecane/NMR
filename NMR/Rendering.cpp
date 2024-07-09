#include "App.h"

#include "Rendering.h"
#include "ByteData.h"
#include "Blur.h"
#include <imgui_freetype.h>
#include <d3dx9tex.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#define NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = Math::InvSqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > FIXNORMAL2F_MAX_INVLEN2) inv_len2 = FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0

bool CRendering::CreateDeviceD3D(HWND hWnd) {
	if ((DirectX = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&DirectXParams, sizeof(DirectXParams));
	DirectXParams.Windowed = TRUE;
	DirectXParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	DirectXParams.BackBufferFormat = D3DFMT_UNKNOWN;
	DirectXParams.EnableAutoDepthStencil = TRUE;
	DirectXParams.AutoDepthStencilFormat = D3DFMT_D16;
	DirectXParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (DirectX->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DirectXParams, &RenderDevice) < 0)
		return false;
	return true;
}

void CRendering::Release()
{
	if (RenderDevice) { RenderDevice->Release(); RenderDevice = NULL; }
	if (DirectX) { DirectX->Release(); DirectX = NULL; }
}

void CRendering::Reset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = RenderDevice->Reset(&DirectXParams);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

void Render::PushClipRect(float x, float y, float w, float h, bool IntersectWithCurrentClipRect) {
	DrawList->PushClipRect(ImVec2(x, y), ImVec2(x + w, y + h), IntersectWithCurrentClipRect);
}
void Render::PopClipRect() {
	DrawList->PopClipRect();
}

static IDirect3DSurface9* rtBackup = nullptr;
static IDirect3DPixelShader9* blurShaderX = nullptr;
static IDirect3DPixelShader9* blurShaderY = nullptr;
static IDirect3DTexture9* blurTexture = nullptr;
static int backbufferWidth = 0;
static int backbufferHeight = 0;

static void BeginBlur(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	const auto device = reinterpret_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

	if (!blurShaderX)
	{
		device->CreatePixelShader(reinterpret_cast<const DWORD*>(blur_x.data()), &blurShaderX);
	}

	if (!blurShaderY)
	{
		device->CreatePixelShader(reinterpret_cast<const DWORD*>(blur_y.data()), &blurShaderY);
	}

	IDirect3DSurface9* backBuffer;
	device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
	D3DSURFACE_DESC desc;
	backBuffer->GetDesc(&desc);

	if (backbufferWidth != desc.Width || backbufferHeight != desc.Height)
	{
		if (blurTexture)
			blurTexture->Release();

		backbufferWidth = desc.Width;
		backbufferHeight = desc.Height;
		device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &blurTexture, nullptr);
	}

	device->GetRenderTarget(0, &rtBackup);

	{
		IDirect3DSurface9* surface;
		blurTexture->GetSurfaceLevel(0, &surface);
		device->StretchRect(backBuffer, NULL, surface, NULL, D3DTEXF_NONE);
		device->SetRenderTarget(0, surface);
		surface->Release();
	}

	backBuffer->Release();

	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

static void FirstBlurPass(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	const auto device = reinterpret_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

	device->SetPixelShader(blurShaderX);
	const float params[4] = { 1.0f / backbufferWidth };
	device->SetPixelShaderConstantF(0, params, 1);
}

static void SecondBlurPass(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	const auto device = reinterpret_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

	device->SetPixelShader(blurShaderY);
	const float params[4] = { 1.0f / backbufferHeight };
	device->SetPixelShaderConstantF(0, params, 1);
}

static void EndBlur(const ImDrawList* parent_list, const ImDrawCmd* cmd)
{
	const auto device = reinterpret_cast<IDirect3DDevice9*>(cmd->UserCallbackData);

	device->SetRenderTarget(0, rtBackup);
	rtBackup->Release();

	device->SetPixelShader(nullptr);
	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

inline void DrawBackgroundBlur(ImDrawList* drawList, IDirect3DDevice9* device)
{
	drawList->AddCallback(BeginBlur, device);

	for (int i = 0; i < 8; ++i) {
		drawList->AddCallback(FirstBlurPass, device);
		drawList->AddImage(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });
		drawList->AddCallback(SecondBlurPass, device);
		drawList->AddImage(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });
	}

	drawList->AddCallback(EndBlur, device);
	drawList->AddImageRounded(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(255, 255, 255, 255), 7.f);
}
void Render::Blur(float x, float y, float w, float h, Col col, bool drawrect, float alpha) {

	PushClipRect(x, y, w, h, true);

	DrawList->AddCallback(BeginBlur, App->Render->RenderDevice);

	for (int i = 0; i < 8; ++i) {
		DrawList->AddCallback(FirstBlurPass, App->Render->RenderDevice);
		DrawList->AddImage(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });
		DrawList->AddCallback(SecondBlurPass, App->Render->RenderDevice);
		DrawList->AddImage(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });
	}

	DrawList->AddCallback(EndBlur, App->Render->RenderDevice);
	DrawList->AddImageRounded(blurTexture, { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(255, 255, 255, 255 * alpha), 7.f);

	if (drawrect)
		FilledRect(x, y, w, h, col);

	PopClipRect();
}
LPDIRECT3DTEXTURE9 Render::CreateTexture(void* Data, int Size) {
	LPDIRECT3DTEXTURE9 Out;
	D3DXCreateTextureFromFileInMemory(App->Render->RenderDevice, Data, Size, &Out);
	return Out;
}
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0

void Render::DrawVertexesAsLine(Vertex* Vertices, int Count, float thickness){/*
	const int idx_count = (Count - 1) * 6;
	const int vtx_count =( Count - 1) * 4;    // FIXME-OPT: Not sharing edges
	DrawList->PrimReserve(idx_count, vtx_count);
	const ImVec2 opaque_uv = DrawList->_Data->TexUvWhitePixel;
	for (int i1 = 0; i1 < Count - 1; i1++)
	{
		const int i2 = (i1 + 1) == Count ? 0 : i1 + 1;
		const ImVec2 p1 = ImVec2(Vertices[i1].x, Vertices[i1].y);
		const ImVec2 p2 = ImVec2( Vertices[i2].x, Vertices[i2].y);
		ImU32 Col = 
		float dx = p2.x - p1.x;
		float dy = p2.y - p1.y;
		IM_NORMALIZE2F_OVER_ZERO(dx, dy);
		dx *= (thickness * 0.5f);
		dy *= (thickness * 0.5f);

		DrawList->_VtxWritePtr[0].pos.x = p1.x + dy;DrawList->_VtxWritePtr[0].pos.y = p1.y - dx;DrawList->_VtxWritePtr[0].uv = opaque_uv;DrawList->_VtxWritePtr[0].col = col;
		DrawList->_VtxWritePtr[1].pos.x = p2.x + dy;DrawList->_VtxWritePtr[1].pos.y = p2.y - dx;DrawList->_VtxWritePtr[1].uv = opaque_uv;DrawList->_VtxWritePtr[1].col = col;
		DrawList->_VtxWritePtr[2].pos.x = p2.x - dy;DrawList->_VtxWritePtr[2].pos.y = p2.y + dx;DrawList->_VtxWritePtr[2].uv = opaque_uv;DrawList->_VtxWritePtr[2].col = col;
		DrawList->_VtxWritePtr[3].pos.x = p1.x - dy;DrawList->_VtxWritePtr[3].pos.y = p1.y + dx;DrawList->_VtxWritePtr[3].uv = opaque_uv;DrawList->_VtxWritePtr[3].col = col;
		DrawList->_VtxWritePtr += 4;

		DrawList->_IdxWritePtr[0] = (ImDrawIdx)(DrawList->_VtxCurrentIdx);DrawList->_IdxWritePtr[1] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + 1);DrawList->_IdxWritePtr[2] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + 2);
		DrawList->_IdxWritePtr[3] = (ImDrawIdx)(DrawList->_VtxCurrentIdx);DrawList->_IdxWritePtr[4] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + 2);DrawList->_IdxWritePtr[5] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + 3);
		DrawList->_IdxWritePtr += 6;
		DrawList->_VtxCurrentIdx += 4;
	}*/
}
void Render::DrawVertexes(Vertex* Vertices, int Count, bool antialiased) {
	if (Count < 3)
		return;

	const ImVec2 uv = DrawList->_Data->TexUvWhitePixel;

	if (antialiased)
	{
		// Anti-aliased Fill
		const float AA_SIZE = DrawList->_FringeScale;

		const int idx_count = (Count - 2) * 3 + Count * 6;
		const int vtx_count = (Count * 2);
		DrawList->PrimReserve(idx_count, vtx_count);

		// Add indexes for fill
		unsigned int vtx_inner_idx = DrawList->_VtxCurrentIdx;
		unsigned int vtx_outer_idx = DrawList->_VtxCurrentIdx + 1;
		for (int i = 2; i < Count; i++)
		{
			DrawList->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); DrawList->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); DrawList->_IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
			DrawList->_IdxWritePtr += 3;
		}

		// Compute normals
		DrawList->_Data->TempBuffer.reserve_discard(Count);
		ImVec2* temp_normals = DrawList->_Data->TempBuffer.Data;
		for (int i0 = Count - 1, i1 = 0; i1 < Count; i0 = i1++)
		{
			const ImVec2& p0 = ImVec2(Vertices[i0].x, Vertices[i0].y);
			const ImVec2& p1 = ImVec2(Vertices[i1].x, Vertices[i1].y);
			float dx = p1.x - p0.x;
			float dy = p1.y - p0.y;
			NORMALIZE2F_OVER_ZERO(dx, dy);
			temp_normals[i0].x = dy;
			temp_normals[i0].y = -dx;
		}

		for (int i0 = Count - 1, i1 = 0; i1 < Count; i0 = i1++)
		{
			const ImU32 col_trans = Vertices[i1].Color & ~IM_COL32_A_MASK;
			// Average normals
			const ImVec2& n0 = temp_normals[i0];
			const ImVec2& n1 = temp_normals[i1];
			float dm_x = (n0.x + n1.x) * 0.5f;
			float dm_y = (n0.y + n1.y) * 0.5f;
			FIXNORMAL2F(dm_x, dm_y);
			dm_x *= AA_SIZE * 0.5f;
			dm_y *= AA_SIZE * 0.5f;

			// Add vertices
			DrawList->_VtxWritePtr[0].pos.x = (Vertices[i1].x - dm_x); DrawList->_VtxWritePtr[0].pos.y = (Vertices[i1].y - dm_y); DrawList->_VtxWritePtr[0].uv = uv; DrawList->_VtxWritePtr[0].col = Vertices[i1].Color;        // Inner
			DrawList->_VtxWritePtr[1].pos.x = (Vertices[i1].x + dm_x); DrawList->_VtxWritePtr[1].pos.y = (Vertices[i1].y + dm_y); DrawList->_VtxWritePtr[1].uv = uv; DrawList->_VtxWritePtr[1].col = col_trans;  // Outer
			DrawList->_VtxWritePtr += 2;

			// Add indexes for fringes
			DrawList->_IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); DrawList->_IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); DrawList->_IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
			DrawList->_IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); DrawList->_IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); DrawList->_IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
			DrawList->_IdxWritePtr += 6;
		}
		DrawList->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
	else
	{

		const int idx_count = (Count - 2) * 3;
		const int vtx_count = Count;
		DrawList->PrimReserve(idx_count, vtx_count);
		for (int i = 0; i < vtx_count; i++)
		{
			DrawList->_VtxWritePtr[0].pos = ImVec2(Vertices[i].x, Vertices[i].y);  DrawList->_VtxWritePtr[0].uv = uv;  DrawList->_VtxWritePtr[0].col = Vertices[i].Color;
			DrawList->_VtxWritePtr++;
		}
		for (int i = 2; i < Count; i++)
		{
			DrawList->_IdxWritePtr[0] = (ImDrawIdx)(DrawList->_VtxCurrentIdx);  DrawList->_IdxWritePtr[1] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + i - 1);  DrawList->_IdxWritePtr[2] = (ImDrawIdx)(DrawList->_VtxCurrentIdx + i);
			DrawList->_IdxWritePtr += 3;
		}
		DrawList->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
	}
}
void Render::DrawString(float x, float y, Col color, ImFont* font, unsigned int flags, const char* message) {



	DrawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);

	auto size = ImGui::CalcTextSize(message);


 


	if (!(flags & Render::centered_x))
		size.x = 0;
	if (!(flags & Render::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(x - (size.x * .5), y - (size.y * .5));

	if (flags & Render::outline)
	{
		Col outline_clr = Col(0, 0, 0, color[3]);
		if (flags & Render::dropshadow)
			outline_clr[3] *= 0.38f;
		pos.y++;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x++;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.y--;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x--;
		DrawList->AddText(pos, outline_clr.u32(), message);
	}
	else if (flags & Render::dropshadow) {
		Col outline_clr = Col(0, 0, 0, color[3] * 0.38f);
		pos.y += 0.5f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x += 0.5f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.y -= 1.f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x -= 1.f;
		DrawList->AddText(pos, outline_clr.u32(), message);
	}

	DrawList->AddText(pos, color.u32(), message);
	ImGui::PopFont();
}
Vec2 Render::TextSizeFmt(ImFont* font, const char* message, ...) {
	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);
	ImGui::PushFont(font);
	auto size = ImGui::CalcTextSize(output);
	ImGui::PopFont();
	return Vec2(size.x, size.y);
}
Vec2 Render::TextSize(ImFont* font, const char* message) {

	ImGui::PushFont(font);
	auto size = ImGui::CalcTextSize(message);
	ImGui::PopFont();
	return Vec2(size.x, size.y);
}
void Render::DrawFullscreenBlur() {

}
void Render::DrawStringFmt(float x, float y, Col color, ImFont* font, unsigned int flags, const char* message, ...) {

	char output[1024] = {};
	va_list args;
	va_start(args, message);
	vsprintf_s(output, message, args);
	va_end(args);


	DrawList->PushTextureID(font->ContainerAtlas->TexID);
	ImGui::PushFont(font);

	auto size = ImGui::CalcTextSize(output);





	if (!(flags & Render::centered_x))
		size.x = 0;
	if (!(flags & Render::centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(x - (size.x * .5), y - (size.y * .5));

	if (flags & Render::outline)
	{
		Col outline_clr = Col(0, 0, 0, color[3]);
		if (flags & Render::dropshadow)
			outline_clr[3] *= 0.38f;
		pos.y++;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x++;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.y--;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x--;
		DrawList->AddText(pos, outline_clr.u32(), message);
	}
	else if (flags & Render::dropshadow) {
		Col outline_clr = Col(0, 0, 0, color[3] * 0.38f);
		pos.y += 0.5f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x += 0.5f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.y -= 1.f;
		DrawList->AddText(pos, outline_clr.u32(), message);
		pos.x -= 1.f;
		DrawList->AddText(pos, outline_clr.u32(), message);
	}

	DrawList->AddText(pos, color.u32(), output);
	ImGui::PopFont();
}
void Render::FilledRoundedRect(float x, float y, float l, float w, Col color, float rounding) {
	DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + l, y + w), color.u32(), rounding, ImDrawFlags_RoundCornersAll);
}
void Render::FilledCircle(float x, float y, float r, Col color, int count) {
	DrawList->AddCircleFilled(ImVec2(x, y), r, color.u32(), count);
}
void Render::FilledRoundedRectCustom(float x, float y, float l, float w, Col color, float rounding, int flags) {
	DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + l, y + w), color.u32(), rounding, flags);
}
void Render::Rect(float x, float y, float l, float w, Col color, float thickness) {
	DrawList->AddRect(ImVec2(x, y), ImVec2(x + l, y + w), color.u32(), 0.f, 0, thickness);
}
void Render::RoundedRect(float x, float y, float l, float w, Col color, float thickness, float rounding) {
	DrawList->AddRect(ImVec2(x, y), ImVec2(x + l, y + w), color.u32(), rounding, ImDrawFlags_RoundCornersAll, thickness);
}
void  Render::DrawTexture(float x, float y, float l, float w, void* text, Col color) {
	DrawList->AddImage(text, ImVec2(x, y), ImVec2(x + l, y + w), ImVec2(0, 0), ImVec2(1, 1), color.u32());
}
void  Render::DrawRoundedTexture(float x, float y, float l, float w, void* text, float rounding, Col color) {
	DrawList->AddImageRounded(text, ImVec2(x, y), ImVec2(x + l, y + w), ImVec2(0, 0), ImVec2(1, 1), color.u32(), rounding, ImDrawFlags_::ImDrawFlags_RoundCornersAll);
}

void Render::FilledRect(float x, float y, float l, float w, Col color) {
	DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + l, y + w), color.u32());
	//ImGui::GetBackgroundDrawList()->PushTextureID(Fonts::MenuMain->ContainerAtlas->TexID);
//	ImGui::PushFont(Fonts::MenuMain);


	//ImGui::GetBackgroundDrawList()->AddText(ImVec2(500,500), Col, "test text rend");
	//ImGui::PopFont();
	//THIS IS ONLY TESTING RENDERING IGNORE THIS HORRENDOUS CODE
}
void AddRectMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
	if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
		return;

	const ImVec2 uv = Render::DrawList->_Data->TexUvWhitePixel;
	Render::DrawList->PrimReserve(6, 4);
	Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx)); Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx + 1)); Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx + 2));
	Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx)); Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx + 2)); Render::DrawList->PrimWriteIdx((ImDrawIdx)(Render::DrawList->_VtxCurrentIdx + 3));
	Render::DrawList->PrimWriteVtx(p_min, uv, col_upr_left);
	Render::DrawList->PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
	Render::DrawList->PrimWriteVtx(p_max, uv, col_bot_right);
	Render::DrawList->PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}
void Render::GradientFilledRect(float x, float y, float l, float w, Col left, Col right, Col bl, Col br) {
	AddRectMultiColor(ImVec2(x, y), ImVec2(x + l, y + w),
		left.u32(), right.u32(), br.u32(), bl.u32());
}
void Render::GradientCircle(float x, float y, float radius, Col inner, Col outer, bool antialiased) {
	Vertex vert[SinCosPoints + 2] = {};
	ImU32 out = outer.u32();

	for (auto i = 1; i <= SinCosPoints; i++)
		vert[i] =
	{
		x + radius * SinCosTable[i].x,
		y - radius * SinCosTable[i].y,
		out
	};

	vert[0] = { x,y, inner.u32() };
	vert[SinCosPoints + 1] = vert[1];

	DrawVertexes(vert, SinCosPoints + 2, antialiased);
}
void Render::FilledTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Col clr)
{
	DrawList->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.u32());
}


void  Render::Line(float x, float y, float endx, float endy, Col clr, float thick) {
	Render::DrawList->AddLine(ImVec2(x, y), ImVec2(endx, endy), clr.u32(), thick);
}

void Render::DoRender() {

	DrawList = ImGui::GetBackgroundDrawList();

	App->UpdateKeyStates();

	App->RenderScenes();

	
}

bool Vec2::WorldToScreen(Vec3& Vec, Vec2& Out) {
	
	return true;
}
Vec2 Vec3::ToScreen() //check success with OnScreen()
{
	return Vec2(0, 0);
}
void Render::Initialize() {
	if (Initialized)
		return;
	// build fonts
	ImGuiIO& io = ImGui::GetIO();

	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;



	builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	builder.BuildRanges(&ranges);

	ImFontConfig cfg{};
	ImFontConfig defcfg{};
	cfg.OversampleH = cfg.OversampleV = 2;
	cfg.PixelSnapH = false;

	io.Fonts->Clear();
	//GetGlyphRangesDefault not GetGlyphRangesCyrillic cuz notr required
	Fonts::H = io.Fonts->AddFontFromMemoryTTF(CascadiaCode, CascadiaCodeLen, 38.f, &defcfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::TXB = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 20.f, &defcfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::TX = io.Fonts->AddFontFromMemoryTTF(CascadiaCode, CascadiaCodeLen, 19.f, &defcfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::SH = io.Fonts->AddFontFromMemoryTTF(CascadiaCode, CascadiaCodeLen, 26.f, &defcfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::ICL = io.Fonts->AddFontFromMemoryTTF(FontIcons, FontIconsLength, 40.f, &cfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::ICM = io.Fonts->AddFontFromMemoryTTF(FontIcons, FontIconsLength, 30.f, &cfg, io.Fonts->GetGlyphRangesDefault());
	Fonts::ICS = io.Fonts->AddFontFromMemoryTTF(FontIcons, FontIconsLength, 20.f, &cfg, io.Fonts->GetGlyphRangesDefault());
	cfg.FontBuilderFlags = ImGuiFreeType::RasterizerFlags::LightHinting;
	


	Fonts::MenuThin80 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/calibri.ttf", 18.f * 0.8f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	Fonts::MenuMain100 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 28.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	Fonts::MenuThin100 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/calibri.ttf", 18.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

	Fonts::MenuMain140 = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/NirmalaB.ttf", 28.f * 1.4f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	
	static const ImWchar GlobalRange[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x2000, 0x206F, // General Punctuation
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0x4e00, 0x9FAF, // CJK Ideograms
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0,
	};

	cfg.FontBuilderFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome; //pixelate sharp
	//Fonts::Verdana = io.Fonts->AddFontFromFileTTF("C:/windows/fonts/verdana.ttf", 13, &cfg, GlobalRange); //since their name might have Chinese/Japanese/Russion
	

	//	io.Fonts->Build(); //ALL I DID IS CHANGE THIS ONE LINE SO WE BUILD WITH FREETYPE NOW
	if (!ImGuiFreeType::BuildFontAtlas(io.Fonts, 0x00))
		throw IException("Error: Fonts are unavailable", 0);

	//build table used for shapes with a curve
	if (SinCosTable.empty()) {
		for (float i = 0.f; i <= SinCosPoints; i++)
			SinCosTable.emplace_back(
				std::cos(2.f * FPI * (i / static_cast<float>(SinCosPoints))),
				std::sin(2.f * FPI * (i / static_cast<float>(SinCosPoints)))
			);
	}


	Initialized = true;
}

namespace Fonts {
	 ImFont* H = nullptr;;
	 ImFont* SH = nullptr;;
	 ImFont* TX = nullptr;;
	 ImFont* TXB = nullptr;;
	ImFont* ICL = nullptr;
	ImFont* ICM = nullptr;
	ImFont* ICS = nullptr;
	ImFont* MenuThin80 = nullptr;
	ImFont* MenuThin100 = nullptr;
	ImFont* MenuMain100 = nullptr;
	ImFont* MenuMain140 = nullptr;
};