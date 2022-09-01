﻿// GuestBookEditor.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "GuestBookEditor.h"
#include "mmsystem.h"
#include<vector>
#include<iostream>
#include <Ole2.h>
#include <gdiplus.h>
#include<algorithm>

using namespace std;
using namespace Gdiplus;

#pragma comment(lib, "winmm.lib");
#pragma comment(lib, "Gdiplus.lib");

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 윈도우 크기
const int windows_size_width = 1280;
const int windows_size_height = 720;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    GdiplusStartupInput gdiplus_startup_input;
    ULONG_PTR gdiplus_token;

    GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, NULL);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GUESTBOOKEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
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

    GdiplusShutdown(gdiplus_token);
    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
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

void OnPaint(HDC hdc, LPARAM lParam)
{
    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 0, 0));
    graphics.DrawRectangle(&pen, 0, 0, 100, 100);
    graphics.DrawRectangle(&pen, 100, 0, 100, 100);
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
bool is_click = false;
int pre_x;
int pre_y;
int current_x;
int current_y;
RECT r;
RECT r1;
POINT point;
std::vector<std::vector<POINT>> list;   // 사용자가 그린 그림을 저장할 2차원 벡터


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
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
    case WM_LBUTTONDOWN:
    {
        is_click = true;
        r = { 0,0,100,100 };
        r1 = { 100, 0, 200, 100 };
        point.x = LOWORD(lParam);
        point.y = HIWORD(lParam);
        std::vector<POINT> tempv;   // 그림을 저장할 임시 벡터
        list.push_back(tempv);
        list[list.size() - 1].push_back(point);     // 계속해서 vector를 추가하고 접근하기
        tempv.clear();
        std::vector<POINT>().swap(tempv);


        if (PtInRect(&r, point))
        {
            is_click = false;
            POINT pos;  
            // 지우는 좌표 받아오기
            pos.x = LOWORD(lParam);
            pos.y = HIWORD(lParam);




        }
        if (PtInRect(&r1, point))
        {
            is_click = false;
            MessageBox(hWnd, TEXT("2번째 버튼이 눌렸습니다."), TEXT("메시지 박스"), MB_ICONINFORMATION | MB_RETRYCANCEL);
        }
       
    }
    break;
    case WM_LBUTTONUP:
    {
        is_click = false;
    }
    break;
    
    case WM_MOUSEMOVE:
    {
        HDC hdc;

        hdc = GetDC(hWnd);

        

        if (true == is_click)
        {
            Graphics graphics(hdc);
            Pen pen(Color(255, 0, 0, 0));

            current_x = LOWORD(lParam);
            current_y = HIWORD(lParam);
            list[list.size()].push_back(point);

            graphics.DrawLine(&pen, point.x, point.y, current_x, current_y);
            
            point.x = current_x;
            point.y  = current_y;
        }
        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);
        ReleaseDC(hWnd, hdc);
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
        FillRect(hdc, &buffer, (HBRUSH)GetStockObject(WHITE_BRUSH));

        for (int i = 0; i < list.size(); i++)
        {
            POINT st_pos = list[i].at(0);

            for (int j = 0; j < list[i].size(); j++)
            {
                POINT next_pos = list[i].at(j);
                MoveToEx(memDC, st_pos.x, st_pos.y, NULL);
                LineTo(memDC, next_pos.x, next_pos.y);
                st_pos = next_pos;
            }
        }

        OnPaint(hdc, lParam);

        GetClientRect(hWnd, &buffer);
        BitBlt(memDC, 0, 0, buffer.right, buffer.bottom, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, oldBitmap);
        DeleteObject(newBitmap);
        DeleteDC(hdc);
        EndPaint(hWnd, &ps);
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