#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <sqlite3.h>
#include <memory>
#include "Cell.hpp"

// Map file name
#define DB_NAME "map.db"

// Tile size in pixels
#define TILE_SIZE 48

// Tileset file
#define TILESET_FILE "test.png"//"AOtexture.bmp"

// Cell size in tiles
#define CELL_WIDTH 10
#define CELL_HEIGHT 10

// Map size in tiles
#define MAP_WIDTH 1000
#define MAP_HEIGHT 1000

// Number of cells to buffer around screen
#define MAP_BUFFER 4

// Window size in pixels
#define WINDOW_H 768
#define WINDOW_W 1024

// How many layers to draw
#define DRAW_LAYERS 15

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
