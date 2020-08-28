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