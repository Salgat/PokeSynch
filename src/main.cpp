#include <iostream>
#include <chrono>
#include <thread>

using namespace std::literals;

#include "GameBoy.hpp"

sf::RenderWindow window;

void DrawFrame(sf::Image const& frame, GameBoy& gameBoy) {
    window.clear(sf::Color::Green);
	
    sf::Texture texture;
    texture.setSmooth(false);
    texture.loadFromImage(frame, sf::IntRect(0, 0, 160, 144));
    sf::Sprite sprite;
    sprite.setTexture(texture);
	
    window.draw(sprite);
    
    // Draw any pending text
    gameBoy.display.RenderText(window);
    
    window.display();
}

int main(int argc, char* argv[]) {
    std::string game_name = "";
    std::string ipAddress = "";
    std::string name = "";
    unsigned short port = 34231;
    unsigned short hostPort = 34232;
    for (int argument = 1; argument < argc; ++argument) {
        auto arg = std::string(argv[argument]);
        if (arg.find("-game=") != std::string::npos) {
            // Game's File Name to load
            game_name = arg.substr(6);
        } else if (arg.find("-host") != std::string::npos) {
            // Just leave ip blank
        } else if (arg.find("-connect=") != std::string::npos) {
            ipAddress = arg.substr(9);
        } else if (arg.find("-name=") != std::string::npos) {
            name = arg.substr(6);
        } else if (arg.find("-port=") != std::string::npos) {
            port = std::stoi(arg.substr(6));
        } else if (arg.find("-hostport=") != std::string::npos) {
            hostPort = std::stoi(arg.substr(10));
        }
    }

    if (game_name == "") {
        std::cout << "No game loaded." << std::endl;
        return 0;
    }

    window.create(sf::VideoMode(160, 144), "GBS");
    GameBoy gameboy(window, name, port, ipAddress, hostPort);
    gameboy.LoadGame(game_name);

	bool running = true;
    auto start_time = std::chrono::steady_clock::now();
	auto next_frame = start_time + 16.75041876ms;
    while(running) {
		auto result = gameboy.RenderFrame();
        DrawFrame(result.first, gameboy);
        running = result.second;
        
		std::this_thread::sleep_until(next_frame);
		next_frame += 16.75041876ms;
    }
}