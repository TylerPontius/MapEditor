#include "Cell.hpp"
#include <iostream>

Cell::Cell( sf::Uint32 id, sf::Vector3i position ) : myID( id ), myPosition( position )
{
    // Set the tile info
    setOrigin( 0.f, 0.f );
    setPosition( position.x * tileSize * cellWidth / 2, position.y  * tileSize * cellHeight / 2 );
};


void Cell::SetTile( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    if( !LayerExists( position ) )
        AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z )->SetTile( position, tile );

};

// Set a Z layer to have a biome
void Cell::SetBiome( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    if( !LayerExists( position ) )
        AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z )->SetBiome( tile );
};

void Cell::SetAir( sf::Vector3i position, sf::Uint32 air )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.at( position.z )->SetAir( air );
};

void Cell::SetRegion( sf::Vector3i position, sf::Uint32 region )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.at( position.z )->SetRegion( region );
};

// Return the biome tile ID, or 0 if there is none
sf::Uint32 Cell::GetBiome( sf::Vector3i position )
{
    if( myLayers.find( position.z ) == myLayers.end() )
        return 0;
    else
        return myLayers.at( position.z )->GetBiome();
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
            target.draw( *(myLayers.at( i )), states );
};

bool Cell::LayerExists( sf::Vector3i position )
{
    if( myLayers.find( position.z ) == myLayers.end() )
        return false;
    return true;
};

// Create a layer if it doesn't exist
void Cell::AddLayer( sf::Vector3i position, sf::Texture* tileset )
{
    // Make the layer with the given info
    LayerPtr newLayer(new Layer( sf::Vector3i( myPosition.x, myPosition.y, position.z ), tileset ) );

    // Set the tile info
    newLayer->setOrigin( getOrigin() );
    newLayer->setPosition( getPosition() );
    myLayers.insert( std::pair<sf::Int32, LayerPtr> (position.z, std::move(newLayer) ) );
};

void Cell::RemoveLayer( sf::Vector3i position )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.erase( position.z );
};

