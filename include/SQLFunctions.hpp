#ifndef SQLFUNCTIONS_H
#define SQLFUNCTIONS_H

#include <string>
#include <iostream>
#include <sqlite3.h>
#include <memory>
#include "Cell.hpp"

class SQLFunctions
{
public:
    SQLFunctions();
    ~SQLFunctions();
    std::shared_ptr<Cell> LoadCell( sf::Uint32 id );
    std::vector< std::shared_ptr<Tile> > LoadTileLayer( sf::Uint32 cell, sf::Int32 z );
    std::map< sf::Uint32, sf::Uint32 > LoadBiomes( sf::Uint32 id );

private:
    sqlite3 *db;
};

#endif // SQLFUNCTIONS_H
