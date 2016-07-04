#include "Spectrum.h"

BEGIN_EVENT_TABLE(Spectrum, wxPanel)
	EVT_PAINT(Spectrum::OnPaint)
END_EVENT_TABLE()


#define BARS 25
#define WIDTH 100
#define BARWIDTH WIDTH / BARS
#define HEIGHT 20

Spectrum::Spectrum(wxWindow *parent) : wxWindow(parent, wxID_ANY)
{
	SetMinSize(wxSize(WIDTH, HEIGHT));
	SetMaxSize(wxSize(-1, HEIGHT));

	config = kiss_fftr_alloc(SAMPLES, 0, nullptr, nullptr);
	spectrum = new kiss_fft_cpx[SAMPLES];
	for (int i = 0; i < SAMPLES; i++) {
		outSpectrum[i] = 0;
	}

}


Spectrum::~Spectrum()
{
	free(config);
	delete[] spectrum;
}

void Spectrum::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	render(dc);
}

void Spectrum::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}


float Spectrum::avg(int pos, int nb)
{
	float sum = 0;
	for (int i = 0; i < nb; i++) {
		sum += outSpectrum[pos + i];
	}

	return (float)(sum / nb);

}

void Spectrum::render(wxDC &dc)
{

	dc.SetBrush(*wxLIGHT_GREY_BRUSH);

	dc.DrawRectangle(0, 0, WIDTH, HEIGHT);
	dc.SetBrush(*wxBLACK_BRUSH);

	const int multi = (SAMPLES / 2) / BARS;

	for (int i = 0, a = 0; i < BARS; a+=multi, i++) {

		float sample = 0.0f;
		for (int j = 0; j < multi; j++) {
			sample += outSpectrum[a + j];
		}

		//float sample = avg(i, (SAMPLES / BARS));
		sample = sample * logf(i + 2);
		if (sample > 1.0f)
			sample = 1.0f;
		
		dc.DrawRectangle(i*BARWIDTH, HEIGHT - (sample * 20.0f), BARWIDTH, HEIGHT);
	}
}

static inline float scale(kiss_fft_scalar val)
{
	if (val < 0) {
		return val * (1 / 32768.0f);
	}
	else {
		return val * (1 / 32767.0f);
	}
}

void Spectrum::calc(short *insamples)
{
	kiss_fftr(config, (kiss_fft_scalar*)insamples, spectrum);

	const int len = SAMPLES / 2 + 1;

	for (int i = 0; i < len; i++) {
		float re = scale(spectrum[i].r) * SAMPLES;
		float im = scale(spectrum[i].i) * SAMPLES;

		if (i > 0) {
			outSpectrum[i] = sqrtf(re*re + im*im) / (SAMPLES / 2);
		}
		else {
			outSpectrum[i] = sqrtf(re*re + im*im) / (SAMPLES);
		}
	}

	paintNow();
}

void Spectrum::clear()
{
	for (int i = 0; i < SAMPLES; i++) {
		outSpectrum[i] = 0.0f;
	}

	paintNow();
}