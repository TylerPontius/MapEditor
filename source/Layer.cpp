#include "Layer.hpp"
#include "Map.hpp"

Layer::Layer( sf::Int32 cell, sf::Vector3i position, sf::Texture* tileset ) : myCell( cell ), myPosition( position ), myTileset( tileset )
{
    // Resize the vertex array to fit the cell size
    myVertices.setPrimitiveType( sf::Quads );
    myVertices.resize( Settings::cellWidth * Settings::cellHeight * 4 );
};

Layer::~Layer()
{
    if( Settings::saveChanges ) Save();
};

void Layer::Save()
{
    // Create the layer entry if it doesn't already exist
    bool exists = false;

    try
    {
        SQLite::Statement query( Map::db, "SELECT * FROM Layers WHERE Cell = ? AND Z = ?" );

        // Bind our request
        query.bind( 1, (int)myCell );
        query.bind( 2, (int)myPosition.z );

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
            SQLite::Statement query( Map::db, "INSERT INTO Layers VALUES( ?, ?, ?, ?, ? )" );

            // Bind our request
            query.bind( 1, (int)myCell );
            query.bind( 2, (int)myPosition.z );
            query.bind( 3, (int)myBiome );
            query.bind( 4, (int)myAir );
            query.bind( 5, (int)myRegion );

            query.exec();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }

    // Looks like we already exist. Update our entry
    else
    {
        try
        {
            SQLite::Statement query( Map::db, "UPDATE Layers SET Biome = ?, Air = ?, Region = ? WHERE Cell = ? AND Z = ?" );

            // Bind our request
            query.bind( 1, (int)myBiome );
            query.bind( 2, (int)myAir );
            query.bind( 3, (int)myRegion );

            query.bind( 4, (int)myCell );
            query.bind( 5, (int)myPosition.z );

            query.exec();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }


    // Remove tiles
    try
    {
        // Make a SQL query
        SQLite::Statement query( Map::db, "DELETE FROM Tiles WHERE Cell = ? and Z = ?" );

        // Bind our request
        query.bind( 1, (int)myCell );
        query.bind( 2, (int)myPosition.z );

        query.exec();
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }


    // Save all tiles
    for( Tile& t : myTiles )
    {
        if( t.myTileID != myBiome )
        {
            try
            {
                // Make a SQL query
                SQLite::Statement query( Map::db, "INSERT INTO Tiles VALUES( ?, ?, ?, ?, ?, ? )" );

                // Bind our request
                query.bind( 1, (int)myCell );
                query.bind( 2, (int)t.myPosition.x );
                query.bind( 3, (int)t.myPosition.y );
                query.bind( 4, (int)t.myPosition.z );
                query.bind( 5, (int)t.myTileID );
                query.bind( 6, (int)t.myAlpha );

                query.exec();
            }
            catch (std::exception& e)
            {
                std::cout << "exception: " << e.what() << std::endl;
            }
        }
    }

};

void Layer::Load()
{
    try
    {
        // Make a SQL query
        SQLite::Statement query( Map::db, "SELECT X, Y, Tile, Alpha FROM Tiles WHERE Cell = ? AND Z = ?" );

        // Bind our request
        query.bind( 1, (int)myCell );
        query.bind( 2, (int)myPosition.z );

        while( query.executeStep() )
        {
            // Everything is good to go! Make a new layer
            sf::Vector3i tilePos;
            tilePos.x       = query.getColumn( 0 ).getInt();
            tilePos.y       = query.getColumn( 1 ).getInt();
            tilePos.z       = myPosition.z;
            sf::Int32 tile = query.getColumn( 2 ).getInt();

            SetTile( tilePos, tile );
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
};

// Set every tile in the cell to the biome
void Layer::SetBiome( sf::Int32 biome )
{
    Clear();

    myBiome = biome;

    for( sf::Int32 i = 0; i < Settings::cellWidth; i++ )
        for( sf::Int32 j = 0; j < Settings::cellHeight; j++ )
            SetTile( sf::Vector3i( i, j, myPosition.z ), biome );
}

// Add a tile to the layer
void Layer::SetTile( sf::Vector3i position, sf::Int32 tile )
{
    using namespace Settings;

    // First, remove a tile if one already occupies the same position
    // Use a lambda and std::remove_if
    myTiles.erase(  std::remove_if( myTiles.begin(), myTiles.end(),
                    [=] ( auto tile ) { return (tile.myPosition == position); } ),
                    myTiles.end() );

    // Find the relative position
    sf::Int32 x = position.x ;
    sf::Int32 y = position.y ;

    // Get a pointer to the current tile's quad and delete its info
    sf::Vertex* quad = &myVertices[ (x + (y * cellWidth)) * 4 ];

    for( int i = 0; i < 4; i++ )
    {
        quad[i].position = sf::Vector2f( 0.f, 0.f);
        quad[i].texCoords = sf::Vector2f( 0.f, 0.f);
    }

    if( tile == 0 ) return;

    // Add the tile and start computing
    myTiles.emplace_back ( position, tile, 255 );

    // Define its 4 corners
    quad[0].position = sf::Vector2f(x * tileSize, y * tileSize);
    quad[1].position = sf::Vector2f((x + 1) * tileSize, y * tileSize);
    quad[2].position = sf::Vector2f((x + 1) * tileSize, (y + 1) * tileSize);
    quad[3].position = sf::Vector2f(x * tileSize, (y + 1) * tileSize);

    // Find its position in the tileset texture
    int tu = tile % (myTileset->getSize().x / tileSize );
    int tv = tile / (myTileset->getSize().x / tileSize );

    // Define its 4 texture coordinates
    quad[0].texCoords = sf::Vector2f(tu * tileSize, tv * tileSize);
    quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize, tv * tileSize);
    quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize, (tv + 1) * tileSize);
    quad[3].texCoords = sf::Vector2f(tu * tileSize, (tv + 1) * tileSize);
};

sf::Int32 Layer::GetTile( sf::Vector3i position )
{
    for( auto& tile : myTiles )
        if( tile.myPosition == position )
            return tile.myTileID;

    return 0;
}

// Remove all tiles and vertices
void Layer::Clear()
{
    myTiles.clear();
    myVertices.clear();
    myVertices.resize( Settings::cellWidth * Settings::cellHeight * 4 );
};

void Layer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = myTileset;

    // draw the vertex array
    target.draw( myVertices, states);
};
