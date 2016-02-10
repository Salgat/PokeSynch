//
// Created by Austin on 6/11/2015.
//

#ifndef GAMEBOYEMULATOR_DISPLAY_HPP
#define GAMEBOYEMULATOR_DISPLAY_HPP

#include <vector>
#include <utility>
#include <algorithm>
#include <array>
#include <queue>

// SFML
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include "Network.hpp"

class Processor;
class MemoryManagementUnit;
class Input;

/**
 * Defines each of the 40 sprites in the OAM sprite table
 */
struct Sprite {
    int x = 0;
    int y = 0;
    int tile_number = 0;
    uint8_t attributes = 0;

    bool x_flip = false;
    bool y_flip = false;
    bool draw_priority = false;
    uint8_t palette = 0x00;
    int height = 8;
};

enum class PlayerDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * Holds the current state of the simulated player.
 */
struct SimulatedPlayerState {
    int xPosition; int xPositionDestination;
    int yPosition; int yPositionDestination;
    int xPixelOffset;
    int yPixelOffset;
    int walkCounter; // Increment every cycle, with a pixel increase every four
    PlayerDirection direction;
    bool alternateStep; // Tracks the frame to show when walking
    int uniqueId;
    int walkBikeSurfState;
};

/**
 * Holds instructions for what text to display.
 */
struct TextToDisplay {
    int offsetY;
    int offsetX;
    int line; // Either the first or second line of text
    sf::Text text;
};

const unsigned int kWindowOffsetY = 144 - 48; // Window height is 48 pixels
const unsigned int kOptionsWindowOffsetY = kWindowOffsetY - 40; // Window height is 40 pixels
const unsigned int kOptionsWindowOffsetX = 160 - 73;

/**
 * Handles rendering to the "screen".
 */
class Display {
	friend class Input;

    sf::Color const kWhite       = sf::Color(255, 255, 255, 255);
    sf::Color const kLightGray   = sf::Color(192, 192, 192, 255);
    sf::Color const kDarkGray    = sf::Color( 96,  96,  96, 255);
    sf::Color const kBlack       = sf::Color(  0,   0,   0, 255);
    sf::Color const kTransparent = sf::Color(  1,   1,   1,   0);

public:
    Display();

    void Initialize(Processor* cpu_, MemoryManagementUnit* mmu_);
    void Reset();
	void RenderFrame();
    void RenderScanline(uint8_t line_number);
	
	void UpdateSprite(uint8_t sprite_address, uint8_t value);
    
    // Synchronize Logic
    void DisplayPlayers(HostGameState hostGameState, int myUniqueId);
    void DrawSpriteToImage(sf::Image spriteImage, int frame, int pixelPositionX, int pixelPositionY);
    void DrawWindowWithText(const std::string& message, int line);
    void DrawOptionsWindowWithText(const std::string& message, int line, bool selected);
    void RenderText(sf::RenderWindow& window);
    
	sf::Image frame;
    std::unordered_map<int, SimulatedPlayerState> simulatedPlayerStates;
	
private:
    Processor* cpu;
    MemoryManagementUnit* mmu;

    std::vector<sf::Color> background;
    std::vector<bool> show_background;
    std::vector<sf::Color> window;
    std::vector<bool> show_window;
    std::vector<sf::Color> sprite_map;
    std::vector<bool> show_sprite;
    std::vector<bool> sprite_priority; // 0 = False, 1 = True
	std::array<Sprite, 40> sprite_array;
    std::queue<TextToDisplay> textQueue;
    
    // Synchronize Properties
    std::vector<sf::Texture> spriteTextures;
    std::vector<sf::Image> spriteImages;
    std::unordered_map<int, int> spriteFrame;
    std::vector<sf::Font> fonts;
    std::vector<sf::Texture> windowTextures;
    std::vector<sf::Image> windowImages;
    bool textWindowDrawn;
    bool textOptionsWindowDrawn;

    std::array<int, 8> DrawTilePattern(uint16_t tile_address, int tile_line);
	void DrawBackground(uint8_t lcd_control, int line_number);
    void DrawWindow(uint8_t lcd_control, int line_number);
	void DrawBackgroundOrWindow(uint8_t lcd_control, int line_number, bool is_background);
    std::vector<Sprite*> ReadSprites(uint8_t lcd_control);
    void DrawSprites(uint8_t lcd_control, int line_number, std::vector<Sprite*> const& sprites);
    void DrawImage(unsigned int xOffset, unsigned int yOffset, const sf::Image& image);
    
    // Synchronize Logic
    void DrawSpriteToImage(sf::Image spriteImage, int frameNumber, int pixelPositionX, int pixelPositionY, bool flipHorizontal);
    void WalkSimulatedPlayer(SimulatedPlayerState& simulatedPlayerState, PlayerDirection direction);
};

#endif //GAMEBOYEMULATOR_DISPLAY_HPP
