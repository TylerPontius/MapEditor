#ifndef TILE_H
#define TILE_H

#include <SFML/System.hpp>
#include <iostream>

class Tile
{
    public:
        Tile( sf::Vector3i position, sf::Uint32 tileID, sf::Uint8 alpha ) : myPosition( position ), myTileID( tileID ), myAlpha( alpha ) {};
        ~Tile() {};

        sf::Vector3i myPosition;
        sf::Uint32 myTileID;
        sf::Uint8 myAlpha;
};

#endif
