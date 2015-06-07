#ifndef LAYER_H
#define LAYER_H

#include <SFML/Graphics.hpp>
#include <memory>
#include "Tile.hpp"
#include "Settings.hpp"

// Layer contains a VertexArray which holds the graphics for a cell

class Layer : public sf::Drawable, public sf::Transformable
{
public:

    Layer( sf::Texture* tileset, sf::Vector3i position );

    void SetBiome( sf::Uint32 biome );
    void AddTile( sf::Vector3i position, sf::Uint32 tile );
    void RemoveTile( sf::Vector3i position );

    sf::Uint32 GetBiome() { return myBiome; };
    sf::Uint32 GetArea()  { return myArea;  };
    sf::Uint32 GetTile();

    void Clear();

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray myVertices;
    std::vector< std::unique_ptr<Tile> > myTiles;

    sf::Texture* myTileset;
    sf::Vector3i myPosition;

    sf::Uint32 myBiome, myArea;

};

#endif // LAYER_H
