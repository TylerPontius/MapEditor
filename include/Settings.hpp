#ifndef SETTINGS_HP
#define SETTINGS_H

// Map file name
const std::string mapFile = "map.db";

// Tile size in pixels
const sf::Uint32 tileSize = 48;

// Tileset file
const std::string tilesetFile = "test.png"; //"AOtexture.bmp"

// Cell size in tiles
const sf::Uint32 cellWidth = 16;
const sf::Uint32 cellHeight = 16;

// Map size in tiles
const sf::Uint32 mapWidth = 1000;
const sf::Uint32 mapHeight = 1000;

// Number of cells to buffer around screen
const sf::Uint32 mapBuffer = 4;

// Window size in pixels
const sf::Uint32 windowHeight = 768;
const sf::Uint32 windowWidth = 1024;

// How many layers to draw
const sf::Uint32 drawLayers = 16;

// Should map changes be saved?
extern bool saveChanges;

#endif