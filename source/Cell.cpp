#include "Cell.hpp"
#include "Map.hpp"
#include <iostream>
/*
// Load the graphics for a tile layer
void Cell::InitTileLayer( std::vector< std::shared_ptr<Tile> > tiles, sf::Vector3i position )
{
    // Add our current Z layer (if there is a biome for it)
    if( GetBiome( position ) != 0 )
        SetBiome( position, GetBiome( position ) );

    // Create the tile layer
    if( !tiles.empty() )
    {
        // Initialize the map and set the tiles
        InitializeTileMap( position );
        myTiles[ position.z ] = tiles;

        // Add the tiles to the graphics
        for( auto it = tiles.begin(); it != tiles.end(); ++it )
            myTileMaps[position.z].AddTile( *it );
    }
};*/


void Cell::draw(sf::RenderTarget& target, sf::RenderStates states) const
{/*
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
            window->draw( myTileMaps[position.z] );*/
};

void Cell::SetTile( sf::Vector3i position, sf::Uint32 tile )
{/*
    // Make sure we have a layer for the tile
    auto& layer = myTiles[ position.z ];
    InitializeTileMap( position );

    // Remove the existing tile from the layer if it already exists
    std::remove_if( layer.begin(), layer.end(),
                    [&] ( std::shared_ptr<Tile> pos ) { return (pos->position == position); } );

    // Make the new tile
    auto newTile = layer.emplace( layer.end(), std:*/
};


// Set a Z layer to have a biome
void Cell::SetBiome( sf::Vector3i position, sf::Uint32 tile )
{/*
    // Make sure we have a layer for the tile
    InitializeTileMap( position );

    // Remove all the other tiles
    auto& tileVec = myTiles[ position.z ];
    tileVec.clear();

    // Set the biome
    myBiomes[ position.z ] = tile;

    // Get the TileMap
    auto& tMap = myTileMaps[ position.z ];

    // Make sure the biome isn't blank
    if( tile != 0 )
    {
        // Fill in with biome tile
        for( sf::Uint32 i = 0; i < cellWidth; i++ )
            for( sf::Uint32 j = 0; j < cellHeight; j++ )
            {
                auto newTile = std::make_shared<Tile>( sf::Vector3i( i, j, position.z ), tile, 255 );
                tileVec.push_back( newTile );
                tMap.AddTile( newTile );
            }
    }
    else
        tMap.Clear();*/
};

// Return the biome tile ID, or 0 if there is none
sf::Uint32 Cell::GetBiome( sf::Vector3i position )
{/*
    auto biome = myBiomes.find( position.z );

    if( biome == myBiomes.end() )
        return 0;
    else
        return biome->second;*/
};
/*
// Initialize the graphics for the TileMap
//void Cell::InitializeTileMap( sf::Vector3i position )
{
    // Create the tile map if we need to
    if( myTileMaps.find( position.z ) == myTileMaps.end() )
    {
        myTileMaps.emplace( position.z, std::make_shared<TileMap>(myTexture) );
    }

    // Grab the tile map
    auto tMap = myTileMaps[position.z];

    // Set the tile info
    tMap.setOrigin( 0.f, 0.f );
    tMap.setPosition( myPosition.x * tileSize * cellWidth, myPosition.y  * tileSize * cellHeight );
};*/

