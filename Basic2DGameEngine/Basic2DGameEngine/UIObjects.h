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
}

std::vector<UIObject::TextMessage> textMessages;

void CreateMessage(std::string text, float timeToDisplay) {
	UIObject::TextMessage message;
	message.text = text;
	message.timeRemaining = timeToDisplay;

	textMessages.push_back(message);
}