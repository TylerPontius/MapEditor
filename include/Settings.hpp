#ifndef SETTINGS_HP
#define SETTINGS_H

namespace Settings
{
    // Version of this program
    const std::string version = "0.2";

    // Map file name
    const std::string mapFile = "map.db";

    // Tile size in pixels
    const sf::Int32 tileSize = 48;

    // Tileset file
    const std::string tilesetFile = "test.png";//"AOtexture.bmp";

    // Cell size in tiles
    const sf::Int32 cellWidth = 16;
    const sf::Int32 cellHeight = 16;

    // Map size in tiles
    const sf::Int32 mapWidth = 10000;
    const sf::Int32 mapHeight = 10000;

    // Number of cells to buffer around screen
    const sf::Int32 mapBuffer = 9;

    // Window size in pixels
    const sf::Int32 windowHeight = 768;
    const sf::Int32 windowWidth = 1024;

    // How many layers to draw
    const sf::Int32 drawLayers = 16;

    // Current position (used for drawing)
    extern sf::Vector3i globalPosition;

    // Should map changes be saved?
    extern bool saveChanges;
    extern bool showFPS;
}


#endif
