#include "Cell.hpp"
#include <iostream>

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

    // If z is negative, subtract 1 to avoid drawing the wrong set of layers
    if( z < 0 )
        z -= drawLayers - 1;

    // Exploit integer division to round to the nearest layer base
    z /= drawLayers;
    z *= drawLayers;

    // Draw our layers
    for( sf::Int32 i = z; i < (z + (sf::Int32)drawLayers); i++ )
        if( myLayers.find( i ) != myLayers.end() )
            target.draw( myLayers.at( i ), states );
};

// Create a layer if it doesn't exist
void Cell::AddLayer( sf::Vector3i position, sf::Texture* tileset )
{
    if( myLayers.find( position.z ) == myLayers.end() )
    {
        myLayers.emplace( position.z, Layer{sf::Vector3i( myPosition.x, myPosition.y, position.z ), tileset} );

        Layer& newLayer = myLayers.at( position.z );

        // Set the tile info
        newLayer.setOrigin( 0.f, 0.f );
        newLayer.setPosition( position.x * tileSize * cellWidth, position.y  * tileSize * cellHeight );
    }
};

void Cell::RemoveLayer( sf::Vector3i position )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.erase( position.z );
};

