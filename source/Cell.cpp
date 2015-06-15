#include "Cell.hpp"
#include <iostream>

Cell::Cell( sf::Uint32 id, sf::Vector3i position, sf::Texture* tileset, sqlite3* db ) : myID( id ), myPosition( position )
{
    // Load our layers from the database

    // Make a SQL query
    std::string query = "SELECT Z, Biome, Air, Region FROM Layers WHERE Cell = ?";
    sqlite3_stmt* statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, myID );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }

    // Looks good! Step it
    err = sqlite3_step( statement );

    while( err != SQLITE_DONE )
    {
        if( err == SQLITE_ROW )
        {
            // Everything is good to go! Make a new layer
            sf::Vector3i layerPos = position;
            layerPos.z        = sqlite3_column_int( statement, 0 );
            sf::Uint32 biome  = sqlite3_column_int( statement, 1 );
            sf::Uint32 air    = sqlite3_column_int( statement, 2 );
            sf::Uint32 region = sqlite3_column_int( statement, 3 );

            AddLayer( layerPos, tileset );
            SetBiome( layerPos, biome, tileset );

            // Read in the next row
            err = sqlite3_step( statement );
        }

        // If we didn't get a row, process the error
        else if( err != SQLITE_DONE and err != SQLITE_ROW )
            std::cout << "Step failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
    }

    sqlite3_finalize( statement );
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

