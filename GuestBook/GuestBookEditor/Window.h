#pragma once

#include "resource.h"
#include "framework.h"
#include "QuickPanel.h"
#include "Timeline.h"
#include "Canvas.h"

#define MAX_LOADSTRING 100

class Window
{
private:
	Window();
	~Window();

	HINSTANCE hInst; // ���� �ν��Ͻ�
	HWND hWnd;

	static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
	static void CALLBACK TimerProc(UINT m_nTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	void OnPaint(HDC hdc);

	// �̱���
	static Window* instance_;

	RECT client_area_;
	RECT window_area_;

	UINT drawing_timer_;
	UINT play_timer_;

	double timer_;

	Timeline* timeline_;
	Canvas* canvas_;

	int current_x_;
	int current_y_;
public:
	WCHAR szTitle[MAX_LOADSTRING]; // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
	WCHAR szWindowClass[MAX_LOADSTRING]; // �⺻ â Ŭ���� �̸��Դϴ�.

	ATOM MyRegisterClass(HINSTANCE hinstance);
	BOOL InitInstance(HINSTANCE, int);

	static void Create();
	static Window* GetInstance();
};
