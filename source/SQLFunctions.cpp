#include "SQLFunctions.hpp"
#include "Map.hpp"
/*
SQLFunctions::SQLFunctions()
{
    // start sqlite
    std::string dbName = dbName;
    if( sqlite3_open_v2( dbName.c_str(), &db, SQLITE_OPEN_READWRITE, NULL ) )
    {
        std::cout << "Failed to open db!" << sqlite3_extended_errcode( db ) << std::endl;
    }
};

Cell* SQLFunctions::LoadCell( sf::Vector3i position )
{
    // Make a SQL query
    std::string query = "SELECT ID FROM Cells WHERE X = ? AND Y = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return nullptr;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, position.x );
    err = sqlite3_bind_int( statement, 2, position.y );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return nullptr;
    }

    // Looks good! Step it
    err = sqlite3_step( statement );
    if( err != SQLITE_DONE and err != SQLITE_ROW )
    {
        std::cout << "Step failed!" << err << " " << sqlite3_errmsg( db ) << std::endl;
        return nullptr;
    }

    // Grab our value
    sf::Int32 id = sqlite3_column_int( statement, 0 );

    sqlite3_finalize( statement );

    return new Cell( id, position );
};


SQLFunctions::LoadLayers( sf::Int32 cell, sf::Int32 z )
{
    std::vector< std::shared_ptr<Tile> > tiles;

    // Make a SQL query
    std::string query = "SELECT X, Y, Tile FROM Tiles WHERE Cell = ? AND Z = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return tiles;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, cell );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return tiles;
    }
    err = sqlite3_bind_int( statement, 2, z );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return tiles;
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
            std::cout << "Step failed!" << err << " " << sqlite3_errmsg( db ) << std::endl;
            return tiles;
        }
        else
        {
            // Everything is good to go! Make a new tile
            sf::Int32 ret[3];
            for( int i = 0; i < 3; i++ )
                ret[i] = sqlite3_column_int( statement, i );

            std::shared_ptr<Tile> newTile( new Tile( ret[2], ret[0], ret[1], z, 255 ) );
            tiles.push_back( newTile );
        }
    }

    sqlite3_finalize( statement );

    return tiles;
};

std::map< sf::Int32, sf::Int32 > SQLFunctions::LoadBiomes( sf::Int32 id )
{
    std::map< sf::Int32, sf::Int32 > biomes;

    // Make a SQL query
    std::string query = "SELECT Z, Biome FROM CellBiomes WHERE Cell = ?";
    sqlite3_stmt *statement;

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return biomes;
    }

    // Bind our request
    err = sqlite3_bind_int( statement, 1, id );
    if( err != SQLITE_OK )
    {
        std::cout << "Bind failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return biomes;
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
            std::cout << "Step failed!" << err << " " << sqlite3_errmsg( db ) << std::endl;
            return biomes;
        }
        else
        {
            // Everything is good to go! Make a new biome entry
            sf::Int32 ret[2];
            for( int i = 0; i < 2; i++ )
                ret[i] = sqlite3_column_int( statement, i );

            biomes.insert( std::pair< sf::Int32, sf::Int32 > ( ret[0], ret[1] ) );
        }
    }

    sqlite3_finalize( statement );

    return biomes;
};

sf::Int32 SQLFunctions::GetMaxCellID()
{
    sf::Int32 maxCellID = 0;
    // Make a SQL query
    sqlite3_stmt *statement;
    std::string query = "SELECT MAX(ID) FROM Cells";

    int err = sqlite3_prepare_v2( db, query.c_str(), -1, &statement, NULL );
    if( err != SQLITE_OK )
    {
        std::cout << "Query failed! " << err << " " << sqlite3_errmsg( db ) << std::endl;
        return 0;
    }

    // Grab the ID
    err = sqlite3_step( statement );
    if( err == SQLITE_ROW )
        maxCellID = sqlite3_column_int( statement, 0 );
    else
        std::cout << "Error grabbing max cell ID." << std::endl;

    return maxCellID;
};

SQLFunctions::~SQLFunctions()
{
    sqlite3_close( db );
};
*/
