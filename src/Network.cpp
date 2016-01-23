#include "Network.hpp"
#include "Input.hpp"
#include "MemoryManagementUnit.hpp"
#include "Display.hpp"
#include "Processor.hpp"
#include "Timer.hpp"
#include "GameBoy.hpp"

// NOTE: Might want to make the connect logic TCP (gauranteed) and update gamestate logic UDP

/**
 * Serializes a NetworkGameState.
 */
sf::Packet& operator <<(sf::Packet& packet, const NetworkGameState& networkGameState) {
    packet << networkGameState.uniqueId << networkGameState.name << networkGameState.currentMap;
    
    auto& playerPosition = networkGameState.playerPosition;
    packet << playerPosition.yPosition << playerPosition.xPosition 
           << playerPosition.yBlockPosition << playerPosition.xBlockPosition;

    packet << networkGameState.sprites.size();
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

    // TODO: Make sure this part works
    int spriteCount;
    packet >> spriteCount;
    for (unsigned int index = 0; index < spriteCount; ++index) {
        auto& sprite = networkGameState.sprites[index];
        packet >> sprite.spriteIndex >> sprite.pictureId >> sprite.moveStatus >> sprite.direction
               >> sprite.yDisplacement >> sprite.xDisplacement >> sprite.yPosition >> sprite.xPosition
               >> sprite.canMove >> sprite.inGrass;    
    }
    
    return packet;
}

/**
 * Serializes a HostGameState.
 */
sf::Packet& operator <<(sf::Packet& packet, const HostGameState& hostGameState) {
    packet << hostGameState.playerGameStates.size();
    for (auto& playerGameState : hostGameState.playerGameStates) {
        auto& playerPosition = playerGameState.playerPosition;
        packet << playerGameState.uniqueId << playerGameState.name << playerGameState.currentMap
               << playerPosition.yPosition << playerPosition.xPosition 
               << playerPosition.yBlockPosition << playerPosition.xBlockPosition; 
    }

    packet << hostGameState.sprites.size();
    for (auto& sprite : hostGameState.sprites) {
        packet << sprite.spriteIndex << sprite.pictureId << sprite.moveStatus << sprite.direction
               << sprite.yDisplacement << sprite.xDisplacement << sprite.yPosition << sprite.xPosition
               << sprite.canMove << sprite.inGrass;    
    }
    
    return packet;
}

/**
 * Deserializes a HostGameState.
 */
sf::Packet& operator >>(sf::Packet& packet, HostGameState& hostGameState) {
    int playerCount;
    packet >> playerCount;
    for (unsigned int index = 0; index < playerCount; ++index) {
        auto& playerGameState = hostGameState.playerGameStates[index];
        auto& playerPosition = playerGameState.playerPosition;
        packet >> playerGameState.uniqueId >> playerGameState.name >> playerGameState.currentMap
               >> playerPosition.yPosition >> playerPosition.xPosition 
               >> playerPosition.yBlockPosition >> playerPosition.xBlockPosition; 
    }

    int spriteCount;
    packet >> spriteCount;
    for (unsigned int index = 0; index < spriteCount; ++index) {
        auto& sprite = hostGameState.sprites[index];
        packet >> sprite.spriteIndex >> sprite.pictureId >> sprite.moveStatus >> sprite.direction
               >> sprite.yDisplacement >> sprite.xDisplacement >> sprite.yPosition >> sprite.xPosition
               >> sprite.canMove >> sprite.inGrass;    
    }
    
    return packet;
}

/**
 * Serializes a Connect Request.
 */
sf::Packet& operator <<(sf::Packet& packet, const ConnectRequest& connectRequest) {
    packet << connectRequest.name;
    return packet;
}

/**
 * Deserializes a Connect Request.
 */
sf::Packet& operator >>(sf::Packet& packet, ConnectRequest& connectRequest) {
    packet >> connectRequest.name;
    return packet;
}

/**
 * Serializes a Connect Response.
 */
sf::Packet& operator <<(sf::Packet& packet, const ConnectResponse& connectResponse) {
    packet << connectResponse.uniqueId;
    return packet;
}

/**
 * Deserializes a Connect Response.
 */
sf::Packet& operator >>(sf::Packet& packet, ConnectResponse& connectResponse) {
    packet >> connectResponse.uniqueId;
    return packet;
}

Network::Network() {
    networkMode = NetworkMode::IDLE;
}

void Network::Initialize(MemoryManagementUnit* mmu_, Display* display_, 
				       Timer* timer_, Processor* cpu_, GameBoy* gameboy_, sf::RenderWindow* window_) {
    mmu = mmu_;
	display = display_;
	timer = timer_;
	cpu = cpu_;
    window = window_;
}

/**
 * Prepares socket to asynchronously listen on the specified port.
 */
bool Network::SetupSocket(unsigned short port) {
    if (socket.bind(port) != sf::Socket::Done) {
        return false;
    }
    socket.setBlocking(false);
    
    return true;
}

/**
 * Returns true after successfully acting as host and listening on the specified port.
 */
bool Network::Host(unsigned short port) {
    networkMode = NetworkMode::CONNECTING;
    if (!SetupSocket(port)) {
        networkMode = NetworkMode::FAILED_CONNECTING;
        return false;
    }
    
    networkMode = NetworkMode::CONNECTED_AS_HOST;
    std::srand(std::time(0)); // use current time as seed for random generator
    uniqueId = std::rand();
    return true;
}


/**
 * Returns true after successfully receiving a response from the host.
 */
bool Network::Connect(sf::IpAddress address, unsigned short port, std::string name) {
    networkMode = NetworkMode::CONNECTING;
    if (socket.bind(port) != sf::Socket::Done)
    {
        networkMode = NetworkMode::FAILED_CONNECTING;
        return false;
    }
    
    // Setup request packet
    ConnectRequest connectRequest;
    connectRequest.name = name;
    sf::Packet connectRequestPacket;
    connectRequestPacket << static_cast<int>(PacketType::CONNECT_REQUEST) << connectRequest;
    
    // Send a request to connect to host
    if (socket.send(connectRequestPacket, address, port) != sf::Socket::Done) {
        networkMode = NetworkMode::FAILED_CONNECTING;
        return false;
    }
        
    // Wait for a response from the host
    sf::Packet connectResponsePacket;
    int packetType;
    while (true) {
        if (socket.receive(connectResponsePacket, address, port) != sf::Socket::Done) {
            networkMode = NetworkMode::FAILED_CONNECTING;
            return false;
        }
        connectRequestPacket >> packetType;
        
        // Keep looping through until we receive a Connection Response
        // TODO: Make this asynchronous, otherwise we could be waiting forever on a dropped packet
        if (packetType == static_cast<int>(PacketType::CONNECT_RESPONSE)) break;
    }
    
    connectRequestPacket >> uniqueId;
    networkMode = NetworkMode::CONNECTED_AS_CLIENT;
    socket.setBlocking(false);
    return true;
}

/**
 * Handle processing any responses received and requests made. Returns null if
 * no updates to state are received.
 */
HostGameState Network::Update() {
    if (networkMode == NetworkMode::CONNECTED_AS_HOST) {
        return HostUpdate();
    } else if (networkMode == NetworkMode::CONNECTED_AS_CLIENT) {
        return ClientUpdate();
    } else {
        throw;
    }
}

HostGameState Network::HostUpdate() {
    // Loop through all pending packets
    std::vector<NetworkGameState> gameState;
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;
    auto status = sf::Socket::Done;
    while (status == sf::Socket::Done) {
        auto result = socket.receive(packet, sender, port);
        if (result == sf::Socket::Done) {
            int packetType;
            packet >> packetType;
            if (packetType == static_cast<int>(PacketType::CONNECT_REQUEST)) {
                HandleConnectRequest(packet, sender, port);
            } else if (packetType == static_cast<int>(PacketType::NETWORK_GAME_STATE)) {
                gameState.push_back(HandleGameStateResponse(packet, sender, port));
            }
        } else if (result == sf::Socket::Disconnected) {
            // TODO
        } else if (result == sf::Socket::Error) {
            // Do Nothing
        }
    }
    
    // TODO: Create a HostGameState from all the client networkGameStates
    HostGameState hostGameState;
    
    // TODO: Send the game state out to all clients
    
    return hostGameState;
}

/**
 * For a connect request from the client, adds them to the client list and responds with their uniqueId.
 */
void Network::HandleConnectRequest(sf::Packet requestPacket, sf::IpAddress sender, unsigned short port) {
    // Respond with a uniqueId and add the requester to the client listen
    ConnectResponse response;
    std::srand(std::time(0)); // use current time as seed for random generator
    response.uniqueId = std::rand();
    
    // Create client Id and add to client list
    NetworkId clientId;
    std::string name;
    requestPacket >> name;
    clientId.uniqueId = response.uniqueId;
    clientId.name = name;
    clientId.address = sender;
    clientId.port = port;
    clients[clientId.uniqueId] = clientId;
    
    // Send response to client with uniqueId
    sf::Packet connectResponsePacket;
    connectResponsePacket << static_cast<int>(PacketType::CONNECT_RESPONSE) << response;
    socket.send(connectResponsePacket, sender, port);
}

/**
 * Handle's the client's game state packet and returns the game state they sent here.
 */
NetworkGameState Network::HandleGameStateResponse(sf::Packet gameStatePacket, sf::IpAddress sender, unsigned short port) {
    NetworkGameState clientGameState;
    gameStatePacket >> clientGameState; // TODO: Need to validate this against the IP Address and Port
    return clientGameState;
}

/**
 * Receives 
 */
HostGameState Network::ClientUpdate() {
    // Loop through all pending packets
    HostGameState hostGameState;
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;
    auto status = sf::Socket::Done;
    while (status == sf::Socket::Done) {
        auto result = socket.receive(packet, sender, port);
        if (result == sf::Socket::Done) {
            int packetType;
            packet >> packetType;
            if (packetType == static_cast<int>(PacketType::CONNECT_RESPONSE)) {
                HandleConnectResponse(packet, sender, port);
            } else if (packetType == static_cast<int>(PacketType::HOST_GAME_STATE)) {
                packet >> hostGameState;
            }
        } else if (result == sf::Socket::Disconnected) {
            // TODO
        } else if (result == sf::Socket::Error) {
            // Do Nothing
        }
    }
    
    // TODO: Send the game state to the host
    
    return hostGameState;
}

/**
 * Accepts connect response from host.
 */
void Network::HandleConnectResponse(sf::Packet gameStatePacket, sf::IpAddress sender, unsigned short port) {
    // TODO: Normally this should only be handled during the connect phase, but need to account for this
    // on a re-sent UDP packet for example.
    throw;
}