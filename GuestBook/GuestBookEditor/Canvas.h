#pragma once

#include "framework.h"

#include <vector>

class Canvas
{
private:
	struct PointInfo
	{
		int start_x;
		int start_y;
		int end_x;
		int end_y;
		int width;

		double time;

		COLORREF color;
	};

	HWND hWnd;
	RECT client_area_;
	RECT window_area_;

	int x_;
	int y_ = 50;
	int width_;
	int height_;

	RECT canvas_area_;

	bool is_canvas_click_;

	vector<PointInfo> points_;

	int current_x;
	int current_y;

	void UpdateWindowArea();
public:
	Canvas(HWND hWnd, int width, int height);
	~Canvas();

	void MouseUp();
	void MouseDown(POINT mouse_position);
	void MouseMove(POINT mouse_position, int width, double time, COLORREF color);
	void Draw(HDC hdc);
	void UpdateDraw(HDC hdc);

	bool IsCanvasClick();

	vector<PointInfo> GetPoints();
};
