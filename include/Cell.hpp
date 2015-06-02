#ifndef CELL_H
#define CELL_H

#include <memory>
#include <map>
#include "Layer.hpp"

// A cell is a portion of the map.
// A map is made of cells, as they reduce rendering intensity
// and make it easier to manage the world.

// A cell contains all layers from bottom to top

class Cell : public sf::Drawable
{

public:
    Cell( sf::Uint32 id, sf::Vector3i position ) : myID( id ), myPosition( position ) {};
    ~Cell() {};

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    void SetTile( sf::Vector3i position, sf::Uint32 tile );
    void SetBiome( sf::Vector3i position, sf::Uint32 tile );

    std::shared_ptr<Tile> GetTile( sf::Vector3i position );
    sf::Uint32 GetBiome( sf::Vector3i position );

    sf::Uint32 GetID() { return myID; };
    sf::Vector3i GetPosition() { return myPosition; };

private:
    sf::Uint32 myID;
    sf::Vector3i myPosition;

    // Map of all Z layers
    // The first half represents Z level,
    // the second half represents data about that layer
    std::map< sf::Int32, std::unique_ptr<Layer> > myLayers;
};

#endif
