#ifndef TILE_H
#define TILE_H

#include <SFML/System.hpp>

class Tile
{
    public:
        Tile( sf::Vector3i position, sf::Uint32 tileID, sf::Uint8 alpha ) : myPosition( position ), myTileID( tileID ), myAlpha( alpha ) {};
        ~Tile() {};

        const sf::Vector3i myPosition;
        const sf::Uint32 myTileID;
        const sf::Uint8 myAlpha;
};

#endif
