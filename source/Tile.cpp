#include "Tile.hpp"
#include <iostream>

Tile::Tile( sf::Uint16 tile, sf::Uint32 x, sf::Uint32 y, sf::Int32 z, sf::Uint8 a )
{
    SetTileID( tile );
    SetPosition( x, y, z );
    SetAlpha( a );

    //std::cout << "I am tile " << tileID << " at " << xPos << ", " << yPos << ", " << zPos << " with a = " << (int)alpha << std::endl;
};

