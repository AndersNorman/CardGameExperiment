#include "stdafx.h"
#include "iostream"
#include "string"
#pragma once

class Card {

public:
	int id;
	float xcord;
	float ycord;
	std::string texture;
	int flipped;

	Card(int id, float xcord2, float ycord2, std::string texture, int flipped);


};