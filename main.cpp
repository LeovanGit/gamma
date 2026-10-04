#define NOMINMAX // to avoid conflicts with cmath max
#include <windows.h>

#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>
#include <functional>

namespace
{
	float delta = 0.1f;
	float minGamma = 0.3f;
	float maxGamma = 4.4f;
}

void CreateGammaRamp(float gamma, uint16_t* ramp)
{
	gamma = std::max(gamma, 0.0001f); // to avoid division by 0

	// for [0; 255] colors in 1 byte (1 channel):
	for (int i = 0; i != 256; ++i)
	{
		// normalize to [0.0f; 1.0f] range for our transformations:
		float value = float(i) / 255.0f;

		// gamma-correction:
		value = pow(value, 1.0f / gamma);

		// convert to [0.0f; 65535.0f] range (SetDeviceGammaRamp() requirement):
		ramp[i] = static_cast<uint16_t>(value * 65535.0f);
	}
}

class Gamma
{
public:
	Gamma()
	{
		m_screenDC = GetDC(NULL); // entire screen
		assert(m_screenDC && "m_screenDC is nullptr\n");

		CreateGammaRamp(m_defaultGamma, m_defaultGammaRamp[0]); // R
		CreateGammaRamp(m_defaultGamma, m_defaultGammaRamp[1]); // G
		CreateGammaRamp(m_defaultGamma, m_defaultGammaRamp[2]); // B

		CreateGammaRamp(m_gamma, m_gammaRamp[0]);
		CreateGammaRamp(m_gamma, m_gammaRamp[1]);
		CreateGammaRamp(m_gamma, m_gammaRamp[2]);

		// Yellow green:
		// CreateGammaRamp(2.64f, m_gammaRamp[0]);
		// CreateGammaRamp(3.0f, m_gammaRamp[1]);
		// CreateGammaRamp(1.44f, m_gammaRamp[2]);
		
		// Green
		// CreateGammaRamp(1.0f, m_gammaRamp[0]);
		// CreateGammaRamp(3.0f, m_gammaRamp[1]);
		// CreateGammaRamp(1.0f, m_gammaRamp[2]);
	}

	~Gamma()
	{
		ReleaseDC(NULL, m_screenDC);
	}

	void ToggleGamma()
	{
		bool result = m_isGammaEnabled ?
			SetDeviceGammaRamp(m_screenDC, m_defaultGammaRamp) :
			SetDeviceGammaRamp(m_screenDC, m_gammaRamp);

		if (!result) std::cerr << "SDL_SetWindowGammaRamp() is not supported!";

		m_isGammaEnabled = !m_isGammaEnabled;
	}

	float GetGamma() const
	{
		return m_gamma;
	}

	void SetGamma(float gamma)
	{
		m_gamma = std::min(maxGamma, std::max(minGamma, gamma));

		std::cout << "New gamma: " << m_gamma << "\n";

		CreateGammaRamp(m_gamma, m_gammaRamp[0]);
		CreateGammaRamp(m_gamma, m_gammaRamp[1]);
		CreateGammaRamp(m_gamma, m_gammaRamp[2]);

		if (m_isGammaEnabled)
		{
			ToggleGamma();
			ToggleGamma();
		}
	}

private:
	HDC m_screenDC = nullptr;

	bool m_isGammaEnabled = false;

	float m_defaultGamma = 1.0f;
	float m_gamma = 3.0f;

	uint16_t m_defaultGammaRamp[3][256];
	uint16_t m_gammaRamp[3][256];
};

struct HotKey
{
	HotKey(
		int key_,
		std::function<void()> action_)
		: key(key_),
		wasPressed(false),
		action(action_)
	{

	}

	int key;
	// To avoid repeated activations while finger still on the button:
	bool wasPressed;
	std::function<void()> action;
};

int main()
{
	Gamma gamma;

	std::vector<HotKey> keys;
	keys.emplace_back(
		'Z',
		[&gamma]()
		{
			gamma.ToggleGamma();
		});

	keys.emplace_back(
		VK_OEM_MINUS,
		[&gamma]()
		{
			float value = gamma.GetGamma();
			gamma.SetGamma(value - delta);
		});

	keys.emplace_back(
		VK_OEM_PLUS,
		[&gamma]()
		{
			float value = gamma.GetGamma();
			gamma.SetGamma(value + delta);
		});
	
	while (true)
	{
		for (auto& key : keys)
		{
			bool isCurrentlyPressed = (GetAsyncKeyState(key.key) & 0x8000);

			if (isCurrentlyPressed && !key.wasPressed)
			{
				key.action();
			}

			key.wasPressed = isCurrentlyPressed;
		}

		Sleep(100); // To avoid CPU 100% load
	}

	return 0;
}
