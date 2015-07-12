#ifndef CELL_H
#define CELL_H

#include <memory>
#include <map>
#include <string>
#include <sqlite3.h>
#include "Layer.hpp"

// A cell is a portion of the map.
// A map is made of cells, as they reduce rendering intensity
// and make it easier to manage the world.

// A cell contains all layers from bottom to top

class Cell : public sf::Drawable, public sf::Transformable
{

public:
    Cell( sf::Uint32 id, sf::Vector3i position );

    void SetTile( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset );
    void SetBiome( sf::Vector3i position, sf::Uint32 tile, sf::Texture* tileset );
    void SetAir( sf::Vector3i position, sf::Uint32 air );
    void SetRegion( sf::Vector3i position, sf::Uint32 region );

    sf::Uint32 GetTile( sf::Vector3i position );
    sf::Uint32 GetBiome( sf::Vector3i position );

    sf::Uint32 GetID() { return myID; };
    sf::Vector3i GetPosition() { return myPosition; };

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    void AddLayer( sf::Vector3i position, sf::Texture* tileset );
    void RemoveLayer( sf::Vector3i position );

    sf::Uint32 myID;
    sf::Vector3i myPosition;

    // Map of all Z layers
    // The first half represents Z level,
    // the second half represents data about that layer
    std::map< sf::Int32, Layer > myLayers;
};

#endif
