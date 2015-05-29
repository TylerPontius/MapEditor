#include "Tile.hpp"
#include <iostream>

Tile::Tile( sf::Vector3i position, sf::Uint32 tile, sf::Uint8 alpha )
{
    SetPosition( position );
    SetTile( tile );
    SetAlpha( alpha );

    //std::cout << "I am tile " << tileID << " at " << xPos << ", " << yPos << ", " << zPos << " with a = " << (int)alpha << std::endl;
};

