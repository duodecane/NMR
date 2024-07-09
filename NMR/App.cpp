#include "App.h"
#include "Scenes.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    static int ScrollDelta = 0;
    if (msg == WM_MOUSEWHEEL) {
        ScrollDelta += GET_WHEEL_DELTA_WPARAM(wParam);
        for (; ScrollDelta > WHEEL_DELTA; ScrollDelta -= WHEEL_DELTA)
            App->Scroll += 1;
        for (; ScrollDelta < 0; ScrollDelta += WHEEL_DELTA)
            App->Scroll -= 1;
    }



    LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_SIZE:
        if (App->Render->RenderDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            App->Render->DirectXParams.BackBufferWidth = LOWORD(lParam);
            App->Render->DirectXParams.BackBufferHeight = HIWORD(lParam);
            App->Render->Reset();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CApp::Exit() {
    exit(0);
}
std::vector<int> CApp::GetInputCharacters() {
    ImGui::GetIO().WantTextInput = true;
    std::vector<int> A;
    if (ImGui::GetIO().InputQueueCharacters.Size > 0) {
        for (auto c : ImGui::GetIO().InputQueueCharacters) {
            A.push_back(c);
        }
    }

    return A;
    /*
    std::vector<int> A;
    bool Capital = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    if (KeyToggled(VK_RETURN))
        A.push_back(VK_RETURN);

    if (KeyToggled(VK_ESCAPE))
        A.push_back(VK_ESCAPE);

    if (KeyToggled(VK_BACK))
        A.push_back(VK_BACK);

    if (KeyToggled(VK_SPACE))
        A.push_back(VK_SPACE);

    for (int i = 0; i < 10; i++) {
        if (KeyToggled(48 + i))
            A.push_back(48 + i);
    }
    for (int i = 0; i <= 25; i++) {
        if (KeyToggled(65 + i))
            if (KeyPressed(VK_LSHIFT) || KeyPressed(VK_RSHIFT) || Capital)
                A.push_back(65 + i);
            else
                A.push_back(97 + i);
    }
  
    return A;*/
}
void CApp::Init() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    SetProcessDPIAware();
    
  
    ScreenSize = Vec2(1200, 900);
 
    DefaultSearchFolder = "";
    {
        TCHAR* path = new TCHAR[256];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
        {
            DefaultSearchFolder = std::string(path);// + ("\\.NMRProjects");
        }
    }

    SceneContext->ScreenSize = ScreenSize;
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wcex.hIcon = LoadIcon(0, IDI_APPLICATION);
    wcex.hIconSm = LoadIcon(0, IDI_APPLICATION);
    wcex.lpszClassName = "Framework";
    wcex.lpszMenuName = nullptr;
    wcex.hInstance = GetModuleHandleA(NULL);
    wcex.lpfnWndProc = WndProc;
    RegisterClassEx(&wcex);
    Window = CreateWindowA("Framework", "Framework", CW_USEDEFAULT,
        (1920 / 2) - ScreenSize.x / 2, (1080 / 2) - ScreenSize.y / 2, ScreenSize.x, ScreenSize.y, nullptr, nullptr, GetModuleHandleA(NULL), nullptr);
   // SetWindowLongW(Window, GWL_EXSTYLE, 0);

    if (!App->Render->CreateDeviceD3D(Window)) {
        App->Render->Release();
        UnregisterClass(wcex.lpszClassName, wcex.hInstance);
        return;
    }


    ShowWindow(Window, SW_SHOW);
    UpdateWindow(Window);

    // Setup Dear ImGui context
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; //crutial for not leaving the imgui.ini file
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX9_Init(Render->RenderDevice);
   
  
    Render::Initialize();
    ShouldExit = false;
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        Curtime = (float)(clock());

        if (LastLoopTime == -1.f)
            LastLoopTime = Curtime;


        Interval = Curtime - LastLoopTime;


    

        LastLoopTime = Curtime;

  

    

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Render::DoRender();

       ImGui::EndFrame();

        Render->RenderDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
        if (Render->RenderDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());




            Render->RenderDevice->EndScene();
        }

 
        HRESULT result = Render->RenderDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && Render->RenderDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            Render->Reset();
        }
        if (ShouldExit) {
            msg.message = WM_QUIT;
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Render->Release();
    DestroyWindow(Window);
    UnregisterClass(wcex.lpszClassName, wcex.hInstance);
}
void CApp::UpdateKeyStates() {

    PrevMP = Vec2(ImGui::GetIO().MousePosPrev.x, ImGui::GetIO().MousePosPrev.y);
    MP = Vec2(ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

    std::copy(KeyStates, KeyStates + 255, OldKeyStates);
    for (unsigned int i = 0; i < 255; i++)
        KeyStates[i] = GetAsyncKeyState(i);

    Mouse1Click = KeyToggled(VK_LBUTTON);
    Mouse1Press =KeyPressed(VK_LBUTTON);
}

bool test = false;
#include "Fourier.h"

bool page = 0;
float stretchboundre = 5;
float stretchboundim = stretchboundre;

float Time = 0.f;
float TimeInterval = 0.f;
float NyquistFrequency = 0.f;
void graphdata(std::vector<I> val, Col rec, Col imc, bool text = false, bool v=false) {


    
    float delta = 1200 / val.size();




    for (int i = 0; i < val.size();i++) {

        if (i + 1 != val.size()) {



            Render::Line(i * delta, 900.f / 2.f - (val[i].a / stretchboundre) * (900.f / 2.f), (i + 1) * delta, 900.f / 2.f - (val[i + 1].a / stretchboundre) * (900.f / 2.f), rec, 2.f);
            Render::Line(i * delta, 900.f / 2.f - (val[i].b / stretchboundim) * (900.f / 2.f), (i + 1) * delta, 900.f / 2.f - (val[i + 1].b / stretchboundim) * (900.f / 2.f), imc, 2.f);

       
        }
        else {
            Render::Line(i * delta, 900.f / 2.f - (val[i].a / stretchboundre) * (900.f / 2.f), 1199, 900.f/2.f, rec, 2.f);
            Render::Line(i * delta, 900.f / 2.f - (val[i].b / stretchboundim) * (900.f / 2.f), 1199, 900.f / 2.f,imc, 2.f);

        }
        if(v)
            Render::FilledCircle(i * delta, 900.f / 2.f - (val[i].a / stretchboundre) * (900.f / 2.f), 3.f, Col(255, 255, 255, 255), 10);
       //if (text && val[i].a != 0.f) {
       //     Render::DrawString(i * delta - 10.f, 900.f / 2.f - (val[i].a / stretchboundre) * (900.f / 2.f) - 10.f, Col(), Fonts::MenuThin80, Render::centered_xy, ("(" + std::to_string(i * TimeInterval) + "Hz, " + std::to_string(val[i].a) + ")").c_str());
       // }
       // if (text && val[i].b != 0.f) {
        //    Render::DrawString(i * delta - 10.f, 900.f / 2.f - (val[i].b / stretchboundre) * (900.f / 2.f) - 10.f, Col(), Fonts::MenuThin80, Render::centered_xy, ("(" + std::to_string(i * TimeInterval) + "Hz, " + std::to_string(val[i].b) + ")").c_str());
        //}
    }
}

void graphdata2(std::vector<FourierPair> val, Col rec, Col imc, bool text) {



    float delta = 1200 / val.size();

    float maxbound = 0.f;
    for (auto& v : val) {
        if (v.Value.a > maxbound)
            maxbound = v.Value.a;

        if (v.Value.b > maxbound)
            maxbound = v.Value.b;
    }
  
    maxbound *= 1.13f;
    for (int i = 0; i < val.size(); i++) {

        if (val[i].Value.a != 0.f) {
            Render::FilledRect(i * delta, 900.f - (fabs(val[i].Value.a) / maxbound) * 900.f, delta,(fabs(val[i].Value.a) / maxbound) * 900.f, rec);
   
        }
        else  if (val[i].Value.b != 0.f) {
            Render::FilledRect(i * delta, 900.f - (fabs(val[i].Value.b) / maxbound) * 900.f,delta, (fabs(val[i].Value.b) / maxbound )* 900.f, imc);
        }

      
       
    }
}

void tx(Vec2 v, std::string a, Col b) {
    Render::FilledRect(v.x, v.y - 4, 8, 8, b);
    Render::DrawString(v.x + 11, v.y, Col(), Fonts::MenuThin100, Render::centered_y, a.c_str());
}

std::vector<I> a = {};
std::vector<I> out = {};
std::vector<I> testeq = {};
std::vector<FourierPair> predict = {};
std::vector<HarmonicEquation> equations = {};


int LastPage = -1;
void CApp::RenderScenes() {
    Render::FilledRect(0, 0, ScreenSize.x, ScreenSize.y, SC->Bg);
    SceneContext->AnimationModifier = Interval * 1.1f;

    if (SceneContext->CurrentPage != LastPage) {
        switch (SceneContext->CurrentPage) {
        case 0:
            SMStart();
            break;
        case 1:
            NFStart();
            break;
        }
    }
    LastPage = SceneContext->CurrentPage;


    switch (SceneContext->CurrentPage) {
    case 0:
        SMRender();
        break;
    case 1:
        NFRender();
        break;
    }
    GRender();

    /*
    if (!test) {
        test = true;

        std::vector<float> data = {};
        Time = 1.f; //sample time
        int samplespercycle = 102; //must be divisible by 2
        float samplerate = (float)( 1.f / samplespercycle);
        int totalcycle = 1;
        for (int c = 0; c < totalcycle; c++) {
            for (int i = 0; i < samplespercycle; i++) {
                data.push_back(sinf(i * samplerate * PI2)+sinf(0.5f* i * samplerate * PI2) + sinf(0.2f * i * samplerate * PI2)+ sinf(0.78f * i * samplerate * PI2));
            }
        }
       

    
        TimeInterval = Time / data.size(); //samples per second
        NyquistFrequency = 0.5f * TimeInterval;
        FourierTransformManual(data, out);

        for (auto& s : data) {
            a.push_back(I(s, 0.f));
        }
        FourierNormalize(out);
        int midpoint = std::floor(out.size() / 2.0);
        for (int i = 0; i < out.size(); i++) {
            predict.push_back(FourierPair{ (float)(-i + midpoint) / PI2,out[i] });
        }
     
        for (auto& A : predict) {
            //	float Sin;
           // float Cos;
           // float AmplitudeSin;
           // float AmplitudeCos;
            if (A.Value.a != 0.f && A.Value.b != 0.f) {
                equations.push_back({ A.Frequency * PI2, A.Frequency * PI2, A.Value.b, A.Value.a });
            }
            else if (A.Value.a != 0.f) {
                equations.push_back({ 0.f, A.Frequency * PI2, 0.f, A.Value.a });
            }
            else if (A.Value.b != 0.f) {
                equations.push_back({ A.Frequency * PI2, 0.f, A.Value.b, 0.f });
            }
        }
        //equations.clear();
       // equations.push_back({ 0.f, 1.f, 0.f, 1.f });
        for (int c = 0; c < totalcycle; c++) {
            for (int i = 0; i < samplespercycle; i++) {
              //  if (i % 2 != 0)
             //       continue;
                float f = 0.f;
                for (auto& eq : equations) {

                    if (eq.Sin != 0.f && eq.AmplitudeSin != 0.f) {
                        f += eq.AmplitudeSin * sinf(i * samplerate * PI2 * eq.Sin);
                    }
                    if (eq.Cos != 0.f && eq.AmplitudeCos != 0.f) {
                        f += eq.AmplitudeCos * cosf(i * samplerate * PI2 * eq.Cos);
                    }
             
                }
                testeq.push_back(I(f, 0.f));
            }
        }
    }

   // return;

    if (KeyToggled(0x4E)) {
        page = !page;
    }
        //nkey


    for (int i = 0; i < 10; i++) {
        int a = i / 10.f * stretchboundre;
        Render::DrawString(10, 900.f / 2.f - (i / 10.f) * (900.f / 2.f), Col(), Fonts::MenuThin100, Render::centered_y, std::to_string(a).c_str());
        a = -a;
        Render::DrawString(10, 900.f / 2.f + (i / 10.f) * (900.f / 2.f), Col(), Fonts::MenuThin100, Render::centered_y, std::to_string(a).c_str());

        Render::Line(30, 900.f / 2.f - (i / 10.f) * (900.f / 2.f), 1199, 900.f / 2.f - (i / 10.f) * (900.f / 2.f), Col(255, 255, 255, 100), 1.f);
        Render::Line(30, 900.f / 2.f + (i / 10.f) * (900.f / 2.f), 1199, 900.f / 2.f + (i / 10.f) * (900.f / 2.f), Col(255, 255, 255, 100), 1.f);
    }
    for (int i = 0; i < 20; i++) {
        
        if(i != 0 && i != 20) 
            Render::Line(i / 20.f * 1200,0, i / 20.f * 1200,900.f, Col(255, 255, 255, 100), 1.f);
 
    }

    Render::DrawString(1200 - 300, 20, Col(), Fonts::MenuThin100, Render::centered_y, "Fourier Transform");

    tx(Vec2(70, 20), "Input Re()", Col(0, 255, 255));
    tx(Vec2(70, 45), "Output Re()", Col(0, 0, 255));
    tx(Vec2(70, 70), "Output Im()", Col(255, 0, 0));
  

       graphdata(testeq, Col(255, 0, 255), Col(0, 0, 0), false);

        graphdata2(predict, Col(0, 0, 255), Col(255, 0, 0), false);
        graphdata(a, Col(0, 255, 255), Col(0, 0, 0), false, true);*/
}

CApp* App = new CApp();