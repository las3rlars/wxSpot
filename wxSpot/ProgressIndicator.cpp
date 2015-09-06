#include "ProgressIndicator.h"

BEGIN_EVENT_TABLE(ProgressIndicator, wxPanel)
	EVT_LEFT_DOWN(ProgressIndicator::mouseDown)
	EVT_PAINT(ProgressIndicator::OnPaint)
	EVT_SIZE(ProgressIndicator::OnSize)
END_EVENT_TABLE()

wxDEFINE_EVENT(PI_SCROLL_CHANGED, wxCommandEvent);

ProgressIndicator::ProgressIndicator(wxWindow *parent) : wxWindow(parent, wxID_ANY)
{

	width = 200;
	height = 20;
	value = 0;
	SetMinSize(wxSize(width, height));
	SetMaxSize(wxSize(-1, height));
}


ProgressIndicator::~ProgressIndicator()
{
}


void ProgressIndicator::OnSize(wxSizeEvent &event)
{
	width = event.GetSize().GetWidth();
	Refresh();
}

void ProgressIndicator::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	render(dc);
}

void ProgressIndicator::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}


void ProgressIndicator::render(wxDC &dc)
{

	dc.SetBrush(*wxLIGHT_GREY_BRUSH);

	dc.DrawRectangle(0, 0, width, height);
	dc.SetBrush(*wxGREY_BRUSH);
	dc.DrawRectangle(0, 1, width * value, height - 2);
}

void ProgressIndicator::mouseDown(wxMouseEvent &event)
{
	this->value = (float)event.GetPosition().x / (float)width;
	paintNow();

	wxCommandEvent pressEvent = wxCommandEvent(PI_SCROLL_CHANGED);
	
	QueueEvent(pressEvent.Clone());
}

void ProgressIndicator::SetValue(float value)
{
	this->value = value;
	paintNow();
}