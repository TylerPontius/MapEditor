#include "Cell.hpp"
#include "Map.hpp"
#include <iostream>

Cell::Cell( sf::Int32 id, sf::Vector3i position ) : myID( id ), myPosition( position )
{
    // Set the tile info
    setOrigin( 0.f, 0.f );
    setPosition( position.x * Settings::tileSize * Settings::cellWidth / 2, position.y  * Settings::tileSize * Settings::cellHeight / 2 );

    Load();
};

Cell::~Cell()
{
    if( Settings::saveChanges ) Save();
};

void Cell::Save()
{
    // Create the cell entry if it doesn't already exist
    bool exists = false;

    try
    {
        SQLite::Statement query( Map::db, "SELECT ID FROM Cells WHERE X = ? AND Y = ?" );

        // Bind our request
        query.bind( 1, myPosition.x );
        query.bind( 2, myPosition.y );

        while( query.executeStep() )
        {
            exists = true;
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }

    if( !exists )
    {
        // Make a SQL query
        try
        {
            // Make a SQL query
            SQLite::Statement query( Map::db, "INSERT INTO Cells VALUES( ?, ?, ? )" );

            // Bind our request
            query.bind( 1, myID );
            query.bind( 2, myPosition.x );
            query.bind( 3, myPosition.y );

            query.exec();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }

    // Save all layers
    for( auto& layer : myLayers )
        layer.second->Save();
};

void Cell::Load()
{
    myLayers.clear();

    try
    {
        // Make a SQL query
        SQLite::Statement query( Map::db, "SELECT Z, Biome, Air, Region FROM Layers WHERE Cell = ?" );

        // Bind our request
        query.bind( 1, myID );

        while( query.executeStep() )
        {
            // Everything is good to go! Make a new layer
            sf::Vector3i layerPos = myPosition;
            layerPos.z        = query.getColumn( 0 ).getInt();
            sf::Int32 biome  = query.getColumn( 1 ).getInt();
            sf::Int32 air    = query.getColumn( 2 ).getInt();
            sf::Int32 region = query.getColumn( 3 ).getInt();

            AddLayer ( layerPos, Map::tileset );
            SetBiome ( layerPos, biome, Map::tileset );
            SetAir   ( layerPos, air );
            SetRegion( layerPos, region );

            myLayers.at( layerPos.z )->Load();
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
};


void Cell::SetTile( sf::Vector3i position, sf::Int32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    if( !LayerExists( position ) )
        AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z )->SetTile( position, tile );

};

// Set a Z layer to have a biome
void Cell::SetBiome( sf::Vector3i position, sf::Int32 tile, sf::Texture* tileset )
{
    // Make sure we have a layer for the tile
    if( !LayerExists( position ) )
        AddLayer( position, tileset );

    // Set the tile
    myLayers.at( position.z )->SetBiome( tile );
};

void Cell::SetAir( sf::Vector3i position, sf::Int32 air )
{
    if( LayerExists( position ) )
        myLayers.at( position.z )->SetAir( air );
};

void Cell::SetRegion( sf::Vector3i position, sf::Int32 region )
{
    if( LayerExists( position ) )
        myLayers.at( position.z )->SetRegion( region );
};

// Return the biome tile ID, or 0 if there is none
sf::Int32 Cell::GetTile( sf::Vector3i position )
{
    if( LayerExists( position ) )
        return myLayers.at( position.z )->GetTile( position );

    else
        return 0;
};

// Return the biome tile ID, or 0 if there is none
sf::Int32 Cell::GetBiome( sf::Vector3i position )
{
    if( LayerExists( position ) )
        return myLayers.at( position.z )->GetBiome();

    else
        return 0;
};

// Return the biome tile ID, or 0 if there is none
sf::Int32 Cell::GetRegion( sf::Vector3i position )
{
    if( LayerExists( position ) )
        return myLayers.at( position.z )->GetRegion();

    else
        return 0;
};

void Cell::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    using namespace Settings;

    // Apply the transform
    states.transform *= getTransform();

    // Make sure we don't draw other layers
    sf::Int32 z = globalPosition.z;

    // Round to the nearest layer base
    if( z < 0 and (z % drawLayers) != 0 )
        z = z - ( drawLayers + (z % drawLayers) );

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
    LayerPtr newLayer = std::make_unique<Layer>( myID, sf::Vector3i( myPosition.x, myPosition.y, position.z ), tileset );

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

