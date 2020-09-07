#pragma once
#include <iostream>
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
		Button(int x, int y, float width, float height, std::string text = "");
		~Button();

		std::string text;

		float width;
		float height;

		int xPos, yPos;

		bool MouseOver(int mouseX, int mouseY) {
			bool withinX = mouseX > xPos && mouseX < xPos + width;
			bool withinY = mouseY > yPos && mouseY < yPos + height;

			return withinX && withinY;
		}
	};

	Button::Button(int x, int y, float width, float height, std::string text)
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