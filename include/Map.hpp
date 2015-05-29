#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <sqlite3.h>
#include <memory>
#include "Cell.hpp"

// Map file name
const std::string mapFile = "map.db";

// Tile size in pixels
const sf::Uint32 tileSize = 48;

// Tileset file
const std::string tilesetFile = "test.png"; //"AOtexture.bmp"

// Cell size in tiles
const sf::Uint32 cellWidth = 10;
const sf::Uint32 cellHeight = 10;

// Map size in tiles
const sf::Uint32 mapWidth = 1000;
const sf::Uint32 mapHeight = 1000;

// Number of cells to buffer around screen
const sf::Uint32 mapBuffer = 4;

// Window size in pixels
const sf::Uint32 windowHeight = 768;
const sf::Uint32 windowWidth = 1024;

// How many layers to draw
const sf::Uint32 drawLayers = 15;

extern bool saveChanges;


class Map
{
public:
    Map();
    ~Map();
    void SetCellTile( sf::Vector3i position, sf::Uint32 tile );
    void SetCellBiome( sf::Vector3i position, sf::Uint32 tile );

    void UpdateLoadedCells( sf::Vector3i position );
    void DrawMap( sf::RenderWindow* window, sf::Vector3i position );

    sf::Texture* GetTexture() { return texture; };

 private:
    void CreateCell( sf::Vector3i position, sf::Uint32 cell );
    sf::Uint32 LoadCell( sf::Vector3i position );

    void LoadCellTileLayer( sf::Uint32 cell, sf::Vector3i position );
    sf::Uint32 GetCell( sf::Vector3i position );

    void SaveCell( sf::Uint32 cell );

    sf::Vector3i ConvertToCellPosition( sf::Vector3i position );
    sf::Vector3i ConvertToTilePosition( sf::Vector3i position );

    sqlite3* db;
    std::map< sf::Uint32, std::shared_ptr<Cell> > myCells;
    sf::Texture* texture;
    sf::Uint32 maxCellID;
};


#endif // MAP_H
