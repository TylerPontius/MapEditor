#include "Map.hpp"
#include <math.h>

bool saveChanges = false;

Map::Map() : db( mapFile )
{
    // Make the tiles
    sf::Image temp;
    temp.loadFromFile( tilesetFile );
    temp.createMaskFromColor( temp.getPixel( temp.getSize().x-1, temp.getSize().y-1 ) );

    tileset = new sf::Texture;
    tileset->loadFromImage( temp );

    // Grab max cell ID
    try
    {
        maxCellID = db.execAndGet( "SELECT MAX(ID) FROM Cells" ).getInt();
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
};

// Set a particular tile
void Map::SetTile( sf::Vector3i position, sf::Uint32 tile )
{
    // Find the cell the tile is in
    sf::Vector3i cellPos = position;
    ConvertToCellPosition( cellPos );
    std::cout << "Looking for cell at " << cellPos.x << ", " << cellPos.y << ", " << cellPos.z << "\n";


    // Make sure it exists!
    if( CellExists( cellPos ) )
    {
        ConvertToTilePosition( position );
        GetCell( cellPos ).SetTile( position, tile, tileset );
        std::cout << "Setting tile " << tile << " at " << position.x << ", " << position.y << ", " << position.z << "\n";
    }

};

void Map::SetBiome( sf::Vector3i position, sf::Uint32 tile )
{
    // Now find the position inside the cell
    ConvertToCellPosition( position );

    // Make sure it exists!
    if( CellExists( position ) )
        GetCell( position ).SetBiome( position, tile, tileset );
};

// Make sure we have all the proper areas loaded
void Map::UpdateLoadedCells( sf::Vector3i position )
{
    // Which cells need to be loaded?

    // First, find what cell we are in
    // This lets us work the rest of the math in terms of cell IDs and cell positions
    sf::Vector3i centerCell = position;
    ConvertToCellPosition( centerCell );

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
    if( int(centerCell.x - (horizCellsNeeded/2)) < 0  )
        currentCell.x = 0;
    else if( centerCell.x + (horizCellsNeeded/2) > (mapWidth / cellWidth) )
        currentCell.x = (mapWidth / cellWidth) - horizCellsNeeded;
    else
        currentCell.x = centerCell.x - (horizCellsNeeded/2);

    // Check vertical bounds
    if( int(centerCell.y - (vertiCellsNeeded/2)) < 0  )
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
        // Cell not found in the map, load it!
        if( !CellExists( it ) )
            LoadCell( it );
    }

    // Clean up any unused cells
    // If it's already loaded, do we still need it?
    std::vector< sf::Uint32 > toRemove;
    for( auto& it : myCells )
    {
        bool isNeeded = false;

        for( auto& it2 : cellsNeeded )
            if( it.second.GetPosition() == it2 )
                isNeeded = true;

        if( !isNeeded )
            toRemove.push_back( it.second.GetID() );
    }


    // Remove the old cells
    for( auto& it : toRemove )
        RemoveCell( it );
};


void Map::LoadCell( sf::Vector3i position )
{
    // Make sure it's not already loaded
    if( CellExists( position ) )
        return;

    sf::Uint32 cellID;
    bool cellLoaded = false;

    try
    {
        // Compile a SQL query, containing one parameter (index 1)
        SQLite::Statement query( db, "SELECT ID FROM Cells WHERE X = ? AND Y = ?" );

        // Bind our request
        query.bind( 1, position.x );
        query.bind( 2, position.y );

        if( query.executeStep() )
        {
            // We got some data!
            if( query.getColumn( 0 ).size() > 0 and query.getColumn( 0 ).isInteger() )
            {
                cellID = query.getColumn( 0 ).getInt();
                cellLoaded = true;
            }
        }
        else
            cellID = ++maxCellID;
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }

    // Add the cell to the map
    Cell& newCell = CreateCell( position, cellID );

    // Load the cell layers from the database
    if( cellLoaded )
    {
        try
        {
            // Make a SQL query
            SQLite::Statement query( db, "SELECT Z, Biome, Air, Region FROM Layers WHERE Cell = ?" );

            // Bind our request
            query.bind( 1, (int)cellID );

            while( query.executeStep() )
            {
                // Everything is good to go! Make a new layer
                sf::Vector3i layerPos = position;
                layerPos.z        = query.getColumn( 0 ).getInt();
                sf::Uint32 biome  = query.getColumn( 1 ).getInt();
                sf::Uint32 air    = query.getColumn( 2 ).getInt();
                sf::Uint32 region = query.getColumn( 3 ).getInt();

                newCell.SetBiome ( layerPos, biome, tileset );
                newCell.SetAir   ( layerPos, air );
                newCell.SetRegion( layerPos, region );
            }
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }
}

Cell& Map::CreateCell( sf::Vector3i position, sf::Uint32 cellID )
{
    myCells.emplace( cellID, Cell{ cellID, position } );

    Cell& cell = GetCell( position );
    return cell;
};


// Save a cell to the database
void Map::RemoveCell( sf::Uint32 cellID )
{/*
    if( myCells.find( cellID ) == myCells.end() ) return;

    Cell& theCell = myCells.at( cellID );

    if( saveChanges )
    {
        // Create the cell entry if it doesn't already exist
        {
            // Make a SQL query
            std::string query = "SELECT * FROM Cells WHERE ID = ?";
            sqlite3_stmt *statement;

            int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
            if( err != SQLITE_OK )
            {
                std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 1, cellID );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            // Make sure we still have a row to read
            err = sqlite3_step( statement );

            // If we need to, make the cell entry
            if( err != SQLITE_ROW )
            {
                // Make a SQL query
                query = "INSERT INTO Cells VALUES (?, ?, ?)";

                err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
                if( err != SQLITE_OK )
                {
                    std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 1, cellID );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 2, theCell.GetPosition().x );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                    return;
                }

                err = sqlite3_bind_int( statement, 3, theCell.GetPosition().y );
                if( err != SQLITE_OK )
                {
                    std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                    return;
                }

                err = sqlite3_step( statement );
                if( err != SQLITE_OK and err != SQLITE_DONE )
                {
                    std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                    return;
                }
            }

            sqlite3_finalize( statement );
        }

        // Remove all biome and tile entries
        {
            // Make a SQL query
            std::string query = "DELETE FROM Layers WHERE Cell = ?";
            sqlite3_stmt *statement;

            int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
            if( err != SQLITE_OK )
            {
                std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 1, cellID );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            err = sqlite3_step( statement );
            if( err != SQLITE_OK and err != SQLITE_DONE )
            {
                std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            // Make a SQL query
            query = "DELETE FROM Tiles WHERE Cell = ?";

            err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
            if( err != SQLITE_OK )
            {
                std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            err = sqlite3_bind_int( statement, 1, cellID );
            if( err != SQLITE_OK )
            {
                std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                return;
            }

            err = sqlite3_step( statement );
            if( err != SQLITE_OK and err != SQLITE_DONE )
            {
                std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
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

                    int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 1, cell );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 2, z );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 3, biome );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                        return;
                    }

                    err = sqlite3_bind_int( statement, 4, area );
                    if( err != SQLITE_OK )
                    {
                        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                        return;
                    }

                    err = sqlite3_step( statement );
                    if( err != SQLITE_OK and err != SQLITE_DONE )
                    {
                        std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
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

                        int err = sqlite3_prepare_v2( mapDB, query.c_str(), -1, &statement, NULL );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Query failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_bind_int( statement, 1, cell );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_bind_int( statement, 2, position.x );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_bind_int( statement, 3, position.y );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_bind_int( statement, 4, position.z );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_bind_int( statement, 5, tile );
                        if( err != SQLITE_OK )
                        {
                            std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        err = sqlite3_step( statement );
                        if( err != SQLITE_OK and err != SQLITE_DONE )
                        {
                            std::cout << "Execute failed! " << err << " " << sqlite3_errmsg( mapDB ) << std::endl;
                            return;
                        }

                        sqlite3_finalize( statement );
                    }
                }
            }
        }
    }


    myCells.erase( cellID );*/
};


void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for( auto& it : myCells )
        target.draw( it.second, states );
};

Cell& Map::GetCell( sf::Vector3i position )
{
    for( auto& it : myCells )
        if( it.second.GetPosition() == position )
            return it.second;
};

bool Map::CellExists( sf::Vector3i position )
{
    // We don't care about z when looking up cells
    position.z = 0;

    for( auto& it : myCells )
        if( it.second.GetPosition() == position )
            return true;

    return false;
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
            RemoveCell( it.second.GetID() );

    delete tileset;
};
