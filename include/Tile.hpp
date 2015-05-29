#ifndef TILE_H
#define TILE_H

#include <SFML/System.hpp>

class Tile
{
    protected:
        sf::Vector3i myPosition;
        sf::Uint32 myTile;
        sf::Uint8 myAlpha;

    public:
        Tile( sf::Vector3i position, sf::Uint32 tile, sf::Uint8 alpha );
        ~Tile() {};

        void SetPosition( sf::Vector3i position ) { myPosition = position; };
        void SetTile( sf::Uint32 tile ) { myTile = tile; };
        void SetAlpha( sf::Uint8 alpha ) { myAlpha = alpha; };

        sf::Vector3i GetPosition() { return myPosition; };
        sf::Uint32 GetTile() { return myTile; };
        sf::Uint8 GetAlpha() { return myAlpha; };

};

#endif
