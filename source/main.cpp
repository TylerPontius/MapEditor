#include <TGUI/TGUI.hpp>
#include <string>
#include <iostream>
#include <sstream>

#include "Map.hpp"
#include "TileMap.hpp"

#define MAP_ED_VERSION "0.2"



int main()
{
    // create the window
    std::string windowTitle = "Map Editor ";
    windowTitle.append( MAP_ED_VERSION );
    sf::RenderWindow window( sf::VideoMode( WINDOW_W, WINDOW_H ), windowTitle );
    //window.setFramerateLimit( 60 );

    tgui::Gui gui( window );
    auto labelZPos = tgui::Label::create( "widgets/Black.conf" );

    try
    {
        // Load the font
        gui.setGlobalFont( "fonts/DejaVuSans.ttf" );

        // Load the widgets
        labelZPos->setText("Username:");
        labelZPos->setPosition( 32, WINDOW_H - 48 );
    }
    catch (const tgui::Exception& e)
    {
        std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
        return 1;
    }

    // Set the position
    sf::Vector3i position;
    position.x = 0;
    position.y = 0;
    position.z = 0;


    // Load the map
    sf::Clock cl;
    Map worldMap;
    worldMap.UpdateLoadedCells( position );
    std::cout << cl.getElapsedTime().asMicroseconds() << "us to load map\n";

    // Load the tile selector
    sf::Texture tileTexture;
    tileTexture.loadFromFile( TILESET_FILE );
    sf::Sprite tileSprite;
    tileSprite.setTexture( tileTexture );
    sf::RenderWindow window2( sf::VideoMode( tileTexture.getSize().x, tileTexture.getSize().y ), "Tile Selector" );
    window2.setPosition( sf::Vector2i( window.getPosition().x + WINDOW_W + 32, window.getPosition().y ) );

    // Make the selection rectangle
    sf::RectangleShape tileSelection( sf::Vector2f( TILE_SIZE, TILE_SIZE ) );
    tileSelection.setFillColor( sf::Color::Transparent );
    tileSelection.setOutlineColor( sf::Color::Red );
    tileSelection.setOutlineThickness( 2.f );

    sf::Uint32 currentTile = 0;

    // Create the view
    sf::View view( sf::Vector2f( WINDOW_W / 2, WINDOW_H / 2 ), sf::Vector2f( WINDOW_W, WINDOW_H ) );
    window.setView( view );

    float zoomLevel = 1.f;
    view.zoom( zoomLevel );

    // Make the selection rectangle
    sf::RectangleShape selection( sf::Vector2f( TILE_SIZE, TILE_SIZE ) );
    selection.setFillColor( sf::Color::Transparent );
    selection.setOutlineColor( sf::Color::Red );
    selection.setOutlineThickness( 3.f );

    sf::RectangleShape selectionCell( sf::Vector2f( TILE_SIZE * CELL_WIDTH, TILE_SIZE * CELL_HEIGHT ) );
    selectionCell.setFillColor( sf::Color::Transparent );
    selectionCell.setOutlineColor( sf::Color::Magenta );
    selectionCell.setOutlineThickness( 5.f );

    auto UpdateSelection = [] ( sf::RectangleShape* selection, sf::RectangleShape* selectionCell, sf::Vector3i position )
    {
        selection->setPosition( position.x, position.y );
        selectionCell->setPosition( position.x - ( position.x % ( TILE_SIZE * CELL_WIDTH ) ), position.y - ( position.y % ( TILE_SIZE * CELL_HEIGHT ) ) );
    };


    // run the program as long as the window is open
    sf::Clock clock; int fps = 0;
    while( window.isOpen() )
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while( window.pollEvent(event) )
        {
            // mouse moved
            if( event.type == sf::Event::MouseButtonPressed )
            {
                // get the current mouse position in world coords
                sf::Vector2f worldPos = window.mapPixelToCoords( sf::Mouse::getPosition(window) );

                position.x = worldPos.x;
                position.y = worldPos.y;

                // Snap to grid
                position.x -= position.x % TILE_SIZE;
                position.y -= position.y % TILE_SIZE;
            }

            // Z layer switching
            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::PageUp )
                position.z += DRAW_LAYERS;

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::PageDown )
                position.z -= DRAW_LAYERS;

            // Z layer switching
            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Num1 )
                position.z += 1;

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Num2 )
                position.z -= 1;

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::B )
                worldMap.SetCellBiome( position, currentTile );

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::E )
                worldMap.SetCellTile( position, currentTile );

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::C )
                worldMap.SetCellBiome( position, 0 );

            if( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Z )
                saveChanges = !saveChanges;

            // Scrollwheel moved
            if( event.type == sf::Event::MouseWheelMoved )
            {
                float zoomFactor = 1.f;

                if( event.mouseWheel.delta == -1 and zoomLevel < 16.f )
                    zoomFactor = 2;


                if( event.mouseWheel.delta == 1 and zoomLevel > 1.f )
                    zoomFactor = 0.5;

                zoomLevel *= zoomFactor;

                view.zoom( zoomFactor );
            }

            // If a movement button changed state
            if(   event.type == sf::Event::EventType::KeyPressed and
                 (event.key.code == sf::Keyboard::A  or
                  event.key.code == sf::Keyboard::D or
                  event.key.code == sf::Keyboard::W    or
                  event.key.code == sf::Keyboard::S ) )
            {
                sf::Int32 increment = TILE_SIZE * 2 * zoomLevel;
                sf::Vector2f pos;
                pos.x = 0;
                pos.y = 0;

                switch( event.key.code )
                {
                    case sf::Keyboard::A: pos.x -= increment; break;
                    case sf::Keyboard::D: pos.x += increment; break;
                    case sf::Keyboard::W: pos.y -= increment; break;
                    case sf::Keyboard::S: pos.y += increment; break;
                    default: break;
                }

                view.move( pos );

                // Make sure we don't move out of the world
                if( view.getCenter().x - WINDOW_W / 2 < 0 )
                    view.setCenter( WINDOW_W / 2, view.getCenter().y );

                if( view.getCenter().y - WINDOW_H / 2 < 0 )
                    view.setCenter( view.getCenter().x, WINDOW_H / 2 );

                if( view.getCenter().x + WINDOW_W / 2 > MAP_WIDTH * TILE_SIZE )
                    view.setCenter( MAP_WIDTH * TILE_SIZE - WINDOW_W / 2, view.getCenter().y );

                if( view.getCenter().y + WINDOW_H / 2 > MAP_HEIGHT * TILE_SIZE )
                    view.setCenter( view.getCenter().x, MAP_HEIGHT * TILE_SIZE - WINDOW_H / 2 );

                sf::Vector3i viewPos;
                viewPos.x = view.getCenter().x;
                viewPos.y = view.getCenter().y;

                worldMap.UpdateLoadedCells( viewPos );
            }

            // If a movement button changed state
            if(   event.type == sf::Event::EventType::KeyPressed and
                 (event.key.code == sf::Keyboard::Up  or
                  event.key.code == sf::Keyboard::Left or
                  event.key.code == sf::Keyboard::Down    or
                  event.key.code == sf::Keyboard::Right ) )
            {
                sf::Int32 increment = TILE_SIZE;
                sf::Vector2f pos;
                pos.x = 0;
                pos.y = 0;

                switch( event.key.code )
                {
                    case sf::Keyboard::Left:  pos.x -= increment; break;
                    case sf::Keyboard::Right: pos.x += increment; break;
                    case sf::Keyboard::Up:    pos.y -= increment; break;
                    case sf::Keyboard::Down:  pos.y += increment; break;
                    default: break;
                }

                position.x += pos.x;
                position.y += pos.y;
            }


            // catch the resize events
            if( event.type == sf::Event::Resized )
            {
                // update the view to the new size of the window
                sf::Vector2f visibleArea( event.size.width, event.size.height );
                view.setSize( visibleArea );
            }

            // "close requested" event: we close the window
            if( event.type == sf::Event::Closed or ( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Escape) )
                window.close();

            // Work the GUI
            gui.handleEvent(event);

            // Update our selection rectangles
            UpdateSelection( &selection, &selectionCell, position );
        }


        // Process the tile selection window
        while( window2.pollEvent(event) )
        {
            // mouse moved
            if( event.type == sf::Event::MouseButtonPressed )
            {
                // get the current mouse position in world coords
                sf::Vector2f mousePos = window2.mapPixelToCoords( sf::Mouse::getPosition(window2) );

                // Snap to grid
                mousePos.x -= (int)mousePos.x % TILE_SIZE;
                mousePos.y -= (int)mousePos.y % TILE_SIZE;

                tileSelection.setPosition( mousePos ); std::cout << ((int)mousePos.x % TILE_SIZE ) << "  " << ((int)mousePos.y / TILE_SIZE ) << "  " << (tileTexture.getSize().x / TILE_SIZE ) << std::endl;

                currentTile = ((int)mousePos.x / TILE_SIZE ) + ( ((int)mousePos.y / TILE_SIZE ) * (tileTexture.getSize().x / TILE_SIZE ) );
            }

            // "close requested" event: we close the window
            if( event.type == sf::Event::Closed or ( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Escape) )
                window.close();
        }

        {
            std::stringstream ss;
            if( saveChanges ) ss << "Saving on, ";
            else ss << "Saving off, ";
            ss << "Z = " << position.z;
            labelZPos->setText( ss.str() );
        }

        // clear the window with black color
        window.clear(sf::Color::Black);
        window.setView( view );

        // draw everything here...
        worldMap.DrawMap( &window, position );

        window.draw( selection );
        window.draw( selectionCell );

        gui.draw();

        // end the current frame
        window.display();

        window2.clear(sf::Color::Blue);
        window2.draw( tileSprite );
        window2.draw( tileSelection );
        window2.display();

        if( clock.getElapsedTime().asSeconds() > 1.0f )
        {
            clock.restart();
            std::cout << fps << "\n";
            fps = 0;
        }

        else fps++;
    }

    return 0;
}
