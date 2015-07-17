#include "Cell.hpp"
#include <iostream>

Cell::Cell( sf::Uint32 id, sf::Vector3i position ) : myID( id ), myPosition( position )
{
    // Set the tile info
    setOrigin( 0.f, 0.f );
    setPosition( position.x * tileSize * cellWidth / 2, position.y  * tileSize * cellHeight / 2 );
    //std::cout << "New cell at " << tileSize * cellWidth << ", " << tileSize * cellHeight << "\n";
    //std::cout << "  I am: " << position.x << ", " << position.y << "\n";
};


void Cell::SetTile( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z ).SetTile( position, tile );

};

// Set a Z layer to have a biome
void Cell::SetBiome( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z ).SetBiome( tile );
};

void Cell::SetAir( sf::Vector3i position, sf::Uint32 air )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.at( position.z ).SetAir( air );
};

void Cell::SetRegion( sf::Vector3i position, sf::Uint32 region )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.at( position.z ).SetRegion( region );
};

// Return the biome tile ID, or 0 if there is none
sf::Uint32 Cell::GetBiome( sf::Vector3i position )
{
    if( myLayers.find( position.z ) == myLayers.end() )
        return 0;
    else
        return myLayers.at( position.z ).GetBiome();
};

void Cell::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // Apply the transform
    states.transform *= getTransform();

    // Make sure we don't draw other layers
    sf::Int32 z = globalPosition.z;

    // Round to the nearest layer base
    if( z < 0 and (z % drawLayers) != 0 )
        z = z - ( drawLayers - (-z % drawLayers) );

    else
        z = z - (z % drawLayers);

    // Draw our layers
    for( sf::Int32 i = z; i < (z + drawLayers); i++ )
        if( myLayers.find( i ) != myLayers.end() )
            target.draw( myLayers.at( i ), states );
};

// Create a layer if it doesn't exist
void Cell::AddLayer( sf::Vector3i position, sf::Texture* tileset )
{
    std::cout << "looking for layer " << position.z << "\n";
    if( myLayers.find( position.z ) == myLayers.end() )
    {
        std::cout << "looking to make\n";
        myLayers.emplace(   std::piecewise_construct,
                            std::forward_as_tuple( position.z ),
                            std::forward_as_tuple( std::initializer_list<sf::Vector3i, sf::Texture*>( sf::Vector3i( myPosition.x, myPosition.y, position.z ), tileset ) )
                         );
std::cout << "made\n";
        Layer& newLayer = myLayers.at( position.z );

        // Set the tile info
        newLayer.setOrigin( getOrigin() );
        newLayer.setPosition( getPosition() );

        //std::cout << "I am layer " << position.z << " at " << newLayer.getPosition().x << ", " << newLayer.getPosition().y << "\n";
    }
    std::cout << "done looking for layer\n\n\n";
};

void Cell::RemoveLayer( sf::Vector3i position )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.erase( position.z );
};

