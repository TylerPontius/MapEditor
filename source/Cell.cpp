#include "Cell.hpp"
#include "Map.hpp"
#include <iostream>

Cell::Cell( sf::Uint32 id, sf::Vector3i position, sf::Texture* texture )
{
    myID = id;
    myPosition = position;
    myPosition.z = 0;
    myTexture = texture;
};

void Cell::InitTileLayer( std::vector< std::shared_ptr<Tile> > tiles, sf::Vector3i position )
{
    // Add our current Z layer
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
};

// Store the biomes and areas for a cell that were read from the database
void Cell::SetProperties( std::map< sf::Int32, sf::Uint32 > biomes, std::map< sf::Int32, sf::Uint32 > areas )
{
    myBiomes = biomes;
    myAreas = areas;
};

void Cell::DrawCell( sf::RenderWindow* window, sf::Vector3i position )
{
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

    if( myTileMaps.find( position.z ) != myTileMaps.end() )
        window->draw( myTileMaps[position.z] );
};

void Cell::SetTile( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure we have a layer for the tile
    auto layer = &myTiles[ position.z ];
    InitializeTileMap( position );

    std::vector< std::shared_ptr<Tile> >::iterator toRemove;
    bool isSet = false;

    // Find the previous tile
    for( auto it = layer->begin(); it != layer->end(); ++it )
        if( (*it)->GetPosition() == position )
        {
            toRemove = it;
            isSet = true; std::cout << position.x << position.y << position.z << "\n";
            break;
        }

    // If it exists, remove it
    if( isSet )
        layer->erase( toRemove );

    std::shared_ptr<Tile> newTile (new Tile( position, tile, 255 ) );
    layer->push_back( newTile );
    myTileMaps[ position.z ].AddTile( newTile );
};

void Cell::SetBiome( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure we have a layer for the tile
    InitializeTileMap( position );

    // Remove all the other tiles
    myTiles[ position.z ].clear();

    // Set the biome
    myBiomes[ position.z ] = tile;


    // Make sure the biome isn't blank
    if( tile != 0 )
    {
        // Fill in with biome tile
        for( sf::Uint32 i = 0; i < cellWidth; i++ )
            for( sf::Uint32 j = 0; j < cellHeight; j++ )
            {
                std::shared_ptr<Tile> newTile( new Tile( sf::Vector3i( i, j, position.z ), tile, 255 ) );
                myTiles[ position.z ].push_back( newTile );
                myTileMaps[ position.z ].AddTile( newTile );
            }
    }
    else
        myTileMaps[ position.z ].Clear();
};

sf::Uint32 Cell::GetBiome( sf::Vector3i position )
{
    if( myBiomes.find( position.z ) == myBiomes.end() )
        return 0;
    else
        return myBiomes.find( position.z )->second;
};

void Cell::InitializeTileMap( sf::Vector3i position )
{
    // Add all tile textures
    myTileMaps[position.z].Load( myTexture );
    myTileMaps[position.z].setOrigin( 0.f, 0.f );
    myTileMaps[position.z].setPosition( myPosition.x * tileSize * cellWidth, myPosition.y  * tileSize * cellHeight );
};

