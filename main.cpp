#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Function to convert hex color string to sf::Color
sf::Color hexToColor(const std::string& hex) {
    unsigned int r, g, b;
    std::sscanf(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
    return sf::Color(r, g, b);
}

int main() {
    // Load configuration from JSON file
    std::ifstream configFile("assets/config.json");
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config.json" << std::endl;
        return 1;
    }

    json config;
    configFile >> config;

    // Extract configuration values
    sf::Color clockColor = hexToColor(config["clock_color"]);
    std::string fontPath = config["clock_font"];
    sf::Color backgroundColor = hexToColor(config["background_color"]);
    sf::Color flashColor = hexToColor(config["flash_color"]);
    std::string audioFilePath = config["audio_file"];
    int alarmDuration = config["alarm_duration"];
    std::map<std::string, std::string> alarmSchedule = config["alarm_schedule"];

    // Initialize SFML window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Digital Clock");

    // Load font
    sf::Font font;
    if (!font.openFromFile(fontPath)) {
        std::cerr << "Error: Could not load font" << std::endl;
        return 1;
    }

    // Load sound buffer and create sound
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(audioFilePath)) {
        std::cerr << "Error: Could not load sound file" << std::endl;
        return 1;
    }
    sf::Sound alarmSound(buffer);

    // Create clock text
    sf::Text clockText(font, "", 72); // Correct order: font, string, character size
    clockText.setFillColor(clockColor);
    clockText.setPosition({250, 250});

    // Main loop
    while (window.isOpen()) {
        // Poll events
        while (auto event = window.pollEvent()) {
            if (event.has_value() && event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
        std::tm* localTime = std::localtime(&currentTime);

        // Format time as HHMM
        char timeStr[5];
        std::strftime(timeStr, sizeof(timeStr), "%H%M", localTime);
        std::string currentTimeStr = timeStr;

        // Get current day
        const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        std::string currentDay = days[localTime->tm_wday];

        // Check if alarm should trigger
        bool alarmTriggered = false;
        if (alarmSchedule.find(currentDay) != alarmSchedule.end() && alarmSchedule[currentDay] == currentTimeStr) {
            alarmTriggered = true;
        }

        // Update clock text
        clockText.setString(timeStr);

        // Set background color
        if (alarmTriggered) {
            // Flash background
            for (int i = 0; i < alarmDuration * 2; ++i) {
                window.clear((i % 2 == 0) ? flashColor : backgroundColor);
                window.draw(clockText);
                window.display();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            // Play sound
            if (alarmSound.getStatus() != sf::Sound::Status::Playing) {
                alarmSound.play();
            }

            // Reset background color
            window.clear(backgroundColor);
        } else {
            window.clear(backgroundColor);
        }

        // Draw clock
        window.draw(clockText);
        window.display();
    }

    return 0;
}