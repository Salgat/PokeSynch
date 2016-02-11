//
// Created by Austin on 6/11/2015.
//

#include "GameBoy.hpp"

#include <iostream>

GameBoy::GameBoy(sf::RenderWindow& window, std::string name, unsigned short port, std::string ipAddress, unsigned short hostPort)
	: screen_size(1)
	, game_speed(1) {
    cpu.Initialize(&mmu);
    mmu.Initialize(&cpu, &input, &display, &timer);
    display.Initialize(&cpu, &mmu);
    timer.Initialize(&cpu, &mmu, &display);
	input.Initialize(&mmu, &display, &timer, &cpu, this, &window);
    network.Initialize(&mmu, &display, &timer, &cpu, this, &window);

	Reset();
    
    // Attempt to connect as either host or client
    // If IP Address provided, connect as client
    if (ipAddress != "") {  
        network.Connect(sf::IpAddress(ipAddress), hostPort, port, name);
    } else {
        network.Host(hostPort, name);
    }
}

void GameBoy::LoadGame(std::string rom_name) {
    mmu.LoadRom(rom_name);
}

void GameBoy::Reset() {
    cpu.Reset();
    mmu.Reset();
    timer.Reset();
    
    synchronizedMap = false;
    initiateBattleFlag = false;
}

// Todo: Frame calling v-blank 195-196x per frame??
std::pair<sf::Image, bool> GameBoy::RenderFrame() {
    // First check for any updates on the network
    NetworkGameState localGameState = CreateGameState();
    auto hostGameState = network.Update(localGameState);
    UpdateLocalGameState(hostGameState, network.isHost);
    
    if (initiateBattleFlag) {
        InitiateBattle();
        initiateBattleFlag = false;
    }
    
    //DebugPrint();
    
    bool running = (input.PollEvents())?true:false;
	cpu.frame_clock = cpu.clock + 17556; // Number of cycles/4 for one frame before v-blank
	bool v_blank = false;
	do {
        if (cpu.halt) {
            cpu.clock += 1;
        } else {
            cpu.ExecuteNextInstruction();
        }

        uint8_t if_memory_value = mmu.ReadByte(0xFF0F);
        if (mmu.interrupt_enable and cpu.interrupt_master_enable and if_memory_value) {
			cpu.halt = 0;
			cpu.interrupt_master_enable = 0;
			uint8_t interrupt_fired = mmu.interrupt_enable & if_memory_value;

            if (interrupt_fired & 0x01) {if_memory_value &= 0XFE; mmu.WriteByte(0xFF0F, if_memory_value); cpu.RST40();}
			else if (interrupt_fired & 0x02) {if_memory_value &= 0XFD; mmu.WriteByte(0xFF0F, if_memory_value); cpu.RST48();}
			else if (interrupt_fired & 0x04) {if_memory_value &= 0XFB; mmu.WriteByte(0xFF0F, if_memory_value); cpu.RST50();}
			else if (interrupt_fired & 0x08) {if_memory_value &= 0XF7; mmu.WriteByte(0xFF0F, if_memory_value); cpu.RST58();}
			else if (interrupt_fired & 0x10) {if_memory_value &= 0XEF; mmu.WriteByte(0xFF0F, if_memory_value); cpu.RST60();}
			else {cpu.interrupt_master_enable = 1;}
			
			mmu.WriteByte(0xFF0F, if_memory_value);
		}
		
		timer.Increment();
	} while(cpu.clock < cpu.frame_clock);
	
	if (!v_blank) {
		display.RenderFrame();
        display.DisplayPlayers(hostGameState, network.uniqueId);
        /*display.DrawWindowWithText("PokeSynch Testing!", 0);
        display.DrawWindowWithText("Line 2 testing.", 1);
        
        display.DrawOptionsWindowWithText("BATTLE", 0, true);
        display.DrawOptionsWindowWithText("TRADE", 1, false);*/
	}
	
	return std::make_pair(display.frame, running);
}

NetworkGameState GameBoy::CreateGameState() {
    NetworkGameState localGameState;
    
    localGameState.currentMap = mmu.wram[0xD35E & 0x1FFF];
    localGameState.walkBikeSurfState = mmu.ReadByte(0xd700);
    
    localGameState.playerPosition.yPosition = mmu.wram[0xD361 & 0x1FFF];
    localGameState.playerPosition.xPosition = mmu.wram[0xD362 & 0x1FFF];
    localGameState.playerPosition.yBlockPosition = mmu.wram[0xD363 & 0x1FFF];
    localGameState.playerPosition.xBlockPosition = mmu.wram[0xD364 & 0x1FFF];
    
    localGameState.sprites.resize(16);
    for (uint16_t index = 0; index < 16; ++index) {
        uint16_t offset = (0xC100 + index*0x10) & 0x1FFF;
        uint16_t offset2 = (0xC200 + index*0x10) & 0x1FFF;
        
        auto& sprite = localGameState.sprites[index];
        sprite.spriteIndex = index;
        sprite.pictureId = mmu.wram[offset + 0x0];
        sprite.moveStatus = mmu.wram[offset + 0x1];
        sprite.direction = mmu.wram[offset + 0x9];
        sprite.yDisplacement = mmu.wram[offset2 + 0x2];
        sprite.xDisplacement = mmu.wram[offset2 + 0x3];
        sprite.yPosition = mmu.wram[offset2 + 0x4];
        sprite.xPosition = mmu.wram[offset2 + 0x5];
        sprite.canMove = mmu.wram[offset2 + 0x6];
        sprite.inGrass = mmu.wram[offset2 + 0x7];
    }
    
    unsigned int offset = 0xd163;
    for (unsigned int index = 0; index < 0x108+8; ++index) {
        localGameState.partyMonsters[index] = mmu.wram[(offset+index) & 0x1FFF];
    }
    
    return localGameState;
}

/**
 * Using the host's memory, synchronizes local sprites with host's sprites if on the same map.
 */
void GameBoy::UpdateLocalGameState(const HostGameState& hostGameState, bool isHost) {
    if (hostGameState.playerGameStates.size() == 0 || hostGameState.sprites.size() < 16) {
        return;
    }
    
    // If on the same map, synchronize sprites
    if (!isHost && hostGameState.playerGameStates[0].currentMap == mmu.wram[0xD35E & 0x1FFF]) {
        auto numberOfSprites = static_cast<uint16_t>(mmu.wram[0xd4e1 & 0x1fff]);
        for (uint16_t index = 1; index < 16; ++index) {
            const auto& sprite = hostGameState.sprites[index];
            uint16_t offset = (0xC100 + index*0x10);
            uint16_t offset2 = (0xC200 + index*0x10);
            
            if (!synchronizedMap || std::abs(static_cast<int>(mmu.ReadByte(offset2 + 0x5)) - 4 - static_cast<int>(mmu.ReadByte(0xd362))) > 4 ||
                                    std::abs(static_cast<int>(mmu.ReadByte(offset2 + 0x4)) - 4 - static_cast<int>(mmu.ReadByte(0xd361))) > 4 ||
                                    std::abs(static_cast<int>(sprite.xPosition) - 4 - static_cast<int>(mmu.ReadByte(0xd362))) > 4 ||
                                    std::abs(static_cast<int>(sprite.yPosition) - 4 - static_cast<int>(mmu.ReadByte(0xd361))) > 4) {
                // The first time a player joins another's map, synchronize all sprites OR if
                // the sprite is outside the vision.
                mmu.WriteByte(offset2 + 0x4, sprite.yPosition);
                mmu.WriteByte(offset2 + 0x5, sprite.xPosition);
            
                synchronizedMap = true;
            } else {
                if (mmu.ReadByte(offset2 + 0x5) != sprite.xPosition ||
                    mmu.ReadByte(offset2 + 0x4) != sprite.yPosition) {
                    // If position differs from ingame, instruct NPC to move in that direction
                    uint8_t direction = 0x0;
                    if (mmu.ReadByte(offset2 + 0x5) > sprite.xPosition) {
                        // Move Left
                        direction = 0xd2;
                    } else if (mmu.ReadByte(offset2 + 0x5) < sprite.xPosition) {
                        // Move Right
                        direction = 0xd3;
                    } else if (mmu.ReadByte(offset2 + 0x4) > sprite.yPosition) {
                        // Move Up
                        direction = 0xd1;
                    } else if (mmu.ReadByte(offset2 + 0x4) < sprite.yPosition) {
                        // Move Down
                        direction = 0xd0;
                    }
                    
                    auto spriteMoving = mmu.ReadByte(offset + 0x1);
                    mmu.WriteByte(0xd4e4+0x2*(index-1), direction);
                    mmu.WriteByte(offset2 + 0x6, 0xfe);
                    if (spriteMoving == 0x2) {
                        mmu.WriteByte(offset+0x1, 0x1);
                    }
                } else {
                    // Instructs NPC not to move on its own
                    mmu.WriteByte(offset + 0x9, sprite.direction);
                    mmu.WriteByte(offset2 + 0x6, 0xff);
                }
            }
        }
    } else {
        synchronizedMap = false;
    }
}

/**
 * Returns true if there is a tile collision in front of the player sprite.
 */
bool GameBoy::TileCollisionInFront(const HostGameState& hostGameState) {
    int tileOnPlayer = static_cast<int>(mmu.wram[0xcf0e&0x1fff]);
    int tileFrontOfPlayer = static_cast<int>(mmu.wram[0xcfc6&0x1fff]);
    bool collision = false;
    int collisionPointer = static_cast<int>(mmu.wram[0xD530&0x1fff]) + (static_cast<int>(mmu.wram[0xD531&0x1fff])<<8);
    int count = 0;
    
    /*
    // TODO: Need to detect if player is in proper map to do this comparison
    if ((tileOnPlayer == 0x20 && tileFrontOfPlayer == 0x05) ||
        (tileOnPlayer == 0x41 && tileFrontOfPlayer == 0x05) ||
        (tileOnPlayer == 0x30 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x2A && tileFrontOfPlayer == 0x05) ||
        (tileOnPlayer == 0x05 && tileFrontOfPlayer == 0x21) ||
        (tileOnPlayer == 0x52 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x55 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x56 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x20 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x5E && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x5F && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x14 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x48 && tileFrontOfPlayer == 0x2E) ||
        (tileOnPlayer == 0x14 && tileFrontOfPlayer == 0x05)) {
            collision = true;
    }*/
    
    // If there is a match to the collision tile list, the tile is not passable
    while (collision == false) {
        int tile = static_cast<int>(mmu.ReadByte(static_cast<uint16_t>(collisionPointer&0xFFFF)));
        ++collisionPointer;
        ++count;
        if (tile == tileFrontOfPlayer) {
            return false;
        }
        if (tile == 0xff) break;
    }
    
    return true;
}

/**
 * Returns true if there is a sprite collision in front of the player sprite.
 */
bool GameBoy::SpriteCollisionInFront(const HostGameState& hostGameState) {
    // TODO: For collision detect with sprites, iterate through the 16 for their positions and see if the player's direction
    //       plus one unit is the position of any of the sprites. If so, prevent the player from moving manually (to prevent
    //       things like walking through other sprites).
    auto positionInFrontX = static_cast<int>(mmu.wram[0xd362 & 0x1FFF]);
    auto positionInFrontY = static_cast<int>(mmu.wram[0xd361 & 0x1FFF]);
    int playerDirection = hostGameState.sprites[0].direction;
    switch (playerDirection) {
        case 0x0:
            // Down
            positionInFrontX += 1;
            break;
        case 0x4:
            // Up
            positionInFrontX -= 1;
            break;
        case 0x8:
            // Left
            positionInFrontY -= 1;
            break;
        case 0xc:
            // Right
            positionInFrontY += 1;
            break;    
    }
    
    for (uint16_t index = 1; index < 16; ++index) {
        const auto& sprite = hostGameState.sprites[index];
        uint16_t offset = (0xC100 + index*0x10) & 0x1FFF;
        uint16_t offset2 = (0xC200 + index*0x10) & 0x1FFF;
        
        if (static_cast<int>(sprite.yPosition)-4 == positionInFrontY &&
            static_cast<int>(sprite.xPosition)-4 == positionInFrontX) {
                return true;
        }
    }
    
    return false;
}

/**
 * Initiates a battle in game with a trainer.
 */
void GameBoy::InitiateBattle() {
    // Set my first pokemon to level 94 and set the appropriate experience
    mmu.WriteByte(0xd16b + 25 + 8, 94);
    mmu.WriteByte(0xd16b + 14 + 0, 0x08);
    
    // wCurOpponent is found at TrainerDataPointers where YoungsterData (the first) is at 201
    // Use https://github.com/pret/pokered/blob/351146024bdd386c328af0f2abdb04e728e4c133/constants/trainer_constants.asm
    // with 200 + number
    // wTrainerNo indicates which index in that wCurOpponent type to choose (starting at 1 for the first)
    
    mmu.WriteByte(0xd057, 0x2); // wIsInBattle
    mmu.WriteByte(0xd05a, 0x0); // wBattleType
    mmu.WriteByte(0xd059, 247); // wCurOpponent
    mmu.WriteByte(0xd713, 247); // wEnemyMonOrTrainerClass
    mmu.WriteByte(0xd05d, 1); // wTrainerNo
}

void GameBoy::DebugPrint() {
    std::cout << "wIsInBattle: " << static_cast<int>(mmu.ReadByte(0xd057)) 
              << "\twBattleType: " << static_cast<int>(mmu.ReadByte(0xd05a))
              << "\twCurOpponent: " << static_cast<int>(mmu.ReadByte(0xd059))
              << "\twEnemMonOrTrainerClass: " << static_cast<int>(mmu.ReadByte(0xd713))
              << "\twTrainerNo: " << static_cast<int>(mmu.ReadByte(0xd05d)) << std::endl;
}