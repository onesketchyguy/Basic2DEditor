#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <utility>
#include "CoreData.h"

namespace  UIObject {
	class TextMessage
	{
	public:
		TextMessage();
		~TextMessage();

		std::string text;
		float timeRemaining;

	private:
	};

	TextMessage::TextMessage()
	{
	}

	TextMessage::~TextMessage()
	{
	}

	class Button {
	public:
		Button(int x, int y, int width, int height, std::string text = "");
		~Button();

		std::string text;

		int width, height;
		int xPos, yPos;

		olc::Pixel normalColor = olc::BLACK, highlightColor = olc::RED, textColor = olc::YELLOW;

		std::vector<std::function<void()>> onButtonPressed;

		bool MouseOver(int mouseX, int mouseY) {
			bool withinX = mouseX > xPos && mouseX < xPos + width;
			bool withinY = mouseY > yPos && mouseY < yPos + height;

			_mouserOver = withinX && withinY;

			return _mouserOver;
		}

		void Draw(olc::PixelGameEngine* canvas, int offsetY) {
			olc::vi2d pos{ xPos, yPos + offsetY };
			olc::vi2d scale{ width, height };
			canvas->FillRectDecal(pos, scale, (_mouserOver ? highlightColor : normalColor));

			olc::vi2d textPos{ xPos + (width / 4), offsetY + yPos + (height / 2) };
			canvas->DrawStringDecal(textPos, text, textColor, { 0.5f, 0.5f });
		}

		void Activate() {
			// Call each function subscribed to the vector
			for (auto& function : onButtonPressed)
			{
				function();
			}
		}

	private:
		bool _mouserOver;
	};

	Button::Button(int x, int y, int width, int height, std::string text)
	{
		this->width = width;
		this->height = height;
		this->xPos = x;
		this->yPos = y;
		this->text = text;
	}

	Button::~Button()
	{
	}
}

std::vector<UIObject::Button> buttons;

std::vector<UIObject::TextMessage> textMessages;

void CreateMessage(std::string text, float timeToDisplay) {
	UIObject::TextMessage message;
	message.text = text;
	message.timeRemaining = timeToDisplay;

	textMessages.push_back(message);
}