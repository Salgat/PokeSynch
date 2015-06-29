//
// Special credit goes to Imran Nazar's JavaScript implementation of the input (written in JavaScript), which was
// used as a reference. imrannazar.com/GameBoy-Emulation-in-Javascript:-Input
//

#include "Input.hpp"
#include "MemoryManagementUnit.hpp"

Input::Input() {
    Reset();
}

void Input::Initialize(MemoryManagementUnit* mmu_, sf::RenderWindow* window_) {
    mmu = mmu_;
    window = window_;
}

/**
 * Resets Input to an initial state.
 */
void Input::Reset() {
    rows[0] = 0x0F;
    rows[1] = 0x0F;
    column = 0x00;
}

/**
 * Depending on the bits written to 5 and 6 of 0xFF00, returns which selected inputs were pressed and not pressed.
 */
uint8_t Input::ReadByte() {
    switch (column) {
        case 0x00:
            return 0x00;
        case 0x10:
            return rows[0];
        case 0x20:
            return rows[1];
        default:
            return 0x00;
    }
}

/**
 * Selects which inputs to return upon reading.
 */
void Input::WriteByte(uint8_t value) {
    column = value & 0x30;
}

/**
 * Sets bits for inputs that are released (1 = not pressed).
 */
void Input::KeyUp(KeyType key) {
    switch(key) {
        case KeyType::RIGHT:  rows[1] |= 0x01; break;
        case KeyType::LEFT:   rows[1] |= 0x02; break;
        case KeyType::UP:     rows[1] |= 0x04; break;
        case KeyType::DOWN:   rows[1] |= 0x08; break;
        case KeyType::A:      rows[0] |= 0x01; break;
        case KeyType::B:      rows[0] |= 0x02; break;
        case KeyType::SELECT: rows[0] |= 0x04; break;
        case KeyType::START:  rows[0] |= 0x08; break;
    }
}

/**
 * Clears bits for inputs that are pressed (0 = pressed).
 */
void Input::KeyDown(KeyType key) {
    mmu->interrupt_flag |= 0x04;

    switch(key) {
        case KeyType::RIGHT:  rows[1] &= 0xE; break;
        case KeyType::LEFT:   rows[1] &= 0xD; break;
        case KeyType::UP:     rows[1] &= 0xB; break;
        case KeyType::DOWN:   rows[1] &= 0x7; break;
        case KeyType::A:      rows[0] &= 0xE; break;
        case KeyType::B:      rows[0] &= 0xD; break;
        case KeyType::SELECT: rows[0] &= 0xB; break;
        case KeyType::START:  rows[0] &= 0x7; break;
    }
}

/**
 * Handles user inputs such as close window and resize window.
 */
bool Input::PollEvents() {
    sf::Event event;
    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window->close();
            return false;
        }
    }

    return true;
}

/**
 * Upon reading 0xFF00, updates the inputs to be returned.
 */
void Input::UpdateInput() {
    static bool left = false;
    static bool right = false;
    static bool up = false;
    static bool down = false;
    static bool a = false;
    static bool b = false;
    static bool start = false;
    static bool select = false;

    // Read button states
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) and !left) {
        KeyDown(KeyType::LEFT);
        left = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Left) and left) {
        KeyUp(KeyType::LEFT);
        left = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) and !right) {
        KeyDown(KeyType::RIGHT);
        right = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Right) and right) {
        KeyUp(KeyType::RIGHT);
        right = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) and !up) {
        KeyDown(KeyType::UP);
        up = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) and up) {
        KeyUp(KeyType::UP);
        up = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) and !down) {
        KeyDown(KeyType::DOWN);
        down = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Down) and down) {
        KeyUp(KeyType::DOWN);
        down = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) and !a) {
        KeyDown(KeyType::A);
        a = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::X) and a) {
        KeyUp(KeyType::A);
        a = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) and !b) {
        KeyDown(KeyType::B);
        b = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Z) and b) {
        KeyUp(KeyType::B);
        b = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return) and !start) {
        KeyDown(KeyType::START);
        start = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Return) and start) {
        KeyUp(KeyType::START);
        start = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) and !select) {
        KeyDown(KeyType::SELECT);
        select = true;
    } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) and select) {
        KeyUp(KeyType::SELECT);
        select = false;
    }
}