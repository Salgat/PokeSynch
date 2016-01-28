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

    Processor cpu;
    MemoryManagementUnit mmu;
    Display display;
    Timer timer;
    Input input;
    Network network;

    NetworkGameState CreateGameState();
    void UpdateLocalGameState(const HostGameState& hostGameState, bool isHost);
    
    unsigned int movementCounter;
    int playerOffset;
    int oldPositionX;
    int oldPositionY;
};

#endif //GAMEBOYEMULATOR_GAMEBOY_HPP
