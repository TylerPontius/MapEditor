#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <memory>
#include <SQLiteCpp/SQLiteCpp.h>
#include "Cell.hpp"

// Map class manages the entire world map
// Any external program functions will interact with this.

class Map : public sf::Drawable, sf::Transformable
{
public:
    Map();
    ~Map();

    void SetTile( sf::Vector3i position, sf::Uint32 tile );
    void SetBiome( sf::Vector3i position, sf::Uint32 tile );

    void UpdateLoadedCells( sf::Vector3i position );

 private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    Cell& CreateCell( sf::Vector3i position, sf::Uint32 cellID );
    void LoadCell( sf::Vector3i position );

    Cell& GetCell( sf::Vector3i position );
    bool CellExists( sf::Vector3i position );

    void RemoveCell( sf::Uint32 cellID );

    void ConvertToCellPosition( sf::Vector3i& position );
    void ConvertToTilePosition( sf::Vector3i& position );

    SQLite::Database db;
    std::map< sf::Uint32, Cell > myCells;
    sf::Texture* tileset;
    sf::Uint32 maxCellID;
};


#endif // MAP_H
