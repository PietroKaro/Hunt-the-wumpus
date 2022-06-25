#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <limits>
#include <tuple>
#include <regex>
#include <sstream>
#include "room.h"
#include "player.h"

using namespace std;

const int KROOM_AMOUNT = 20;
const int KPLAYER_LOST = 0;
const int KPLAYER_WIN = 1;
const char KACTION_SHOOT = 'S';
const char KACTION_MOVE = 'M';

void instructions()
{
	string instructions = "Your goal is to kill a monster called \"Wumpus\" in a map composed by rooms;\n"
		"Each room is linked to three other rooms but some of these are dangerous;\n"
		"There are rooms with a giant bat that take you in another random room and rooms with a bottomless pit;\n"
		"If you move in the wumpus room, if you fall in a bottomless pit or if you have 0 ammo, you lose the game;\n"
		"You have a weapon with 3 ammo. You can shoot in an adjacent room or through two or three linked rooms;\n"
		"So, when you are in a room, you can choose two actions: move or shoot;\n"
		"Examples: move to room 100: \"M 100\", shoot at room 200: \"S 200\", shoot through three rooms: \"S 300 400 500\".\n\n";
	cout << "************* HUNT THE WUMPUS *************\n\n" << instructions;
}

template<typename T>
T prompt(const string& text, const string& error)
{
	T val{};
	cout << text;
	while (!(cin >> val))
	{
		if (cin.bad())
			throw ios_base::failure{ "Fatal io stream error" };
		else if (cin.fail())
		{
			cout << error << text;
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
	}
	return val;
}

Room* randRoom(vector<Room>& map, bool ignoreNonEmptyRoom)
{
	Room* pr = nullptr;
	do
	{
		int index = uniform_int_distribution<>{ 0, KROOM_AMOUNT - 1 }(Room::rnd_eng);
		pr = &map[index];
	} while ((!ignoreNonEmptyRoom && !pr->isEmpty()) || pr->getRoomType() != eRoomType::Normal);
	return pr;
}

// If !player, func will spawn wumpus
Room* randEntityRoom(vector<Room>& map, Player* player, Room* previousRoom, bool ignoreNonEmptyRoom)
{
	Room* rr = randRoom(map, ignoreNonEmptyRoom);
	if (player)
		player->setRoomPlayerIsIn(rr);
	else
		rr->setWumpusInside(previousRoom);
	return rr;
}

void randDangerousRoom(vector<Room>& map, bool bat, bool hole, unsigned limit)
{
	Room* rr = nullptr;
	for (unsigned i = 0; i < limit; ++i)
	{
		if (bat)
		{
			rr = randRoom(map, false);
			rr->setRoomType(eRoomType::Bat);
		}
		if (hole)
		{
			rr = randRoom(map, false);
			rr->setRoomType(eRoomType::BlackHole);
		}
	}
}

bool linkRoom(vector<Room>& resultMap)
{
	ifstream ifs{ "room_map.txt" };
	if (!ifs)
		return false;
	for (Room& r : resultMap)
	{
		char c;
		ifs >> c;
		if (!ifs || c != '{')
			return false;
		for (int i = 0; i < Room::max_room_conn; ++i)
		{
			int room_index;
			ifs >> room_index;
			if (!ifs || room_index < 0 || room_index >= KROOM_AMOUNT || !r.connectRoom(&resultMap[room_index]))
				return false;
		}
		ifs >> c;
		if (!ifs || c != '}')
			return false;
	}
	return true;
}

// 0 Wumpus, 1 Bat, 2 Hole, 3 text to show, 4 string room id
tuple<bool, bool, bool, string, string> getRoomInfo(const Room& room, bool nearRooms)
{
	int count = nearRooms ? 3 : 1;
	auto result = make_tuple<bool, bool, bool, string, string>(false, false, false, "", "");
	for (int i = 0; i < count; ++i)
	{
		const Room& r = nearRooms ? *room.getLinkedRoomByIndex(i) : room;
		string s_id = to_string(r.getID());
		get<4>(result) += s_id + ' ';
		if (r.hasWumpus() && !get<0>(result))
		{
			get<3>(result) += nearRooms ? "I smell the wumpus...\n" : "You are in room " + s_id + " with Wumpus...\nYOU LOST!\n";
			get<0>(result) = true;
		}
		else if ((r.getRoomType() == eRoomType::Bat) && !get<1>(result))
		{
			get<3>(result) += nearRooms ? "I hear a bat...\n" : "You walked into a bat room (" + s_id + "). Bat will take you to another room...\n";
			get<1>(result) = true;
		}
		else if ((r.getRoomType() == eRoomType::BlackHole) && !get<2>(result))
		{
			get<3>(result) += nearRooms ? "I feel a breeze...\n" : "You fell in a bottomless pit, in room " + s_id + "...\nYOU LOST!\n";
			get<2>(result) = true;
		}
	}
	return result;
}

// 0 action type, 1 room id
tuple<char, vector<int>> getPlayerAction()
{
	auto result_action = make_tuple<char, vector<int>>(0, {});
	regex pat_action{ R"(((s|S)\s\d{2,}\s\d{2,}\s\d{2,}$)|((s|S)\s\d{2,}\s\d{2,}$)|((m|s|M|S)\s\d{2,}$))" };
	cout << "Choose an action: ";
	for (string action; getline(cin, action);)
	{
		smatch matches;
		if (regex_search(action, matches, pat_action))
		{
			istringstream iss{ action };
			iss >> get<0>(result_action);
			for (int i; iss >> i; get<1>(result_action).push_back(i));
			break;
		}
		else
			cout << "Invalid input!\n";
	}
	if (cin.bad())
		throw ios_base::failure{ "Fatal io stream error" };
	return result_action;
}

bool gameLoop(Player& player, vector<Room>& map, Room* wumpus_room)
{
	while (true)
	{
		Room& player_r = *player.getRoomPlayerIsIn();
		auto data = getRoomInfo(player_r, false);
		cout << get<3>(data);
		if (get<0>(data) || get<2>(data))
			return KPLAYER_LOST;
		else if (get<1>(data))
		{
			randEntityRoom(map, &player, nullptr, true);
			continue;
		}
		cout << "You are in room " << player_r.getID() << '.';
		data = getRoomInfo(player_r, true);
		cout << " Adjacent rooms are " << get<4>(data) << '\n';
		cout << get<3>(data);
		auto action = getPlayerAction();
		switch (toupper(get<0>(action)))
		{
		case KACTION_MOVE:
			if (!player.move(get<1>(action)[0]))
				cout << "This room is not adjacent with yours!\n";
			break;
		case KACTION_SHOOT:
			Room* hit_room = player.shoot(get<1>(action));
			if (hit_room && hit_room->hasWumpus())
			{
				cout << "You killed the wumpus! YOU WIN!\n";
				return KPLAYER_WIN;
			}
			cout << "The wumpus was not in that room...\n";
			if (!player.getAmmoCount())
			{
				cout << "Out of ammo!\nYOU LOST!\n";
				return KPLAYER_LOST;
			}
			wumpus_room = randEntityRoom(map, nullptr, wumpus_room, true);
			break;
		}
	}
}

int main()
try
{
	instructions();
	string player_name = prompt<string>("Enter player name: ", "Invalid player name.\n");
	Player py{ player_name };
	unsigned int seed = prompt<unsigned int>("Enter seed: ", "Invalid seed.\n");
	Room::rnd_eng.seed(seed);
	string game_running;
	int won = 0, lost = 0;
	while (game_running != "EXIT")
	{
		py.setAmmoCount(3);
		vector<Room> room_map(KROOM_AMOUNT);
		randEntityRoom(room_map, &py, nullptr, false);
		Room* wr{ randEntityRoom(room_map, nullptr, nullptr, false) };
		randDangerousRoom(room_map, true, true, 2);
		if (!linkRoom(room_map))
			throw ios_base::failure{ "Error reading map file" };
		cout << "\n\n";
		cin.ignore(1);
		if (gameLoop(py, room_map, wr))
			won++;
		else
			lost++;
		cout << py.getName() << ":\n" << "Won: " << won << "\tLost: " << lost << '\n';
		game_running = prompt<string>("Enter any key to continue or \"exit\" to close the game: ", "Invalid answer.");
		for (char& c : game_running)
			c = toupper(c);
		wr = nullptr;
		py.setRoomPlayerIsIn(nullptr);
	}
	return 0;
}
catch (ios_base::failure ioex)
{
	cerr << '\n' << ioex.what() << "\nPress any key to close the game...";
	cin.get();
	return -1;
}
catch (logic_error leex)
{
	cerr << '\n' << leex.what() << "\nPress any key to close the game...";
	cin.get();
	return -2;
}
catch (exception ex)
{
	cerr << '\n' << ex.what() << "\nPress any key to close the game...";
	cin.get();
	return 1;
}
catch (...)
{
	cerr << "Fatal exception\nPress any key to close the game...";
	cin.get();
	return 2;
}