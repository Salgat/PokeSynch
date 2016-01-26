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
        network.Host(hostPort);
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
    network.Update(localGameState);
    
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
    
    localGameState.currentMap = mmu.eram[0xD35E];
    
    localGameState.playerPosition.yPosition = mmu.eram[0xD361];
    localGameState.playerPosition.xPosition = mmu.eram[0xD362];
    localGameState.playerPosition.yBlockPosition = mmu.eram[0xD363];
    localGameState.playerPosition.xBlockPosition = mmu.eram[0xD364];
    
    localGameState.sprites.resize(16);
    for (uint16_t index = 0; index < 16; ++index) {
        uint16_t offset = 0xC100 + index*0x10;
        uint16_t offset2 = 0xC200 + index*0x10;
        
        auto& sprite = localGameState.sprites[index];
        sprite.spriteIndex = index;
        sprite.pictureId = mmu.eram[offset + 0x0];
        sprite.moveStatus = mmu.eram[offset + 0x1];
        sprite.direction = mmu.eram[offset + 0x9];
        sprite.yDisplacement = mmu.eram[offset2 + 0x2];
        sprite.xDisplacement = mmu.eram[offset2 + 0x3];
        sprite.yPosition = mmu.eram[offset2 + 0x4];
        sprite.xPosition = mmu.eram[offset2 + 0x5];
        sprite.canMove = mmu.eram[offset2 + 0x6];
        sprite.inGrass = mmu.eram[offset2 + 0x7];
    }
    
    return localGameState;
}