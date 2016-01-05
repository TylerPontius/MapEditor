#ifndef MAP_H
#define MAP_H

#include <string>
#include <iostream>
#include <memory>
#include "Cell.hpp"
#include <Thor/Resources.hpp>

// Map class manages the entire world map
// Any external program functions will interact with this.

class Map : public sf::Drawable, sf::Transformable
{
public:
    Map();
    ~Map();

    void SetTile( sf::Vector3i position, sf::Int32 tile );
    void SetBiome( sf::Vector3i position, sf::Int32 tile );

    std::string GetRegion( sf::Vector3i position, bool subtitle );

    void UpdateLoadedCells( sf::Vector3i position );

    static SQLite::Database db;
    static sf::Texture* tileset;

    thor::ResourceHolder<sf::Texture, sf::Int32> textures;

 private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    void LoadCell( sf::Vector3i position );

    sf::Int32 GetCellID( sf::Vector3i position );

    bool CellExists( sf::Vector3i position );
    bool CellExists( sf::Int32 cellID );

    void RemoveCell( sf::Int32 cellID );

    sf::Vector3i ConvertToCellPosition( sf::Vector3i position );
    sf::Vector3i ConvertToTilePosition( sf::Vector3i position );

    sf::Int32 maxCellID;

    typedef std::unique_ptr<Cell> CellPtr;
    std::map< sf::Int32, CellPtr > myCells;

};


#endif // MAP_H
