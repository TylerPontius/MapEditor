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

    Layer( sf::Vector3i position, sf::Texture* tileset );

    void SetBiome( sf::Uint32 biome );
    void SetArea( sf::Uint32 area ) { myArea = area; };

    void SetTile( sf::Vector3i position, sf::Uint32 tile );

    sf::Uint32 GetBiome()   { return myBiome; };
    sf::Uint32 GetArea()    { return myArea;  };
    sf::Uint32 GetTile( sf::Vector3i position );

private:
    void Clear();

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray myVertices;
    std::vector<Tile> myTiles;

    sf::Texture* myTileset;
    sf::Vector3i myPosition;

    sf::Uint32 myBiome = 0, myArea = 0;

};

#endif // LAYER_H
