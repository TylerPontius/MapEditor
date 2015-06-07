#include "Layer.hpp"

Layer::Layer(sf::Texture* tileset, sf::Vector3i position ) : myTileset( tileset ), myPosition( position )
{
    myTileset = tileset;

    // resize the vertex array to fit the cell size
    myVertices.setPrimitiveType(sf::Quads);
    myVertices.resize( cellWidth * cellHeight * 4 );
};

// Set every tile in the cell to the biome
void Layer::SetBiome( sf::Uint32 biome )
{
    myBiome = biome;

    Clear();
}

// Add a tile to the layer
void Layer::AddTile( sf::Vector3i position, sf::Uint32 tile )
{
    // First, remove a tile if one already occupies the same position
    RemoveTile( position );

    // Add the tile and start computing
    myTiles.emplace_back( std::make_unique<Tile> (position, tile, 255 ) );
    sf::Uint32 x = position.x - myPosition.x, y = position.y - myPosition.y;

    // Find its position in the tileset texture
    int tu = tile % (myTileset->getSize().x / tileSize );
    int tv = tile / (myTileset->getSize().x / tileSize );

    // Get a pointer to the current tile's quad
    sf::Vertex* quad = &myVertices[( (x + y) * cellWidth) * 4];

    // Define its 4 corners
    quad[0].position = sf::Vector2f(x * tileSize, y * tileSize);
    quad[1].position = sf::Vector2f((x + 1) * tileSize, y * tileSize);
    quad[2].position = sf::Vector2f((x + 1) * tileSize, (y + 1) * tileSize);
    quad[3].position = sf::Vector2f(x * tileSize, (y + 1) * tileSize);

    // Define its 4 texture coordinates
    quad[0].texCoords = sf::Vector2f(tu * tileSize, tv * tileSize);
    quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize, tv * tileSize);
    quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize, (tv + 1) * tileSize);
    quad[3].texCoords = sf::Vector2f(tu * tileSize, (tv + 1) * tileSize);
};

// Remove a tile from the layer
void Layer::RemoveTile( sf::Vector3i position )
{
    // Remove the tile from our list
    std::remove_if( myTiles.begin(), myTiles.end(),
                    [&] ( auto& tile ) { return (tile->myPosition == position); } );

    // Find the relative position
    sf::Uint32 x = position.x - myPosition.x, y = position.y - myPosition.y;

    // Get a pointer to the current tile's quad and delete its info
    sf::Vertex* quad = &myVertices[( (x + y) * cellWidth) * 4];

    for( int i = 0; i < 4; i++ )
    {
        quad[i].position = sf::Vector2f( 0.f, 0.f);
        quad[i].texCoords = sf::Vector2f( 0.f, 0.f);
    }
};

// Remove all tiles and vertices
void Layer::Clear()
{
    myVertices.clear();
    myTiles.clear();
};

void Layer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = myTileset;

    // draw the vertex array
    target.draw(myVertices, states);
};
