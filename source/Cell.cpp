#include "Cell.hpp"

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
    myLayers.at( position.z ).SetBiome( position, tile );
};

// Return the biome tile ID, or 0 if there is none
sf::Uint32 Cell::GetBiome( sf::Vector3i position )
{
    if( myLayers.find( position.z ) == myLayers.end()
        return 0;
    else
        return myLayers.at( position.z ).GetBiome();
};

void Cell::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = myTileset;

    // draw the vertex array
    target.draw(myVertices, states);

    // Make sure we don't draw other layers
    // Exploit integer division to round to the nearest layer base
    if( position.z >= 0 )
        position.z /= drawLayers;
    else
    {
        position.z -= drawLayers-1;
        position.z /= drawLayers;
    }
    position.z *= drawLayers;

    for( auto it : myTileMaps )
        if( it.first == position.z )
            window->draw( myTileMaps[position.z] );
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
        newLayer.setPosition( myPosition.x * tileSize * cellWidth, myPosition.y  * tileSize * cellHeight );
    }
};

void Cell::RemoveLayer( sf::Vector3i position )
{
    if( myLayers.find( position.z ) != myLayers.end() )
        myLayers.erase( position.z );
};

