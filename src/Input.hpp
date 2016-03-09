//
// Created by Austin on 6/26/2015.
//

#ifndef GAMEBOYEMULATOR_INPUT_HPP
#define GAMEBOYEMULATOR_INPUT_HPP

// SFML
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <array>

namespace serq {
	class SerializeQueue;
}

enum class KeyType {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    A,
    B,
    START,
    SELECT,
    NONE
};

enum class PlayerDialogue {
    SELECT_BATTLE_OR_TRADE,
    SELECTED_BATTLE,
    SELECTED_TRADE,
    REQUESTED_BATTLE,
    REQUESTED_TRADE,
    SELECTED_BATTLE_RESPONSE,
    SELECTED_TRADE_RESPONSE,
    REFUSED_BATTLE_RESPONSE,
    REFUSED_TRADE_RESPONSE,
    NOT_IN_DIALOGUE,
    NONE
};

class MemoryManagementUnit;
class Display;
class Timer;
class Processor;
class GameBoy;
class Network;

/**
 * Handles input for game (joypad).
 */
class Input {
public:
    Input();

    void Initialize(MemoryManagementUnit* mmu_, Display* display_, 
				    Timer* timer_, Processor* cpu_,  GameBoy* gameboy_, Network* network_, sf::RenderWindow* window_);
    void Reset();
    uint8_t ReadByte();
    void WriteByte(uint8_t value);

    bool PollEvents();
    void UpdateInput();
    void KeyUp(KeyType key);
    void KeyDown(KeyType key);
	
	void SaveGameState(int save_slot);
	void LoadGameState(int save_slot);
    
    bool initiateBattleFlag;
    
    // Dialogue with player
    PlayerDialogue dialogueWithPlayer;
    int talkingWithPlayer;
    int currentSelection; // Selection in current menu, 0 = first, 1 = second, etc
    bool ignoreA;

private:
    MemoryManagementUnit* mmu;
	Display* display;
	Timer* timer;
	Processor* cpu;
	GameBoy* gameboy;
    Network* network;
    sf::RenderWindow* window;

    std::array<uint8_t, 2> rows;
    uint8_t column;
	
	int current_save_slot;
    
    std::array<uint8_t, 0x194> pokemonParty;
	
	// Helper Function
	void SaveCharVector(serq::SerializeQueue& save_data, std::vector<uint8_t>& data, std::size_t length);
	void LoadCharVector(serq::SerializeQueue& save_data, std::vector<uint8_t>& data, std::size_t length);
};

#endif //GAMEBOYEMULATOR_INPUT_HPP
