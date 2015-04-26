#ifndef CELL_H
#define CELL_H

#include <memory>
#include <map>
#include "Tile.hpp"
#include "TileMap.hpp"

class Cell
{
public:
    Cell( sf::Uint32 id, sf::Vector3i position, sf::Texture* texture );
    ~Cell() {};

    void InitTileLayer( std::vector< std::shared_ptr<Tile> > tiles, sf::Vector3i position );
    void SetProperties( std::map< sf::Int32, sf::Uint32 > biomes, std::map< sf::Int32, sf::Uint32 > areas );
    void DrawCell( sf::RenderWindow* window, sf::Vector3i position );

    void SetTile( sf::Vector3i position, sf::Uint32 tile );
    void SetBiome( sf::Vector3i position, sf::Uint32 tile );

    sf::Uint32 GetBiome( sf::Vector3i position );

    std::map< sf::Int32, sf::Uint32 > GetBiomes() { return myBiomes; };
    std::map< sf::Int32, sf::Uint32 > GetAreas() { return myAreas; };
    std::map< sf::Int32, std::vector< std::shared_ptr<Tile> > > GetTiles() { return myTiles; };

    sf::Uint32 GetID() { return myID; };
    sf::Vector3i GetPosition() { return myPosition; };

private:
    sf::Uint32 myID;
    sf::Vector3i myPosition;
    sf::Texture* myTexture;

    void InitializeTileMap( sf::Vector3i position );

    // Map Z layer with properties
    std::map< sf::Int32, std::vector< std::shared_ptr<Tile> > > myTiles;
    std::map< sf::Int32, sf::Uint32 > myBiomes, myAreas;
    std::map< sf::Int32, TileMap > myTileMaps;
};

#endif
