#ifndef GAME_PLAYER
#define GAME_PLAYER

#include <string>
#include <stdexcept>
#include <vector>
#include "room.h"

class Player
{
	std::string name;
	Room* current_room;
	unsigned int ammo, max_ammo_cross_room;

public:
	Player(const std::string n = "Player");

	std::string getName() const { return name; }
	unsigned int getAmmoCount() const { return ammo; }
	Room* getRoomPlayerIsIn() const { return current_room; }
	Room* shoot(const std::vector<int>& roomIDSCross);
	void setRoomPlayerIsIn(Room* room);
	void setAmmoCount(unsigned int count) { ammo = count; }
	bool move(int id);
};

#endif // !GAME_PLAYER

