#include "Map.hpp"
#include <math.h>

bool saveChanges = false;

Map::Map()
{
    // start sqlite
    std::string dbName = DB_NAME;
    if( sqlite3_open_v2( dbName.c_str(), &db, SQLITE_OPEN_READWRITE, NULL ) )
    {
        std::cout << "Failed to open db!" << sqlite3_extended_errcode( db ) << std::endl;
    }

    // Make the tiles
    sf::Image temp;
    temp.loadFromFile( TILESET_FILE );
    temp.createMaskFromColor( temp.getPixel( temp.getSize().x-1, temp.getSize().y-1 ) );

    texture = new sf::Texture;
    texture->loadFromImage( temp );

    // Set up the new cell ID counter
    // Make a SQL query
    sqlite3_stmt *statement;
    std::string query = "SELECT MAX(ID) FROM Cells";

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }

    // Grab the ID
    err = sqlite3_step( statement );
    if( err == SQLITE_ROW )
        maxCellID = sqlite3_column_int( statement, 0 );

    else
        // Cell not found
        std::cout << "Error grabbing max cell ID." << std::endl;
};

// Set a particular tile in a cell
void Map::SetCellTile( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure it exists!
    if( GetCell( position ) != 0 )
        myCells[ GetCell( position ) ]->SetTile( ConvertToTilePosition( position ), tile );
};

void Map::SetCellBiome( sf::Vector3i position, sf::Uint32 tile )
{
    // Make sure it exists!
    if( GetCell( position ) != 0 )
        myCells[ GetCell( position ) ]->SetBiome( position, tile );
};

// Make sure we have all the proper areas loaded
void Map::UpdateLoadedCells( sf::Vector3i position )
{
    // Which cells need to be loaded?
    // What cell is this in?
    sf::Vector3i centerCell;
    centerCell.x = ( position.x / TILE_SIZE ) / CELL_WIDTH;
    centerCell.y = ( position.y / TILE_SIZE ) / CELL_HEIGHT;

    // How many cells do we need loaded?
    sf::Int32 horizCellsNeeded = ceil((float)WINDOW_W / ( CELL_WIDTH  * TILE_SIZE )) + MAP_BUFFER;
    sf::Int32 vertiCellsNeeded = ceil((float)WINDOW_H / ( CELL_HEIGHT * TILE_SIZE )) + MAP_BUFFER;

    // Make sure the numbers are even to make things nice
    if( horizCellsNeeded % 2 != 0 ) horizCellsNeeded++;
    if( vertiCellsNeeded % 2 != 0 ) vertiCellsNeeded++;

    // What's the upper left cell ID?
    // To calculate, find the cell positions of the upper and left cells needed

    // Check horizontal bounds
    sf::Vector3i currentCell;
    currentCell.z = 0;

    if( (centerCell.x - (horizCellsNeeded/2)) < 0  )
        currentCell.x = 0;
    else if( centerCell.x + (horizCellsNeeded/2) > (MAP_WIDTH / CELL_WIDTH) )
        currentCell.x = (MAP_WIDTH / CELL_WIDTH) - horizCellsNeeded;
    else
        currentCell.x = centerCell.x - (horizCellsNeeded/2);

    // Check vertical bounds
    if( (centerCell.y - (vertiCellsNeeded/2)) < 0  )
        currentCell.y = 0;
    else if( centerCell.y + (vertiCellsNeeded/2) > (MAP_HEIGHT / CELL_HEIGHT) )
        currentCell.y = (MAP_HEIGHT / CELL_HEIGHT) - vertiCellsNeeded;
    else
        currentCell.y = centerCell.y - (vertiCellsNeeded/2);

    // Make a list of all the cell IDs we need to load
    std::vector< sf::Vector3i > cellsNeeded;

    // Loop through the rows
    for( sf::Int32 i = 0; i < vertiCellsNeeded; i++ )
    {
        // Loop through the columns
        for( sf::Int32 j = 0; j < horizCellsNeeded; j++ )
        {
            cellsNeeded.push_back( currentCell );
            currentCell.x++;
        }

        // Reset horizontal and increase vertical
        currentCell.x -= horizCellsNeeded;
        currentCell.y++;
    }

    // Now that we know which cells we need, load any that need to be

    // Loop through the rows
    for( auto it = cellsNeeded.begin(); it != cellsNeeded.end(); ++it )
    {
        bool toLoad = true;

        // Is it already loaded?
        for( auto it2 = myCells.begin(); it2 != myCells.end(); ++it2 )
            if( it2->second->GetPosition() == *it )
            {
                toLoad = false;
                break;
            }

        // Cell not found in the map, load it!
        if( toLoad )
        {
            sf::Uint32 cell = LoadCell( *it );

            // If it existed already, load its tile layer
            if( cell != 0 )
                for( auto layer = *it; layer.z < (DRAW_LAYERS + (*it).z); layer.z++ )
                    LoadCellTileLayer( cell, layer );
            // If it doesn't exist, make a new one!
            else
            {
                maxCellID++;
                CreateCell( *it, maxCellID );
            }
        }
    }

    // If it's already loaded, do we still need it?
    std::vector< sf::Uint32 > toRemove;
    for( auto it = myCells.begin(); it != myCells.end(); ++it )
    {
        bool isNeeded = false;

        for( auto it2 = cellsNeeded.begin(); it2 != cellsNeeded.end(); ++it2 )
            if( it->second->GetPosition() == *it2 )
                isNeeded = true;

        if( !isNeeded )
            toRemove.push_back( it->first );
    }


    // Remove the old cells
    for( auto it = toRemove.begin(); it != toRemove.end(); ++it )
    {
        SaveCell( *it );
        myCells.erase( *it );
    }
};


sf::Uint32 Map::LoadCell( sf::Vector3i position )
{
    // Make a SQL query
    std::string query = "SELECT ID FROM Cells WHERE X = ? AND Y = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, position.x );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }
    err = sqlite3_bind_int( statement, 2, position.y );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }

    // Grab the ID
    err = sqlite3_step( statement );
    sf::Uint32 cellID;
    if( err == SQLITE_ROW )
    {
        cellID = sqlite3_column_int( statement, 0 );
    }
    else if( err == SQLITE_DONE )
    {
        // Cell not found
        return 0;
    }
    else
    {
        std::cout << "Step failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }


    // Now grab all the Biomes and Areas
    // Make a SQL query
    query = "SELECT Z, Biome, Area FROM Biomes WHERE Cell = ?";

    err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, cellID );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }

    // Looks good! Step it
    bool stop = false;
    std::map< sf::Int32, sf::Uint32 > biomes, areas;
    while( stop == false )
    {
        // Make sure we still have a row to read
        err = sqlite3_step( statement );
        if( err == SQLITE_DONE )
            stop = true;
        else if( err != SQLITE_DONE and err != SQLITE_ROW )
        {
            std::cout << "Step failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return 0;
        }
        else
        {
            // Everything is good to go! Make a new biome entry
            sf::Int32 zPos;
            sf::Uint32 biome, area;
            zPos  = sqlite3_column_int( statement, 0 );
            biome = sqlite3_column_int( statement, 1 );
            area  = sqlite3_column_int( statement, 2 );
            //                                                     Z   Value
            biomes. insert( std::pair< sf::Int32, sf::Uint32 > ( zPos, biome ) );
            areas.  insert( std::pair< sf::Int32, sf::Uint32 > ( zPos, area  ) );
        }
    }

    sqlite3_finalize( statement );

    CreateCell( position, cellID );
    myCells[cellID]->SetProperties( biomes, areas );

    return cellID;
};

void Map::CreateCell( sf::Vector3i position, sf::Uint32 cell )
{
    myCells.insert( std::pair< sf::Uint32, std::shared_ptr<Cell> > ( cell, std::shared_ptr<Cell>(new Cell( cell, position, texture )) ) );
};


void Map::LoadCellTileLayer( sf::Uint32 cell, sf::Vector3i position )
{
    std::vector< std::shared_ptr<Tile> > tiles;

    // Make a SQL query
    std::string query = "SELECT X, Y, Tile FROM Tiles WHERE Cell = ? AND Z = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, cell );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }
    err = sqlite3_bind_int( statement, 2, position.z );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return;
    }

    // Looks good! Step it
    bool stop = false;
    while( stop == false )
    {
        // Make sure we still have a row to read
        err = sqlite3_step( statement );
        if( err == SQLITE_DONE )
            stop = true;
        else if( err != SQLITE_DONE and err != SQLITE_ROW )
        {
            std::cout << "Step failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
            return;
        }
        else
        {
            // Everything is good to go! Make a new tile
            sf::Vector3i tilePos;
            tilePos.x = sqlite3_column_int( statement, 0 );
            tilePos.y = sqlite3_column_int( statement, 1 );
            tilePos.z = position.z;

            sf::Uint32 tileID = sqlite3_column_int( statement, 2 );

            // Make sure the tile isn't outside cell bounds
            if( tilePos.x >= 0 and tilePos.x < CELL_WIDTH and tilePos.y >= 0 and tilePos.y < CELL_HEIGHT )
                tiles.push_back( std::shared_ptr<Tile> ( new Tile( tilePos, tileID, 255 ) ) );
        }
    }

    myCells[cell]->InitTileLayer( tiles, position );

    sqlite3_finalize( statement );

    return;
};


// Save a cell to the database
void Map::SaveCell( sf::Uint32 cell )
{
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
    }
};


void Map::DrawMap( sf::RenderWindow* window, sf::Vector3i position )
{
    for( auto it = myCells.begin(); it != myCells.end(); ++it )
        it->second->DrawCell( window, position );
};

sf::Uint32 Map::GetCell( sf::Vector3i position )
{
    // Normalize the position
    position = ConvertToCellPosition( position );

    for( auto it = myCells.begin(); it != myCells.end(); ++it )
        if( (*it).second->GetPosition() == position )
            return (*it).first;

    return 0;
};

// Convert a world position to a cell position
sf::Vector3i Map::ConvertToCellPosition( sf::Vector3i position )
{
    // Normalize the position
    position.z = 0;

    position.x = ( position.x / TILE_SIZE ) / CELL_WIDTH;
    position.y = ( position.y / TILE_SIZE ) / CELL_HEIGHT;

    return position;
};

// Convert a world position to a tile position (relative to its cell)
sf::Vector3i Map::ConvertToTilePosition( sf::Vector3i position )
{
    // Normalize the position
    position.x = ( position.x / TILE_SIZE ) % CELL_WIDTH;
    position.y = ( position.y / TILE_SIZE ) % CELL_HEIGHT;

    return position;
};

Map::~Map()
{
    // Save all the open cells
    for( auto it = myCells.begin(); it != myCells.end(); ++it )
        SaveCell( (*it).first );

    delete texture;
    sqlite3_close( db );
};
