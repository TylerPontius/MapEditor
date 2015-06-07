#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <sqlite3.h>
#include <memory>
#include "Cell.hpp"

// Map class manages the entire world map
// Any external program functions will interact with this.

class Map : public sf::Drawable
{
public:
    Map();
    ~Map();

    void SetTile( sf::Vector3i position, sf::Uint32 tile );
    void SetBiome( sf::Vector3i position, sf::Uint32 tile );

    void UpdateLoadedCells( sf::Vector3i position );

 private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;



    void CreateCell( sf::Vector3i position, sf::Uint32 cell );
    sf::Uint32 LoadCell( sf::Vector3i position );

    void LoadCellTileLayer( sf::Uint32 cell, sf::Vector3i position );
    sf::Uint32 GetCell( sf::Vector3i position );

    void SaveCell( sf::Uint32 cell );

    sf::Vector3i ConvertToCellPosition( sf::Vector3i position );
    sf::Vector3i ConvertToTilePosition( sf::Vector3i position );

    sqlite3* db;
    std::map< sf::Uint32, std::unique_ptr<Cell> > myCells;
    sf::Texture* texture;
    sf::Uint32 maxCellID;
};


#endif // MAP_H
