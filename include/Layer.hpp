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

    Layer( sf::Int32 cell, sf::Vector3i position, sf::Texture* tileset );
    ~Layer();

    void Save();
    void Load();

    void SetBiome( sf::Int32 biome );
    void SetRegion( sf::Int32 region ) { myRegion = region; };
    void SetAir( sf::Int32 air ) { myAir = air; };

    void SetTile( sf::Vector3i position, sf::Int32 tile );

    sf::Int32 GetBiome()  { return myBiome;  };
    sf::Int32 GetRegion() { return myRegion; };
    sf::Int32 GetAir()    { return myAir;    };

    sf::Int32 GetTile( sf::Vector3i position );

private:
    void Clear();

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray myVertices;
    std::vector<Tile> myTiles;

    sf::Int32 myCell;
    sf::Vector3i myPosition;
    sf::Texture* myTileset;

    sf::Int32 myBiome = 0, myRegion = 0, myAir = 0;

};

#endif // LAYER_H
