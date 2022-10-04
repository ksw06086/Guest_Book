#include "Canvas.h"
#include "Window.h"

Canvas::Canvas(HWND hWnd)
{
	this->hWnd = hWnd;

	x_ = 0;
	y_ = Window::GetInstance()->GetControl()->GetHeight();
	width_ = Window::GetInstance()->GetWindowArea().right - Window::GetInstance()->GetFileManager()->GetWidth();
	height_ = (Window::GetInstance()->GetWindowArea().bottom - y_) - Window::GetInstance()->GetTimeline()->GetHeight();
}

void Canvas::CanvasReset()
{
	lines_.clear();
}

void Canvas::MouseUp()
{
	if (is_canvas_click_)
	{
		is_canvas_click_ = false;
		RECT timeline_area = Window::GetInstance()->GetTimeline()->GetTimelineArea();

		if (line_.size() != 0)
		{
			lines_.push_back(line_);
			line_.clear();
			InvalidateRect(hWnd, &timeline_area, FALSE);
		}
	}
}

void Canvas::MouseDown(POINT mouse_position)
{
	if (PtInRect(&canvas_area_, mouse_position))
	{
		is_canvas_click_ = true;
		mouse_current_x_ = mouse_position.x;
		mouse_current_y_ = mouse_position.y;
	}
}

void Canvas::MouseMove(POINT mouse_position, int width, double time, COLORREF color)
{
	if (is_canvas_click_)
	{
		if (!PtInRect(&canvas_area_, mouse_position))
		{
			MouseUp();
			return;
		}

		HDC hdc;
		hdc = GetDC(hWnd);
		HPEN n = CreatePen(PS_SOLID, width, color);
		HPEN o = (HPEN)SelectObject(hdc, n);
		MoveToEx(hdc, mouse_current_x_, mouse_current_y_, NULL);
		LineTo(hdc, mouse_position.x, mouse_position.y);
		line_.push_back({ mouse_current_x_ - canvas_x_, mouse_current_y_ - canvas_y_, mouse_position.x - canvas_x_, mouse_position.y - canvas_y_, width, time, color });
		SelectObject(hdc, o);
		DeleteObject(n);
		ReleaseDC(hWnd, hdc);
		mouse_current_x_ = mouse_position.x;
		mouse_current_y_ = mouse_position.y;
	}
}

void Canvas::Draw(HDC hdc)
{
	Graphics graphics(hdc);

	// 배경 제거
	SetBkMode(hdc, TRANSPARENT);

	Image edit_icon(L"Resources/EditIcon.png");

	SolidBrush background_brush(Color(255, 219, 219, 219));
	SolidBrush background_brush2(Color(255, 238, 238, 238));
	SolidBrush white_brush(Color(255, 255, 255, 255));
	SolidBrush black_brush(Color(255, 0, 0, 0));

	Pen contour_pen(Color(255, 185, 185, 185));

	FontFamily arial_font(L"Arial");
	Font font_style(&arial_font, 12, FontStyleBold, UnitPixel);

	y_ = Window::GetInstance()->GetControl()->GetHeight();
	width_ = Window::GetInstance()->GetWindowArea().right - Window::GetInstance()->GetFileManager()->GetWidth();
	height_ = (Window::GetInstance()->GetWindowArea().bottom - y_) - Window::GetInstance()->GetTimeline()->GetHeight();

	graphics.FillRectangle(&background_brush, x_, y_, width_, height_);

	graphics.FillRectangle(&background_brush2, x_, y_ + 60, width_, height_ - 90);

	Region region(Rect(x_, y_ + 60, width_, height_ - 90));

	// 클리핑 마스크 시작
	graphics.SetClip(&region, CombineModeReplace);

	// 윈도우 크기에 따른 위치 보정
	canvas_width_ = 1000;
	canvas_height_ = 500;
	canvas_x_ = ((x_ + width_) - canvas_width_) / 2;
	canvas_y_ = (((y_ + height_) + 90) - canvas_height_) / 2;

	graphics.FillRectangle(&white_brush, canvas_x_, canvas_y_, canvas_width_, canvas_height_);

	canvas_area_ = { canvas_x_, canvas_y_, canvas_x_ + canvas_width_, canvas_y_ + canvas_height_ };

	if (!Window::GetInstance()->GetTimeline()->OnPlay())
	{
		for (size_t i = 0; i < lines_.size(); i++)
		{
			for (size_t j = 0; j < lines_[i].size(); j++)
			{
				DrawLine(hdc, i, j);
			}
		}
	}
	else
	{
		/*아래와 같은 방식으로 계속해서 다시 그리는 방식을 선택한 이유는
		순차적인 탐색을 하면서 조금씩 조금씩 그리는 경우 비용이 비싸며,
		현재 사용하고 있는 타이머의 구조상 재시간안에 모두 실행할 수 없어
		문제가 발생하기 때문에 아래와 같은 방식을 사용하였다.*/

		for (size_t i = 0; i < lines_.size(); i++)
		{
			for (size_t j = 0; j < lines_[i].size(); j++)
			{
				if ((int)trunc(lines_[i][j].time * 1000) > Window::GetInstance()->GetTimeline()->GetTime())
				{
					break;
				}

				DrawLine(hdc, i, j);
			}
		}
	}

	// 클리핑 마스크 종료
	graphics.ResetClip();

	/*기존 GDI 그래픽 라이브러리에서는 클리핑을 사용할 수 없기 때문에
	다시 한번 더 그리도록 했다.*/
	graphics.FillRectangle(&background_brush, x_, y_, width_, 30);

	// 상단바
	Point background_points[] = {
		Point(x_, y_),
		Point(x_, y_ + 60),
		Point(x_ + width_, y_ + 60),
		Point(x_ + width_, y_ + 30),
		Point(x_ + 90, y_ + 30),
		Point(x_ + 90, y_) };

	Point contour_points[] = {
		Point(x_, y_),
		Point(x_, y_ + 59),
		Point(x_ + width_ - 1, y_ + 59),
		Point(x_ + width_ - 1, y_ + 30),
		Point(x_ + 90, y_ + 30),
		Point(x_ + 90, y_) };

	graphics.FillPolygon(&background_brush2, background_points, 6);
	graphics.DrawPolygon(&contour_pen, contour_points, 6);

	graphics.DrawImage(&edit_icon, x_ + 10, y_ + 5, 20, 20);

	StringFormat string_format;
	string_format.SetLineAlignment(StringAlignmentCenter);

	PointF header_font_position(x_ + 35, y_ + 15);
	graphics.DrawString(L"Canvas", -1, &font_style, header_font_position, &string_format, &black_brush);

	graphics.DrawRectangle(&contour_pen, x_, y_, width_ - 1, height_ - 1);

	// 하단바
	graphics.FillRectangle(&background_brush2, x_, y_ + height_ - 30, width_, 30);
	graphics.DrawRectangle(&contour_pen, x_, y_ + height_ - 30, width_ - 1, 29);
}

void Canvas::DrawLine(HDC hdc, size_t lines_idx, size_t line_idx)
{
	HPEN n = CreatePen(PS_SOLID, lines_[lines_idx][line_idx].width, lines_[lines_idx][line_idx].color);
	HPEN o = (HPEN)SelectObject(hdc, n);
	MoveToEx(hdc, lines_[lines_idx][line_idx].start_x + canvas_x_, lines_[lines_idx][line_idx].start_y + canvas_y_, NULL);
	LineTo(hdc, lines_[lines_idx][line_idx].end_x + canvas_x_, lines_[lines_idx][line_idx].end_y + canvas_y_);
	SelectObject(hdc, o);
	DeleteObject(n);
}

void Canvas::OpenSaveFile()
{
	WCHAR file_name[256] = L"";

	OPENFILENAME OFN;
	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWnd;
	OFN.lpstrFilter = L"Guest Book(*.gb)\0*.gb";
	OFN.lpstrDefExt = L"gb";
	OFN.lpstrFile = file_name;
	OFN.nMaxFile = 256;
	OFN.lpstrInitialDir = Window::GetInstance()->GetFileManager()->GetRootPath();
	OFN.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

	size_t size = lines_.size();

	if (GetSaveFileName(&OFN) != 0)
	{
		ofstream save(OFN.lpstrFile, ios::binary);
		if (save.is_open())
		{
			if (size != 0)
			{
				save.write((const char*)&size, 4);

				for (size_t i = 0; i < lines_.size(); i++)
				{
					size_t line_size = lines_[i].size();

					save.write((const char*)&line_size, 4);
					save.write((const char*)&lines_[i][0], line_size * sizeof(PointInfo));
				}

			}

			save.close();
		}
	}
}

void Canvas::OpenLoadFile()
{
	WCHAR file_name[256] = L"";
	WCHAR path[1024] = L"";

	OPENFILENAME OFN;
	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWnd;
	OFN.lpstrFilter = L"Guest Book(*.gb)\0*.gb";
	OFN.lpstrDefExt = L"gb";
	OFN.lpstrFile = file_name;
	OFN.nMaxFile = 256;
	OFN.lpstrInitialDir = Window::GetInstance()->GetFileManager()->GetRootPath();
	OFN.Flags = OFN_NOCHANGEDIR;

	if (GetOpenFileName(&OFN) != 0)
	{
		wsprintf(path, L"%s", OFN.lpstrFile);
		LoadFile(path);
	}
}

void Canvas::LoadFile(fs::path path)
{
	size_t size = 0;

	ifstream load(path, ios::binary);
	if (load.is_open())
	{
		load.read((char*)&size, 4);
		lines_.resize(size);

		for (size_t i = 0; i < lines_.size(); i++)
		{
			size_t line_size = 0;
			load.read((char*)&line_size, 4);

			lines_[i].resize(line_size);
			load.read((char*)&lines_[i][0], line_size * sizeof(PointInfo));
		}

		load.close();
	}

	Window::GetInstance()->SetTime(lines_[lines_.size() - 1][lines_[lines_.size() - 1].size() - 1].time);
	Window::GetInstance()->GetTimeline()->UpdateMaxTime(lines_[lines_.size() - 1][lines_[lines_.size() - 1].size() - 1].time);

	InvalidateRect(hWnd, &canvas_area_, FALSE);
}

bool Canvas::OnCanvasClick()
{
	return is_canvas_click_;
}

RECT Canvas::GetCanvasArea()
{
	return canvas_area_;
}

int Canvas::GetWidth()
{
	return width_;
}

int Canvas::GetHeight()
{
	return height_;
}

vector<vector<Canvas::PointInfo>> Canvas::GetLines()
{
	return lines_;
}

void Canvas::Undo()
{
	// 개발 중
	if (lines_.size() > 0)
	{
		lines_.pop_back();
		Window::GetInstance()->SetTime(lines_[lines_.size() - 1][lines_[lines_.size() - 1].size() - 1].time);
		Window::GetInstance()->GetTimeline()->UpdateMaxTime(lines_[lines_.size() - 1][lines_[lines_.size() - 1].size() - 1].time);

		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void Canvas::Redo()
{

}