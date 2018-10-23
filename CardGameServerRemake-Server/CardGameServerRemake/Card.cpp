#include "stdafx.h"
#include "Card.h"

Card::Card(int id, float xcord2, float ycord2, std::string texture, int flipped) {
	this->id = id;
	this->xcord = xcord2;
	this->ycord = ycord2;
	this->texture = texture;
	this->flipped = flipped;
}