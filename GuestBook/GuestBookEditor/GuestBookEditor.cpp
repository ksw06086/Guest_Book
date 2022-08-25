﻿// GuestBookEditor.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GuestBookEditor.h"

#include "mmsystem.h"

#include <vector>

#pragma comment(lib, "winmm.lib");

using namespace std;

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 윈도우 크기
const int windows_size_width = 1280;
const int windows_size_height = 720;

// 타임라인 진행률
float progress = 0; // 현재 진행률
float max_progress = 0; // 진행률 최대치 (추후 기록된 시간의 합으로 값을 적용하여 동작)

UINT progress_timer;

ULONGLONG create_time;
ULONGLONG current_time;

bool is_progress_click = false;

bool is_click = false;
int current_x;
int current_y;

struct Point
{
    int current_x;
    int current_y;
    int x;
    int y;
    float time;
};

vector<Point> a;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GUESTBOOKEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUESTBOOKEDITOR));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUESTBOOKEDITOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GUESTBOOKEDITOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,
      0, 0, windows_size_width, windows_size_height, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
        }
        break;
    case WM_KEYDOWN:
        {
            TIMECAPS timecaps;
            timeGetDevCaps(&timecaps, sizeof(TIMECAPS));
            
            switch (wParam)
            {
            case VK_LEFT:
                InvalidateRect(hWnd, NULL, FALSE);
                if (progress_timer != 0)
                {
                    timeKillEvent(progress_timer);
                }
                
                progress_timer = timeSetEvent(1, timecaps.wPeriodMax, TimerProc, (DWORD)hWnd, TIME_PERIODIC);
                break;
            case VK_RIGHT:
                timeKillEvent(progress_timer);
                progress_timer = 0;
                break;
            case VK_UP:
                current_time = (DWORD)GetTickCount64() - create_time;
                max_progress = (float)current_time / 1000;
                
                RECT r = {0, windows_size_height - 220, windows_size_width, windows_size_height};
                InvalidateRect(hWnd, &r, FALSE);
                break;
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            ReleaseCapture();
            is_progress_click = false;
            is_click = false;
        }
        break;
    case WM_LBUTTONDOWN:
        {
            SetCapture(hWnd);
            if (create_time == 0)
            {
                create_time = (DWORD)GetTickCount64();
            }
            
            POINT point;
            RECT r = {0, windows_size_height - 220, windows_size_width, windows_size_height};
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);
            if (PtInRect(&r, point))
            {
                progress = min(max((point.x * max_progress) / 500, 0), max_progress);
                InvalidateRect(hWnd, &r, FALSE);
                is_progress_click = true;
            }

            is_click = true;
            current_x = LOWORD(lParam);
            current_y = HIWORD(lParam);
        }
        break;
    case WM_MOUSEMOVE:
        {
            RECT r = {0, windows_size_height - 220, windows_size_width, windows_size_height};
            RECT ar = {0, 0, windows_size_width, windows_size_height - 220};
            if (is_progress_click)
            {
                int x = LOWORD(lParam);
                progress = min(max((x * max_progress) / 500, 0), max_progress);
                InvalidateRect(hWnd, &r, FALSE);
                InvalidateRect(hWnd, &ar, FALSE);
            }
            
            POINT point;
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);
            if (is_click && PtInRect(&ar, point))
            {
                int x, y;
                Point p;
                HDC hdc;
                hdc = GetDC(hWnd);
                x = LOWORD(lParam);
                y = HIWORD(lParam);
                MoveToEx(hdc, current_x, current_y, NULL);
                LineTo(hdc, x, y);
                current_time = (DWORD)GetTickCount64() - create_time;
                max_progress = (float)current_time / 1000;
                p.current_x = current_x;
                p.current_y = current_y;
                p.x = x;
                p.y = y;
                p.time = (float)current_time / 1000;
                a.push_back(p);
                InvalidateRect(hWnd, &r, FALSE);
                ReleaseDC(hWnd, hdc);
                current_x = x;
                current_y = y;
            }
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
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

            WCHAR word[1024];
            WCHAR progress_word[1024];
            SetBkMode(hdc, TRANSPARENT);
            _stprintf_s(progress_word, L"%.3fs / %.3fs", progress, max_progress);
            TextOut(hdc, 0, windows_size_height - 220, progress_word, lstrlen(progress_word));
            wsprintf(word, L"%d", a.size());
            TextOut(hdc, 500, windows_size_height - 220, word, lstrlen(word));
            Rectangle(hdc, 0, windows_size_height - 200, 500, windows_size_height - 190);
            COLORREF c = RGB(30, 33, 35, 39);
            HBRUSH n = CreateSolidBrush(c);
            HBRUSH o = (HBRUSH)SelectObject(hdc, n);
            Rectangle(hdc, 0, windows_size_height - 200, (progress / max_progress) * 500, windows_size_height - 190);
            SelectObject(hdc, o);
            DeleteObject(n);

            POINT handle[5];
            handle[0].x = (progress / max_progress) * 500 - 5;
            handle[0].y = windows_size_height - 210;
            handle[1].x = (progress / max_progress) * 500 - 5;
            handle[1].y = windows_size_height - 200;
            handle[2].x = (progress / max_progress) * 500;
            handle[2].y = windows_size_height - 190;
            handle[3].x = (progress / max_progress) * 500 + 5;
            handle[3].y = windows_size_height - 200;
            handle[4].x = (progress / max_progress) * 500 + 5;
            handle[4].y = windows_size_height - 210;
            Polygon(hdc, handle, 5);
            
            GetClientRect(hWnd, &buffer);
            BitBlt(memDC, 0, 0, buffer.right, buffer.bottom, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, oldBitmap);
            DeleteObject(newBitmap);
            DeleteDC(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_TIMER:
        {
            switch (wParam)
            {
                default:
                    break;
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

void A(HDC hdc, int idx)
{
    if (floor(a[idx].time * 1000) == floor(progress * 1000))
    {
        MoveToEx(hdc, a[idx].current_x, a[idx].current_y, NULL);
        LineTo(hdc, a[idx].x, a[idx].y);
    }
}

void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    
    if (uTimerID == progress_timer)
    {
        RECT r = {0, windows_size_height - 220, windows_size_width, windows_size_height};
    
        progress += 0.001;
        InvalidateRect((HWND)dwUser, &r, FALSE);
        
        if (floor(progress * 1000) >= floor(max_progress * 1000))
        {
            progress = 0;
            InvalidateRect((HWND)dwUser, NULL, FALSE);
        }
    }
}
