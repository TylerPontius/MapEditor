#ifndef TILEMAP_H
#define TILEMAP_H

#include <memory>
#include "Tile.hpp"
#include <SFML/Graphics.hpp>

class TileMap : public sf::Drawable, public sf::Transformable
{
public:

    void Load( sf::Texture* tileset );
    void AddTile( std::shared_ptr<Tile> tile );
    void Clear();

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray m_vertices;
    sf::Texture* m_tileset;
};

#endif // TILEMAP_H
