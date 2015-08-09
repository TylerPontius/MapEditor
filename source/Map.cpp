#include "Map.hpp"
#include <math.h>

SQLite::Database Map::db( Settings::mapFile, SQLITE_OPEN_READWRITE );
sf::Texture* Map::tileset;

Map::Map()
{
    // Make the tiles
    sf::Image temp;
    temp.loadFromFile( Settings::tilesetFile );
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


std::string Map::GetRegion( sf::Vector3i position, bool subtitle )
{
    sf::Vector3i cellPos = ConvertToCellPosition( position );
    sf::Vector3i tilePos = ConvertToTilePosition( position );

    // Make sure it exists!
    if( CellExists( cellPos ) )
    {
        try
        {
            // Make a SQL query
            SQLite::Statement query( db, "SELECT Name, Subtitle FROM Regions WHERE ID = ?" );

            // Bind our request
            query.bind( 1, int(myCells.at( GetCellID( cellPos ) )->GetRegion( tilePos )) );

            while( query.executeStep() )
            {
                // Everything is good to go! Make a new layer
                std::string region, regionSubtitle;
                region         = query.getColumn( 0 ).getText();
                regionSubtitle = query.getColumn( 1 ).getText();

                if( subtitle )
                    return regionSubtitle;
                else
                    return region;
            }
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }
    }

    return "";
};

// Set a particular tile
void Map::SetTile( sf::Vector3i position, sf::Uint32 tile )
{
    sf::Vector3i cellPos = ConvertToCellPosition( position );
    sf::Vector3i tilePos = ConvertToTilePosition( position );

    // Make sure it exists!
    if( CellExists( cellPos ) )
        myCells.at( GetCellID( cellPos ) )->SetTile( tilePos, tile, tileset );
};

void Map::SetBiome( sf::Vector3i position, sf::Uint32 tile )
{
    sf::Vector3i cellPos = ConvertToCellPosition( position );
    sf::Vector3i tilePos = ConvertToTilePosition( position );

    // Make sure it exists!
    if( CellExists( cellPos ) )
        myCells.at( GetCellID( cellPos ) )->SetBiome( tilePos, tile, tileset );
    else
        std::cout << "no cell\n";
};

// Make sure we have all the proper areas loaded
void Map::UpdateLoadedCells( sf::Vector3i position )
{
    using namespace Settings;
    // Which cells need to be loaded?

    // First, find what cell we are in
    // This lets us work the rest of the math in terms of cell IDs and cell positions
    sf::Vector3i centerCell = ConvertToCellPosition( position );

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
    else if( centerCell.x + (horizCellsNeeded/2) > mapWidth )
        currentCell.x = mapWidth - horizCellsNeeded;
    else
        currentCell.x = centerCell.x - (horizCellsNeeded/2);

    // Check vertical bounds
    if( int(centerCell.y - (vertiCellsNeeded/2)) < 0  )
        currentCell.y = 0;
    else if( centerCell.y + (vertiCellsNeeded/2) > mapWidth )
        currentCell.y = mapWidth - vertiCellsNeeded;
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
        // Cell not found in the map, load it!
        if( !CellExists( it ) )
            LoadCell( it );

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
            toRemove.push_back( it.second->GetID() );
    }


    // Remove the old cells
    for( auto& it : toRemove )
    {
        RemoveCell( it );
        myCells.erase( it );
    }
};


void Map::LoadCell( sf::Vector3i position )
{
    // Make sure it's not already loaded
    if( CellExists( position ) )
        return;

    sf::Uint32 cellID;
    bool toLoadCell = false;

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
                toLoadCell = true;
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
    myCells.insert( std::pair<sf::Uint32, CellPtr>( cellID, std::make_unique<Cell>( cellID, position ) ) );
    assert( myCells.find(cellID) != myCells.end() );

    if( toLoadCell ) myCells.at( cellID )->Load();
}


// Save a cell to the database
void Map::RemoveCell( sf::Uint32 cellID )
{
    if( !CellExists( cellID ) or Settings::saveChanges == false ) return;

    myCells.at( cellID )->Save();
};


void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for( auto& it : myCells )
        target.draw( *(it.second), states );
};

bool Map::CellExists( sf::Vector3i position )
{
    return GetCellID( position ) == 0 ? false : true;
};

bool Map::CellExists( sf::Uint32 cellID )
{
    if( myCells.find( cellID ) == myCells.end() )
        return false;

    return true;
};

sf::Uint32 Map::GetCellID( sf::Vector3i position )
{
    // We don't care about z when looking up cells
    position.z = 0;

    for( auto& it : myCells )
        if( it.second->GetPosition() == position )
            return it.second->GetID();

    return 0;
}

// Convert a world position to a cell position
sf::Vector3i Map::ConvertToCellPosition( sf::Vector3i position )
{
    using namespace Settings;

    position.x = ( position.x / tileSize ) / cellWidth;
    position.y = ( position.y / tileSize ) / cellHeight;

    return position;
};

// Convert a world position to a tile position (relative to its cell)
sf::Vector3i Map::ConvertToTilePosition( sf::Vector3i position )
{
    using namespace Settings;

    position.x = ( position.x / tileSize ) % cellWidth;
    position.y = ( position.y / tileSize ) % cellHeight;

    return position;
};

Map::~Map()
{
    // Save all the open cells
    if( Settings::saveChanges )
        for( auto& it : myCells )
            RemoveCell( it.first );

    delete tileset;
};
