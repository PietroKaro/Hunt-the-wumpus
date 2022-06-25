#include "room.h"

std::default_random_engine Room::rnd_eng;
std::vector<int> Room::used_ids;

Room::Room() : is_empty{ true }, has_wumpus{ false }, has_player{ false }, id{ randomID() }, type{ eRoomType::Normal }
{ 
	if (used_ids.size() < 20)
		used_ids.push_back(id);
	else
		used_ids.clear();
}

void Room::setPlayerInside(Room* previousRoom)
{
	if (previousRoom)
		previousRoom->setEmpty();
	is_empty = false;
	has_player = true;
}

void Room::setWumpusInside(Room* previousRoom)
{
	if (previousRoom)
		previousRoom->setEmpty();
	is_empty = false;
	has_wumpus = true;
}

void Room::setEmpty()
{
	has_wumpus = false;
	has_player = false;
	is_empty = true;
}

Room* Room::getLinkedRoomByID(int roomID) const
{
	for (Room* rptr : room_connections)
		if (rptr->getID() == roomID)
			return rptr;
	return nullptr;
}

Room* Room::getLinkedRoomByIndex(unsigned int index) const
{
	if (index < 0 || index >= max_room_conn)
		throw std::out_of_range{ "Invalid room index" };
	return room_connections[index];
}

bool Room::connectRoom(Room* room)
{
	if (!room || room_connections.size() == max_room_conn)
		return false;
	room_connections.push_back(room);
	return true;
}

int Room::randomID()
{
	int result = std::uniform_int_distribution<>{ 10, 999 }(rnd_eng);
	for (int& i : used_ids)
		if (result == i)
		{
			result = randomID();
			break;
		}
	return result;
}