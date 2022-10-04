#pragma once

#include "framework.h"
#include "Button.h"

class Control
{
private:
	HWND hWnd;

	int x_;
	int y_;
	int width_;
	int height_;

	unique_ptr<Button> button_undo_;
	unique_ptr<Button> button_redo_;
	unique_ptr<Button> button_color_;
	unique_ptr<Button> button_play_;
public:
	Control(HWND hWnd);
	~Control() = default;

	void MouseDown(POINT mouse_position);
	void Draw(HDC hdc);

	int GetWidth();
	int GetHeight();
};
