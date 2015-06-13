#ifndef TILE_H
#define TILE_H

#include <SFML/System.hpp>

class Tile
{
    public:
        Tile( const sf::Vector3i position, const sf::Uint32 tileID, const sf::Uint8 alpha ) : myPosition( position ), myTileID( tileID ), myAlpha( alpha ) {};
        ~Tile() {};

        sf::Vector3i myPosition;
        sf::Uint32 myTileID;
        sf::Uint8 myAlpha;
};

#endif
