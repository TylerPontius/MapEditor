#include "TileMap.hpp"
#include "Map.hpp"

void TileMap::Load(sf::Texture* tileset )
{
    m_tileset = tileset;

    // resize the vertex array to fit the cell size
    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize( CELL_WIDTH * CELL_HEIGHT * 4 );
};

void TileMap::AddTile( std::shared_ptr<Tile> tile )
{
    sf::Uint32 tileNumber = tile->GetTile();
    sf::Uint32 x = tile->GetPosition().x, y = tile->GetPosition().y;

    // find its position in the tileset texture
    int tu = tileNumber % (m_tileset->getSize().x / TILE_SIZE );
    int tv = tileNumber / (m_tileset->getSize().x / TILE_SIZE );

    // get a pointer to the current tile's quad
    sf::Vertex* quad = &m_vertices[(x + y * CELL_WIDTH) * 4];

    // define its 4 corners
    quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
    quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE);
    quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
    quad[3].position = sf::Vector2f(x * TILE_SIZE, (y + 1) * TILE_SIZE);

    // define its 4 texture coordinates
    quad[0].texCoords = sf::Vector2f(tu * TILE_SIZE, tv * TILE_SIZE);
    quad[1].texCoords = sf::Vector2f((tu + 1) * TILE_SIZE, tv * TILE_SIZE);
    quad[2].texCoords = sf::Vector2f((tu + 1) * TILE_SIZE, (tv + 1) * TILE_SIZE);
    quad[3].texCoords = sf::Vector2f(tu * TILE_SIZE, (tv + 1) * TILE_SIZE);
};

void TileMap::Clear()
{
    m_vertices.clear();
};

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = m_tileset;

    // draw the vertex array
    target.draw(m_vertices, states);
};
