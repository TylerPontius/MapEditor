#ifndef TILEMAP_H
#define TILEMAP_H

#include <SFML/Graphics.hpp>
#include <memory>
#include "Tile.hpp"

// Layer contains a VertexArray which holds the graphics for a cell

class Layer : public sf::Drawable, public sf::Transformable
{
public:

    Layer( sf::Texture* tileset, sf::Vector3i position );

    void SetBiome( sf::Uint32 biome );
    void AddTile( std::shared_ptr<Tile> tile );
    void RemoveTile( sf::Vector3i position );
    void Clear();

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray myVertices;
    std::vector< std::shared_ptr<Tile> > myTiles;

    sf::Texture* myTileset;
    sf::Vector3i myPosition;

    sf::Uint32 myBiome, myArea;

};

#endif // TILEMAP_H
