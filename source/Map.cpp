#include "Map.hpp"
#include <math.h>

bool saveChanges = false;

Map::Map()
{
    // Make the tiles
    sf::Image temp;
    temp.loadFromFile( tilesetFile );
    temp.createMaskFromColor( temp.getPixel( temp.getSize().x-1, temp.getSize().y-1 ) );

    tileset = new sf::Texture;
    tileset->loadFromImage( temp );

    // start sqlite
    if( sqlite3_open_v2( mapFile.c_str(), &mapDB, SQLITE_OPEN_READWRITE, NULL ) )
    {
        std::cout << "Failed to open db!" << sqlite3_extended_errcode( mapDB ) << std::endl;
    }

    // Set up the new cell ID counter
    // Make a SQL query
    sqlite3_stmt *statement;
    std::string query = "SELECT MAX(ID) FROM Cells";

    int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;

    // Grab the ID
    err = sqlite3_step( statement );
    if( err == SQLITE_ROW )
        maxCellID = sqlite3_column_int( statement, 0 );

    else if( err == SQLITE_DONE )
        maxCellID = 0;

    else
        std::cout << "Error grabbing max cell ID." << std::endl;
};

// Set a particular tile
void Map::SetTile( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure it exists!
    if( GetCell( position ) != 0 )
        myCells.at( GetCell( position ) ).SetTile( position, tile, tileset );
};

void Map::SetBiome( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure it exists!
    if( GetCell( position ) != 0 )
        myCells.at( GetCell( position ) ).SetBiome( position, tile, tileset );
};

// Make sure we have all the proper areas loaded
void Map::UpdateLoadedCells( sf::Vector3i position )
{/*
    // Which cells need to be loaded?

    // First, find what cell we are in
    // This lets us work the rest of the math in terms
    // of cell IDs
    sf::Vector3i centerCell;
    centerCell.x = ( position.x / tileSize ) / cellWidth;
    centerCell.y = ( position.y / tileSize ) / cellHeight;

    // How many cells do we need loaded?
    // This is equal to the number of cells visible plus a buffer
    sf::Uint32 horizCellsNeeded = ceil( windowWidth  / ( cellWidth  * tileSize )) + mapBuffer;
    sf::Uint32 vertiCellsNeeded = ceil( windowHeight / ( cellHeight * tileSize )) + mapBuffer;

    // Make sure the numbers are even to make things nice
    if( horizCellsNeeded % 2 != 0 ) horizCellsNeeded++;
    if( vertiCellsNeeded % 2 != 0 ) vertiCellsNeeded++;

    // What's the upper left cell ID?
    // To calculate, find the cell positions of the upper and left cells needed

    sf::Vector3i currentCell;
    currentCell.z = position.z;

    // Check horizontal bounds
    if( (centerCell.x - (horizCellsNeeded/2)) < 0  )
        currentCell.x = 0;
    else if( centerCell.x + (horizCellsNeeded/2) > (mapWidth / cellWidth) )
        currentCell.x = (mapWidth / cellWidth) - horizCellsNeeded;
    else
        currentCell.x = centerCell.x - (horizCellsNeeded/2);

    // Check vertical bounds
    if( (centerCell.y - (vertiCellsNeeded/2)) < 0  )
        currentCell.y = 0;
    else if( centerCell.y + (vertiCellsNeeded/2) > (mapWidth / cellHeight) )
        currentCell.y = (mapWidth / cellHeight) - vertiCellsNeeded;
    else
        currentCell.y = centerCell.y - (vertiCellsNeeded/2);

    // Make a list of all the cell IDs we need to load
    // Now that currentCell is set to the upper left cell of the screen,
    // we can populate the list in typewriter fashion:

    std::vector< sf::Vector3i > cellsNeeded;

    // Loop through the rows
    for( sf::Uint32 i = 0; i < vertiCellsNeeded; i++ )
    {
        // Loop through the columns
        for( sf::Uint32 j = 0; j < horizCellsNeeded; j++ )
        {
            cellsNeeded.push_back( currentCell );
            currentCell.x++;
        }

        // Reset horizontal and increase vertical
        currentCell.x -= horizCellsNeeded;
        currentCell.y++;
    }

    // Now that we know which cells we need, load any we need

    // Loop through the rows
    for( auto& it : cellsNeeded )
    {
        bool toLoad = true;

        // Is it already loaded?
        for( auto& it2 : myCells )
            if( it2.second->GetPosition() == it )
            {
                toLoad = false;
                break;
            }

        // Cell not found in the map, load it!
        if( toLoad )
        {
            sf::Uint32 cell = LoadCell( it );

            // If it existed already, load its tile layer
            if( cell != 0 )
                for( auto layer = it; layer.z < (drawLayers + it.z); layer.z++ )
                    LoadCellTileLayer( cell, layer );
            // If it doesn't exist, make a new one!
            else
            {
                maxCellID++;
                CreateCell( it, maxCellID );
            }
        }
    }

    // Clean up any unused cells
    // If it's already loaded, do we still need it?
    std::vector< sf::Uint32 > toRemove;
    for( auto& it : myCells )
    {
        bool isNeeded = false;

        for( auto& it2 : cellsNeeded )
            if( it.second->GetPosition() == it2 )
                isNeeded = true;

        if( !isNeeded )
            toRemove.push_back( it.first );
    }


    // Remove the old cells
    for( auto& it : toRemove )
    {
        SaveCell( it );
        myCells.erase( it );
    }*/
};


void Map::LoadCell( sf::Vector3i position )
{
    // Make sure it's not already loaded
    if( myCells.find( position.z ) != myCells.end() )
        return;

    // Make a SQL query
    std::string query = "SELECT ID FROM Cells WHERE X = ? AND Y = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
        return;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, position.x );
    err = sqlite3_bind_int( statement, 2, position.y );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
        return;
    }

    // Looks good! Step it
    err = sqlite3_step( statement );

    sf::Uint32 cellID;

    // We got some data!
    if( err == SQLITE_ROW )
        cellID = sqlite3_column_int( statement, 0 );

    // Cell doesn't exist! Let's create it.
    else if( err == SQLITE_DONE )
        cellID = ++maxCellID;

    // Database error!
    else
    {
        std::cout << "Step failed!" << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
        return;
    }

    CreateCell( position, cellID );
}

void Map::CreateCell( sf::Vector3i position, sf::Uint32 cellID )
{
    if( myCells.find( position.z ) == myCells.end() )
    {
        myCells.emplace( cellID, Cell{ cellID, position, tileset, mapDB } );

        Cell& cell = myCells.at( cellID );

        // Set the tile info
        cell.setOrigin( 0.f, 0.f );
        cell.setPosition( position.x * tileSize * cellWidth, position.y  * tileSize * cellHeight );
    }
};


// Save a cell to the database
void Map::SaveCell( sf::Uint32 cell )
{/*
    if( saveChanges == false ) return;

    // Create the cell entry if it doesn't already exist
    {
        // Make a SQL query
        std::string query = "SELECT * FROM Cells WHERE ID = ?";
        sqlite3_stmt *statement;

        int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
        if( err != SQLITE_OK )
        {
            std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        err = sqlite3_bind_int( statement, 1, cell );
        if( err != SQLITE_OK )
        {
            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        // Make sure we still have a row to read
        err = sqlite3_step( statement );
        bool create = true;
        if( err == SQLITE_ROW )
            create = false;

        // If we need to, make the cell entry
        if( create )
        {
            // Make a SQL query
            query = "INSERT INTO Cells VALUES (?, ?, ?)";

            int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
            if( err != SQLITE_OK )
            {
                std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 1, cell );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 2, myCells[ cell ]->GetPosition().x );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 3, myCells[ cell ]->GetPosition().y );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                return;
            }

            err = sqlite3_step( statement );
            if( err != SQLITE_OK and err != SQLITE_DONE )
            {
                std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                return;
            }
        }

        sqlite3_finalize( statement );
    }

    // Remove all biome and tile entries
    {
        // Make a SQL query
        std::string query = "DELETE FROM Biomes WHERE Cell = ?";
        sqlite3_stmt *statement;

        int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
        if( err != SQLITE_OK )
        {
            std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        err = sqlite3_bind_int( statement, 1, cell );
        if( err != SQLITE_OK )
        {
            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        err = sqlite3_step( statement );
        if( err != SQLITE_OK and err != SQLITE_DONE )
        {
            std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        // Make a SQL query
        query = "DELETE FROM Tiles WHERE Cell = ?";

        err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
        if( err != SQLITE_OK )
        {
            std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        err = sqlite3_bind_int( statement, 1, cell );
        if( err != SQLITE_OK )
        {
            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        err = sqlite3_step( statement );
        if( err != SQLITE_OK and err != SQLITE_DONE )
        {
            std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }

        sqlite3_finalize( statement );
    }

    // Now, save all biomes
    {
        // Download the info from the cell
        auto biomes = myCells[ cell ]->GetBiomes();
        auto areas  = myCells[ cell ]->GetAreas ();

        // Loop through the lists
        for( auto it = biomes.begin(); it != biomes.end(); it++ )
        {
            // Grab the current info
            auto z = (*it).first;
            auto biome = (*it).second;
            sf::Uint32 area;

            if( areas.find( z ) == areas.end() )
                area = 0;
            else
                area = areas[ z ];

            if( biome != 0 )
            {
                // Make a SQL query
                std::string query = "INSERT INTO Biomes VALUES (?, ?, ?, ?)";
                sqlite3_stmt *statement;

                int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
                if( err != SQLITE_OK )
                {
                    std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 1, cell );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 2, z );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 3, biome );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 4, area );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                err = sqlite3_step( statement );
                if( err != SQLITE_OK and err != SQLITE_DONE )
                {
                    std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                    return;
                }

                sqlite3_finalize( statement );
            }
        }
    }

    // Now, save all tiles
    {
        // Download the info from the cell
        auto tiles = myCells[ cell ]->GetTiles();

        // Loop through the lists
        for( auto it = tiles.begin(); it != tiles.end(); it++ )
        {
            // Loop through the tile layers
            for( auto it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++ )
            {
                // Grab the current info
                sf::Vector3i position = (*it2)->GetPosition();
                auto tile = (*it2)->GetTile();

                // Get the current biome for the layer so we know not to save it
                auto biomes = myCells[ cell ]->GetBiomes();
                sf::Uint32 biome = 0;

                // If there's a biome on this layer, record it
                if( biomes.find( position.z ) != biomes.end() )
                    biome = biomes[ position.z ];

                if( tile != biome )
                {
                    // Make a SQL query
                    std::string query = "INSERT INTO Tiles VALUES (?, ?, ?, ?, ?)";
                    sqlite3_stmt *statement;

                    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 1, cell );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 2, position.x );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 3, position.y );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 4, position.z );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 5, tile );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    err = sqlite3_step( statement );
                    if( err != SQLITE_OK and err != SQLITE_DONE )
                    {
                        std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
                        return;
                    }

                    sqlite3_finalize( statement );
                }
            }
        }
    }*/
};


void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for( auto& it : myCells )
        target.draw( it.second, states );
};

sf::Uint32 Map::GetCell( sf::Vector3i position )
{
    for( auto& it : myCells )
        if( it.second.GetPosition() == position )
            return it.second.GetID();

    return 0;
};

// Convert a world position to a cell position
void Map::ConvertToCellPosition( sf::Vector3i& position )
{
    position.x = ( position.x / tileSize ) / cellWidth;
    position.y = ( position.y / tileSize ) / cellHeight;
};

// Convert a world position to a tile position (relative to its cell)
void Map::ConvertToTilePosition( sf::Vector3i& position )
{
    position.x = ( position.x / tileSize ) % cellWidth;
    position.y = ( position.y / tileSize ) % cellHeight;
};

Map::~Map()
{
    // Save all the open cells
    if( saveChanges )
        for( auto& it : myCells )
            SaveCell( it.second.GetID() );

    delete tileset;
    sqlite3_close( mapDB );
};
