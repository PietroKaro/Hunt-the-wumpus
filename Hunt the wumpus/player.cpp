#include "player.h"

Player::Player(const std::string n) : name{ n }, current_room{ nullptr },
	ammo{ 0 }, max_ammo_cross_room{ 3 } {}

void Player::setRoomPlayerIsIn(Room* room)
{
	if (room)
		room->setPlayerInside(current_room);
	current_room = room;
}

bool Player::move(int id)
{
	if (!current_room)
		throw std::logic_error{ "Player's current room is null" };
	Room* r = current_room->getLinkedRoomByID(id);
	if (!r)
		return false;
	setRoomPlayerIsIn(r);
	return true;
}

Room* Player::shoot(const std::vector<int>& roomIDSCross)
{
	size_t size = roomIDSCross.size();
	if (!current_room || !size || size > max_ammo_cross_room)
		throw std::logic_error{ "A game error has occurred while shooting" };
	Room* r = current_room;
	for (const int& id : roomIDSCross)
	{
		r = r->getLinkedRoomByID(id);
		if (!r)
			break;
	}
	--ammo;
	return r;
}