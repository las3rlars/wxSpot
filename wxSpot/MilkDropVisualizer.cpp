#include "MilkDropVisualizer.h"

#include <projectM.hpp>


BEGIN_EVENT_TABLE(MilkDropVisualizer, wxGLCanvas)
EVT_PAINT(MilkDropVisualizer::OnPaint)
EVT_SIZE(MilkDropVisualizer::OnSize)
END_EVENT_TABLE()

MilkDropVisualizer::MilkDropVisualizer(wxFrame *parent, int *args) : wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	m_context = new wxGLContext(this);
	SetMinSize(wxSize(64, 64));
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	SetCurrent(*m_context);

	//m_projectM = new projectM("projectM/config.inp");
	m_projectM = std::make_unique<projectM>("projectM/config.inp");

}


MilkDropVisualizer::~MilkDropVisualizer()
{
	//delete m_projectM;
}

void MilkDropVisualizer::OnPaint(wxPaintEvent &event)
{
	SetCurrent(*m_context);
	wxPaintDC(this);

	if (!IsShownOnScreen()) return;

	//glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_projectM->renderFrame();

	SwapBuffers();
}

void MilkDropVisualizer::OnSize(wxSizeEvent &event)
{
	Refresh(false);
	m_projectM->projectM_resetGL(event.GetSize().x, event.GetSize().y);
}

void MilkDropVisualizer::updatePCM(short *data)
{
	m_projectM->pcm()->addPCM16Data(data, 512);
	Refresh(false);
}