#include "Network.hpp"
#include "Input.hpp"
#include "MemoryManagementUnit.hpp"
#include "Display.hpp"
#include "Processor.hpp"
#include "Timer.hpp"
#include "GameBoy.hpp"

/**
 * Serializes a NetworkGameState.
 */
sf::Packet& operator <<(sf::Packet& packet, const NetworkGameState& networkGameState) {
    packet << networkGameState.uniqueId << networkGameState.name << networkGameState.currentMap;
    
    auto& playerPosition = networkGameState.playerPosition;
    packet << playerPosition.yPosition << playerPosition.xPosition 
           << playerPosition.yBlockPosition << playerPosition.xBlockPosition;

    for (auto& sprite : networkGameState.sprites) {
        packet << sprite.spriteIndex << sprite.pictureId << sprite.moveStatus << sprite.direction
               << sprite.yDisplacement << sprite.xDisplacement << sprite.yPosition << sprite.xPosition
               << sprite.canMove << sprite.inGrass;    
    }
    
    return packet;
}

/**
 * Deserializes a NetworkGameState.
 */
sf::Packet& operator >>(sf::Packet& packet, NetworkGameState& networkGameState) {
    packet >> networkGameState.uniqueId >> networkGameState.name >> networkGameState.currentMap;
    
    auto& playerPosition = networkGameState.playerPosition;
    packet >> playerPosition.yPosition >> playerPosition.xPosition 
           >> playerPosition.yBlockPosition >> playerPosition.xBlockPosition;

    for (auto& sprite : networkGameState.sprites) {
        packet >> sprite.spriteIndex >> sprite.pictureId >> sprite.moveStatus >> sprite.direction
               >> sprite.yDisplacement >> sprite.xDisplacement >> sprite.yPosition >> sprite.xPosition
               >> sprite.canMove >> sprite.inGrass;    
    }
    
    return packet;
}

Network::Network() {
}

void Network::Initialize(MemoryManagementUnit* mmu_, Display* display_, 
				       Timer* timer_, Processor* cpu_, GameBoy* gameboy_, sf::RenderWindow* window_) {
    mmu = mmu_;
	display = display_;
	timer = timer_;
	cpu = cpu_;
    window = window_;
}

bool Network::Host(unsigned short port) {
    return false;
}

bool Network::Connect(sf::IpAddress address, unsigned short port) {
    return false;
}