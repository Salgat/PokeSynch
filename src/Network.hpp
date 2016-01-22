//
// Created by Austin on 1/21/2016.
//

#ifndef GAMEBOYEMULATOR_NETWORK_HPP
#define GAMEBOYEMULATOR_NETWORK_HPP

// SFML
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>

class MemoryManagementUnit;
class Display;
class Timer;
class Processor;
class GameBoy;

/**
 * Holds information regarding each client/server on the network.
 */
struct NetworkId {
    int uniqueId;
    std::string name;
    sf::IpAddress address;
    unsigned short port;
};

/**
 * Represents player's position in the map.
 */
struct PlayerPosition {
    uint8_t yPosition;
    uint8_t xPosition;
    uint8_t yBlockPosition; // Not 100% sure what this is or if it's needed
    uint8_t xBlockPosition;
};

/**
 * Represents sprite information.
 */
struct SpriteState {
    uint8_t spriteIndex; // 0-16 (0 is the player/client's)
    uint8_t pictureId; // 0xC1X0
    uint8_t moveStatus; // 0xC1X1
    uint8_t direction; // 0xC1X9
    uint8_t yDisplacement; // 0xC2X2
    uint8_t xDisplacement; // 0xC2X3
    uint8_t yPosition; // 0xC2X4
    uint8_t xPosition; // 0xC2X5
    uint8_t canMove; // 0xC2X6
    uint8_t inGrass; // 0xC2X7
};

/**
 * Holds game state passed from the network for a specific client.
 */
struct NetworkGameState {
    int uniqueId; // Populated from NetworkId
    std::string name;
    int currentMap;
    PlayerPosition playerPosition;
    std::vector<SpriteState> sprites;
};

/**
 * All Packets start with PacketType to determine how to deserialize the message.
 */
enum class PacketType {
    CONNECT_REQUEST = 1,
    CONNECT_RESPONSE = 2,
    NETWORK_GAME_STATE = 3,
    NONE
};

/**
 * Indicates what mode the network is in.
 */
enum class NetworkMode {
    CONNECTING,
    FAILED_CONNECTING,
    IDLE,
    CONNECTED_AS_CLIENT,
    CONNECTED_AS_HOST,
    NONE
};

/**
 * Used to request a connection to host.
 */
struct ConnectRequest {
    std::string name;
};

/**
 * Used as a response to a connection request as the host.
 */
struct ConnectResponse {
    int uniqueId; // Assigned from server
};

/**
 * Handles server and client communication.
 */
class Network {
public:
    Network();

    void Initialize(MemoryManagementUnit* mmu_, Display* display_, 
				    Timer* timer_, Processor* cpu_,  GameBoy* gameboy_, sf::RenderWindow* window_);
                    
    bool Host(unsigned short port);
    bool Connect(sf::IpAddress address, unsigned short port, std::string name);

    NetworkMode networkMode;
    
private:
    MemoryManagementUnit* mmu;
	Display* display;
	Timer* timer;
	Processor* cpu;
	GameBoy* gameboy;
    sf::RenderWindow* window;
    
    sf::UdpSocket socket;
    int uniqueId;
    std::vector<NetworkId> clients;
    
    bool SetupSocket(unsigned short port);
    NetworkGameState Update();
};

#endif //GAMEBOYEMULATOR_NETWORK_HPP
