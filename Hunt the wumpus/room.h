#ifndef GAME_ROOM
#define GAME_ROOM

#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <random>

enum class eRoomType
{
	Normal,
	Bat,
	BlackHole,
};

class Room
{
	static std::vector<int> used_ids;
	bool is_empty, has_wumpus, has_player;
	int id;
	eRoomType type;
	std::vector<Room*> room_connections;
	
	static int randomID();

public:
	static const int max_room_conn = 3;
	static std::default_random_engine rnd_eng;

	Room();

	void setPlayerInside(Room* previousRoom);
	void setWumpusInside(Room* previousRoom);
	void setEmpty();
	void setRoomType(eRoomType t) { type = t; }
	bool isEmpty() const { return is_empty; }
	bool hasPlayer() const { return has_player; }
	bool hasWumpus() const { return has_wumpus; }
	eRoomType getRoomType() const { return type; }
	Room* getLinkedRoomByID(int roomID) const;
	Room* getLinkedRoomByIndex(unsigned int index) const;
	int getID() const { return id; }
	bool connectRoom(Room* room);
};

#endif // !GAME_ROOM
