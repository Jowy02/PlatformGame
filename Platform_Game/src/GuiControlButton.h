#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	// Called each loop iteration
	bool Update(float dt);

private:

	bool canClick = true;
	bool drawBasic = false;
	//SLIDER 
	int minValue;
	int maxValue;
	bool isChecked;
	SDL_Rect sliderKnob;

	int scaleX = 12;
	int scaleY = 5;
};

#pragma once