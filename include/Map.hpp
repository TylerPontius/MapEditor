#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <memory>
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

    std::string GetRegion( sf::Vector3i position, bool subtitle );

    void UpdateLoadedCells( sf::Vector3i position );

    static SQLite::Database db;
    static sf::Texture* tileset;

 private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    Cell& CreateCell( sf::Vector3i position, sf::Uint32 cellID );
    void LoadCell( sf::Vector3i position );

    Cell& GetCell( sf::Vector3i position );
    Cell& GetCell( sf::Uint32 cellID );

    bool CellExists( sf::Vector3i position );
    bool CellExists( sf::Uint32 cellID );

    void RemoveCell( sf::Uint32 cellID );

    sf::Vector3i ConvertToCellPosition( sf::Vector3i position );
    sf::Vector3i ConvertToTilePosition( sf::Vector3i position );

    sf::Uint32 maxCellID;
    std::map< sf::Uint32, Cell > myCells;

};


#endif // MAP_H
