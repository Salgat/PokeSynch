//
// Created by Austin on 6/11/2015.
//

#ifndef GAMEBOYEMULATOR_GAMEBOY_HPP
#define GAMEBOYEMULATOR_GAMEBOY_HPP

// SFML
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <Utility>
#include <array>

#include "Processor.hpp"
#include "MemoryManagementUnit.hpp"
#include "Display.hpp"
#include "Timer.hpp"
#include "Input.hpp"
#include "Network.hpp"

/**
 * Holds meta data related to the sprites.
 */
struct LocalSpriteState {
    int frame;
    int animationCounter;
};

/**
 * Defines a pokemon (party_struct).
 */
struct Pokemon {
    // box_struct
    uint8_t species;
    uint16_t hp;
    uint8_t boxLevel;
    uint8_t status;
    uint8_t type1;
    uint8_t type2;
    uint8_t catchRate;
    std::array<uint8_t, 4> moves;
    uint16_t originalTrainerId;
    std::array<uint8_t, 3> experience;
    uint16_t hpExp;
    uint16_t attackExp;
    uint16_t defenseExp;
    uint16_t speedExp;
    uint16_t specialExp;
    std::array<uint8_t, 2> dv;
    std::array<uint8_t, 4> pp;
    
    // party_struct
    uint8_t level;
    uint16_t maxHP;
    uint16_t attack;
    uint16_t defense;
    uint16_t speed;
    uint16_t special;
};

/**
 * Emulates a GameBoy, outputting visuals to an sf::Image (160x144 pixels)
 */
class GameBoy {
public:
	int screen_size; // Multiplier
	int game_speed; // Multiplier

    GameBoy(sf::RenderWindow& window, std::string name = "", unsigned short port = 34231, std::string ipAddress = "", unsigned short hostPort = 34232);

    void Reset();
    std::pair<sf::Image, bool> RenderFrame();
    void LoadGame(std::string rom_name);

//private:
    sf::Image frame;
    bool initiateBattleFlag;
    bool synchronizedMap;

    Processor cpu;
    MemoryManagementUnit mmu;
    Display display;
    Timer timer;
    Input input;
    Network network;

    NetworkGameState CreateGameState();
    void UpdateLocalGameState(const HostGameState& hostGameState, bool isHost);
    
    bool TileCollisionInFront(const HostGameState& hostGameState);
    bool SpriteCollisionInFront(const HostGameState& hostGameState);
    
    void InitiateBattle();
    void DebugPrint();
};

#endif //GAMEBOYEMULATOR_GAMEBOY_HPP
