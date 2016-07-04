#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

//#define FIXED_POINT 1

#include <cstdint>

#include "kiss_fftr.h"

#define SAMPLES 512

class Spectrum : public wxWindow
{
public:
	Spectrum(wxWindow *parent);
	~Spectrum();

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);

	void paintNow();

	void calc(short *insamples);
	void clear();


private:
	wxWindow *m_pParent;
	void render(wxDC &dc);

	kiss_fftr_cfg config;
	kiss_fft_cpx *spectrum;

	float outSpectrum[SAMPLES];

	float avg(int pos, int nb);


	DECLARE_EVENT_TABLE()

};

