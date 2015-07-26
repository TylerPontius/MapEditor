#ifndef LAYER_H
#define LAYER_H

#include <SFML/Graphics.hpp>
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include "Tile.hpp"
#include "Settings.hpp"

// Layer contains a VertexArray which holds the graphics for a cell
class Layer : public sf::Drawable, public sf::Transformable
{
    friend class Map;

public:

    Layer( sf::Uint32 cell, sf::Vector3i position, sf::Texture* tileset );

    void Save();
    void Load();

    void SetBiome( sf::Uint32 biome );
    void SetRegion( sf::Uint32 region ) { myRegion = region; };
    void SetAir( sf::Uint32 air ) { myAir = air; };

    void SetTile( sf::Vector3i position, sf::Uint32 tile );

    sf::Uint32 GetBiome()  { return myBiome;  };
    sf::Uint32 GetRegion() { return myRegion; };
    sf::Uint32 GetAir()    { return myAir;    };

    sf::Uint32 GetTile( sf::Vector3i position );

private:
    void ClearLayer();

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray myVertices;
    std::vector<Tile> myTiles;

    sf::Vector3i myPosition;
    sf::Texture* myTileset;

    sf::Uint32 myCell;
    sf::Uint32 myBiome = 0, myRegion = 0, myAir = 0;

};

#endif // LAYER_H
