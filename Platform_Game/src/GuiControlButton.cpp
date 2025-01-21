#include "GuiControlButton.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"

GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text) : GuiControl(type, id)
{
	this->bounds = bounds;
	this->text = text;

	this->minValue = 0;
	this->maxValue = 100;
	this->value = 100;

	// Tamaño del control deslizante (knob)
	sliderKnob = { bounds.x, bounds.y, 20, bounds.h };

	canClick = true;
	drawBasic = false;
	isChecked = false;

	state = GuiControlState::DISABLED;
}

GuiControlButton::~GuiControlButton()
{

}


bool GuiControlButton::Update(float dt)
{
	if (GuiControl::type == GuiControlType::BUTTON) {

		if (state != GuiControlState::DISABLED)
		{
			// L16: TODO 3: Update the state of the GUiButton according to the mouse position
			Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();
		
			//If the position of the mouse if inside the bounds of the button 
			if (mousePos.getX()*2 > bounds.x && mousePos.getX() * 2 < bounds.x + bounds.w && mousePos.getY() * 2 > bounds.y && mousePos.getY() * 2 < bounds.y + bounds.h && state != GuiControlState::JUST_VISIBLE) {
		
				state = GuiControlState::FOCUSED;

				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
					state = GuiControlState::PRESSED;
				}
			
				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
					NotifyObserver();
				}
			}
			else if( state != GuiControlState::JUST_VISIBLE){
				state = GuiControlState::NORMAL;
			}

			//L16: TODO 4: Draw the button according the GuiControl State
			int scaleText = text.length();
			switch (state)
			{
			case GuiControlState::DISABLED:
				Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 255, true, false);
				break;
			case GuiControlState::JUST_VISIBLE:
				Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, scaleText * 10, bounds.h);
				Engine::GetInstance().render->DrawRectangle(bounds, 138,149, 151, 255, true, false);
				break;
			case GuiControlState::NORMAL:
				Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 255, 255, true, false);
				break;
			case GuiControlState::FOCUSED:
				Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 20, 255, true, false);
				break;
			case GuiControlState::PRESSED:
				Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 0, 255, true, false);
				break;
			}
			Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, scaleText*10, bounds.h);
		}
	}
	else if (GuiControl::type == GuiControlType::SLIDER)
	{
		if (state != GuiControlState::DISABLED)
		{
			Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

			// Revisar si el mouse está sobre el slider
			if (mousePos.getX() * 2 > bounds.x && mousePos.getX() * 2 < bounds.x + bounds.w &&
				mousePos.getY() * 2 > bounds.y && mousePos.getY() * 2 < bounds.y + bounds.h)
			{
				state = GuiControlState::FOCUSED;

				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
				{
					state = GuiControlState::PRESSED;

					// Ajustar la posición del slider knob al movimiento del mouse
					int mouseX = mousePos.getX() * 2;
					sliderKnob.x = std::max(bounds.x, std::min(mouseX - sliderKnob.w / 2, bounds.x + bounds.w - sliderKnob.w));

					// Calcular el valor del slider basado en la posición del knob
					float percentage = (float)(sliderKnob.x - bounds.x) / (bounds.w - sliderKnob.w);
					value = minValue + (int)(percentage * (maxValue - minValue));

					NotifyObserver();
				}
			}
			else
			{
				state = GuiControlState::NORMAL;
			}

			// Dibujar el slider según el estado
			switch (state)
			{
			case GuiControlState::DISABLED:
				Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 255, true, false);
				break;
			case GuiControlState::NORMAL:
				Engine::GetInstance().render->DrawRectangle(bounds, 100, 100, 100, 255, true, false);
				break;
			case GuiControlState::FOCUSED:
				Engine::GetInstance().render->DrawRectangle(bounds, 150, 150, 150, 255, true, false);
				break;
			case GuiControlState::PRESSED:
				Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 0, 255, true, false);
				break;
			}

			// Dibujar el slider knob
			Engine::GetInstance().render->DrawRectangle(sliderKnob, 0, 0, 255, 255, true, false);

			// Dibujar el texto del slider y el valor actual
			int scaleText = text.length();
			Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y - 20,scaleText*10, 20);
			scaleText = 1;
			if (value > 10)scaleText = 2;
			if (value == 100)scaleText = 3;
			Engine::GetInstance().render->DrawText(std::to_string(value).c_str(), bounds.x + bounds.w + 10, bounds.y, scaleText*10, bounds.h);
		}
	}
	else if (GuiControl::type == GuiControlType::CHECKBOX) {
		if (state != GuiControlState::DISABLED)
		{
			Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

			// Verificar si el mouse está sobre el checkbox
			if (mousePos.getX() * 2 > bounds.x && mousePos.getX() * 2 < bounds.x + bounds.w &&
				mousePos.getY() * 2 > bounds.y && mousePos.getY() * 2 < bounds.y + bounds.h)
			{
				state = GuiControlState::FOCUSED;

				if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
				{
					isChecked = !isChecked; // Cambiar el estado del checkbox
					NotifyObserver();      // Notificar cambios si es necesario
				}
			}
			else
			{
				state = GuiControlState::NORMAL;
			}

			// Dibujar el checkbox según el estado
			switch (state)
			{
			case GuiControlState::DISABLED:
				Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 255, true, false);
				break;
			case GuiControlState::NORMAL:
				Engine::GetInstance().render->DrawRectangle(bounds, 150, 150, 150, 255, true, false);
				break;
			case GuiControlState::FOCUSED:
				Engine::GetInstance().render->DrawRectangle(bounds, 100, 100, 255, 255, true, false);
				break;
			}
		

		// Dibujar el contenido del checkbox
		// Dibujar un rectángulo dentro si está seleccionado
		if (isChecked)
		{
			SDL_Rect innerBox = { bounds.x + 4, bounds.y + 4, bounds.w - 8, bounds.h - 8 };
			Engine::GetInstance().render->DrawRectangle(innerBox, 0, 255, 0, 255, true, false); // Color verde
		}

		// Dibujar el texto junto al checkbox
		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x + bounds.w + 10, bounds.y, 100, bounds.h);
		}

	}

	return false;
}

