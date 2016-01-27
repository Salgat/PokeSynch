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
}

// Todo: Frame calling v-blank 195-196x per frame??
std::pair<sf::Image, bool> GameBoy::RenderFrame() {
    // First check for any updates on the network
    NetworkGameState localGameState = CreateGameState();
    auto hostGameState = network.Update(localGameState);
    UpdateLocalGameState(hostGameState, network.isHost);
    
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
		frame = display.RenderFrame();
	}
	
	return std::make_pair(frame, running);
}

NetworkGameState GameBoy::CreateGameState() {
    NetworkGameState localGameState;
    
    localGameState.currentMap = mmu.wram[0xD35E & 0x1FFF];
    
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
        auto selfPositionX = static_cast<int>(mmu.wram[0xd362 & 0x1FFF]);
        auto selfPositionY = static_cast<int>(mmu.wram[0xd361 & 0x1FFF]);

        for (uint16_t index = 1; index < 16; ++index) {
            const auto& sprite = hostGameState.sprites[index];
            uint16_t offset = (0xC100 + index*0x10) & 0x1FFF;
            uint16_t offset2 = (0xC200 + index*0x10) & 0x1FFF;
            std::cout << "Offset: " << static_cast<unsigned int>(offset) << std::endl;
            
            mmu.wram[offset + 0x0] = sprite.pictureId;
            //mmu.wram[offset + 0x1] = sprite.moveStatus;
            mmu.wram[offset + 0x9] = sprite.direction;
            //mmu.wram[offset2 + 0x2] = sprite.yDisplacement;
            //mmu.wram[offset2 + 0x3] = sprite.xDisplacement;
            mmu.wram[offset2 + 0x4] = sprite.yPosition;
            mmu.wram[offset2 + 0x5] = sprite.xPosition;
            //mmu.wram[offset2 + 0x6] = sprite.canMove;
            //mmu.wram[offset2 + 0x7] = sprite.inGrass;
            
            // Calculate and update screen position for each sprite
            auto deltaX = selfPositionX - static_cast<int>(sprite.xPosition) + 4;
            auto deltaY = selfPositionY - static_cast<int>(sprite.yPosition) + 4;
            mmu.wram[offset + 0x6] = 64 - deltaX*16;
            mmu.wram[offset + 0x4] = 60 - deltaY*16;
        }
    }
}