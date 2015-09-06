#include "Visualizer.h"

BEGIN_EVENT_TABLE(Visualizer, wxPanel)
	EVT_PAINT(Visualizer::OnPaint)
END_EVENT_TABLE()

Visualizer::Visualizer(wxWindow *parent) : wxWindow(parent, wxID_ANY)
{
	width = 70;
	height = 25;
	SetMinSize(wxSize(width, height));
	buffer = nullptr;
}


Visualizer::~Visualizer()
{
}

void Visualizer::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	render(dc);
}

void Visualizer::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void Visualizer::render(wxDC &dc)
{
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	dc.DrawRectangle(0, 0, width, height);
	int mul = 512 / width - 2;
	if (buffer != nullptr) {
		dc.SetPen(*wxBLACK_PEN);
		for (unsigned int i = 0; i < width - 2; i++) {
			int temp = buffer[i * mul];
			temp = temp << scale;
			float val = temp / 32767.0f;
			val *= 0.1f;
			int pos = val * (height / 2) + (height / 2);
			dc.DrawPoint(i+1, pos);
		}
	}
}

void Visualizer::update(short int *fftBuffer, unsigned int size, int scale)
{
	buffer = fftBuffer;
	this->scale = scale;
	Refresh(false);
}