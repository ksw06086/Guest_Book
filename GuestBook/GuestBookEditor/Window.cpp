#include "Window.h"

// 멤버 변수 초기화
unique_ptr<Window> Window::instance_ = nullptr;
once_flag Window::flag_;

// 창 클래스를 등록
ATOM Window::MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = StaticWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUESTBOOKEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GUESTBOOKEDITOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// 인스턴스 핸들을 저장하고 주 창을 만듬
BOOL Window::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        0, 0, 1280, 720, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 주 창의 메시지를 처리
LRESULT Window::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return instance_->WndProc(hWnd, message, wParam, lParam);
}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static unique_ptr<QuickPanel> quick_panel;
    static unique_ptr<FileManager> file_manager;

    TIMECAPS timecaps;
    timeGetDevCaps(&timecaps, sizeof(TIMECAPS));
    
    POINT mouse_position;

    switch (message)
    {
    case WM_CREATE:
    {
        GetClientRect(hWnd, &client_area_);
        window_area_ = { 0, 0, client_area_.right - client_area_.left, client_area_.bottom - client_area_.top };

        quick_panel = make_unique<QuickPanel>(hWnd);
        timeline_ = make_unique<Timeline>(hWnd);
        canvas_ = make_unique<Canvas>(hWnd, window_area_.right - 100, 300);
        file_manager = make_unique<FileManager>(hWnd);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_NEW_FILE:
            canvas_->Reset();
            timer_ = 0;
            timeline_->UpdateMaxTime(0);
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case IDM_SAVE:
            canvas_->SaveCanvas();
            break;
        case IDM_LOAD:
            canvas_->LoadCanvas("");
            timer_ = canvas_->GetPoints()[canvas_->GetPoints().size() - 1].time;
            timeline_->UpdateMaxTime(canvas_->GetPoints()[canvas_->GetPoints().size() - 1].time);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        GetClientRect(hWnd, &client_area_);
        window_area_ = { 0, 0, client_area_.right - client_area_.left, client_area_.bottom - client_area_.top };

        PAINTSTRUCT ps;
        HDC hdc, memDC;
        HBITMAP newBitmap, oldBitmap;
        RECT buffer;
        memDC = BeginPaint(hWnd, &ps);

        GetClientRect(hWnd, &buffer);
        hdc = CreateCompatibleDC(memDC);
        newBitmap = CreateCompatibleBitmap(memDC, buffer.right, buffer.bottom);
        oldBitmap = (HBITMAP)SelectObject(hdc, newBitmap);
        PatBlt(hdc, 0, 0, buffer.right, buffer.bottom, WHITENESS);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

        canvas_->Draw(hdc);
        timeline_->Draw(hdc);
        file_manager->Draw(hdc);
        OnPaint(hdc);
        quick_panel->Draw(hdc);

        GetClientRect(hWnd, &buffer);
        BitBlt(memDC, 0, 0, buffer.right, buffer.bottom, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, oldBitmap);
        DeleteObject(newBitmap);
        DeleteDC(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_LEFT:
            timeline_->Play();

            if (play_timer_ == NULL)
            
            {
                play_timer_ = timeSetEvent(1, timecaps.wPeriodMax, TimerProc, (DWORD_PTR)this, TIME_PERIODIC);
            }
            else
            {
                timeKillEvent(play_timer_);
                play_timer_ = NULL;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }
    }
    break;
    case WM_LBUTTONUP:
    {
        mouse_position.x = LOWORD(lParam);
        mouse_position.y = HIWORD(lParam);

        quick_panel->MouseUp();
        timeline_->MouseUp();
        canvas_->MouseUp();
        file_manager->MouseUp();

        timeKillEvent(drawing_timer_);
        drawing_timer_ = NULL;
    }
    break;
    case WM_LBUTTONDOWN:
    {
        mouse_position.x = LOWORD(lParam);
        mouse_position.y = HIWORD(lParam);

        quick_panel->MouseDown(mouse_position);

        if (!quick_panel->IsOpen() and !timeline_->IsPlaying())
        {
            timeline_->MouseDown(mouse_position);
            canvas_->MouseDown(mouse_position);
            file_manager->MouseDown(mouse_position);

            canvas_->LoadCanvas(file_manager->GetListBoxItem()[file_manager->GetIndex()].file_path.string());
            timer_ = canvas_->GetPoints()[canvas_->GetPoints().size() - 1].time;
            timeline_->UpdateMaxTime(canvas_->GetPoints()[canvas_->GetPoints().size() - 1].time);
        }
    }

    break;
    case WM_LBUTTONDBLCLK:
    {
        mouse_position.x = LOWORD(lParam);
        mouse_position.y = HIWORD(lParam);

        if (!timeline_->IsPlaying())
        {
            quick_panel->Open(mouse_position);
        }
    }
    break;
    case WM_MOUSEMOVE:
    {
        mouse_position.x = LOWORD(lParam);
        mouse_position.y = HIWORD(lParam);

        quick_panel->MouseMove(mouse_position);
        timeline_->MouseMove(mouse_position);
        file_manager->MouseMove(mouse_position);

        canvas_->MouseMove(mouse_position, quick_panel->GetPenSize(), timer_, quick_panel->GetRGB());

        if (canvas_->IsCanvasClick())
        {
            if (drawing_timer_ == NULL)
            {
                drawing_timer_ = timeSetEvent(1, timecaps.wPeriodMax, TimerProc, (DWORD_PTR)this, TIME_PERIODIC);
            }

            timeline_->UpdateMaxTime(timer_);

        }
        else
        {
            timeKillEvent(drawing_timer_);
            drawing_timer_ = NULL;
        }
    }
    break;
    case WM_DESTROY:
        // 수동으로 메모리 해제 시 사용(일반적으로는 자동으로 메모리를 해제)
        /*timeline_.reset();
        canvas_.reset();
        quick_panel.reset();*/

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기
INT_PTR CALLBACK Window::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CALLBACK Window::TimerProc(UINT m_nTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    RECT area = { 0, 0, 500, 30 };
    Window* window = (Window*)dwUser;

    HDC hdc;
    hdc = GetDC(window->hWnd);

    if (m_nTimerID == window->drawing_timer_)
    {
        window->timer_ += 0.001;
        InvalidateRect(window->hWnd, &area, FALSE);
    }

    if (m_nTimerID == window->play_timer_)
    {
        window->timeline_->AddTime(0.001);
        InvalidateRect(window->hWnd, NULL, FALSE);
    }

    ReleaseDC(window->hWnd, hdc);
}

void Window::OnPaint(HDC hdc)
{
    /*Graphics graphics(hdc);

    FontFamily arial_font(L"Arial");
    Font font_style(&arial_font, 12, FontStyleRegular, UnitPixel);
    SolidBrush black_brush(Color(255, 0, 0, 0));

    Region region(Rect(0, 0, 100, 50));
    graphics.SetClip(&region, CombineModeReplace);

    fs::path p("./Guests");
    fs::directory_iterator iter(p);
    int count = 0;
    WCHAR header_word[1024];

    while (iter != fs::end(iter))
    {
        const fs::directory_entry& entry = *iter;
        auto a = entry.path().c_str();
        wsprintf(header_word, a);
        PointF header_font_position(10, 10 + (count * 13));
        graphics.DrawString(header_word, -1, &font_style, header_font_position, &black_brush);
        iter++;
        count++;
    }*/

    if (timeline_->IsPlaying() == false)
    {
        for (int i = 0; i < canvas_->GetPoints().size(); i++)
        {
            canvas_->DrawLine(hdc, i);
        }
    }
    else
    {
        /*아래와 같은 방식으로 계속해서 다시 그리는 방식을 선택한 이유는
        순차적인 탐색을 하면서 조금씩 조금씩 그리는 경우 비용이 비싸며,
        현재 사용하고 있는 타이머의 구조상 재시간안에 모두 실행할 수 없어
        문제가 발생하기 때문에 아래와 같은 방식을 사용하였다.*/

        for (int i = 0; i < canvas_->GetPoints().size(); i++)
        {
            if ((int)trunc(canvas_->GetPoints()[i].time * 1000) <= timeline_->GetTime())
            {
                canvas_->DrawLine(hdc, i);
            }
        }
    }
}

Window* Window::GetInstance()
{
    call_once(flag_, []() // 람다식
        {
            instance_.reset(new Window);
        });

    return instance_.get();
}
