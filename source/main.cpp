#include <TGUI/TGUI.hpp>
#include <string>
#include <iostream>
#include <sstream>

#include "Map.hpp"

sf::Vector3i globalPosition( 0, 0, 0 );

int main()
{
    // Create the window
    std::string windowTitle = "Map Editor ";
    windowTitle.append( version );
    sf::RenderWindow window( sf::VideoMode( windowWidth, windowHeight ), windowTitle );
    //window.setFramerateLimit( 60 );

    tgui::Gui gui( window );
    auto labelZPos = tgui::Label::create( "widgets/Black.conf" );

    try
    {
        // Load the font
        gui.setGlobalFont( "fonts/DejaVuSans.ttf" );

        // Load the widgets
        labelZPos->setPosition( 32, windowHeight - 48 );
        labelZPos->setTextColor( sf::Color::White );
        labelZPos->setTextSize( 18 );
        gui.add( labelZPos );
    }
    catch (const tgui::Exception& e)
    {
        std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
        return 1;
    }

    // Set the global view position and the selection box position
    sf::Vector3i selectPosition( 0, 0, 0 );



    // Load the map
    sf::Clock cl;
    Map worldMap;
    worldMap.UpdateLoadedCells( globalPosition );
    std::cout << cl.getElapsedTime().asMicroseconds() << "us to load map\n";

    // Load the tile selector
    sf::Texture tileTexture;
    tileTexture.loadFromFile( tilesetFile );

    sf::Sprite tileSprite( tileTexture );

    sf::RenderWindow window2( sf::VideoMode( tileTexture.getSize().x, tileTexture.getSize().y ), "Tile Selector" );
    window2.setPosition( sf::Vector2i( window.getPosition().x + windowWidth + 32, window.getPosition().y ) );

    // Make the selection rectangle
    sf::RectangleShape tileSelection( sf::Vector2f( tileSize, tileSize ) );
    tileSelection.setFillColor( sf::Color::Transparent );
    tileSelection.setOutlineColor( sf::Color::Red );
    tileSelection.setOutlineThickness( 2.f );

    sf::Uint32 currentTile = 0;

    // Create the view
    sf::View view( sf::Vector2f( windowWidth / 2, windowHeight / 2 ), sf::Vector2f( windowWidth, windowHeight ) );
    window.setView( view );

    float zoomLevel = 1.f;
    view.zoom( zoomLevel );

    // Make the selection rectangle
    sf::RectangleShape selection( sf::Vector2f( tileSize, tileSize ) );
    selection.setFillColor( sf::Color::Transparent );
    selection.setOutlineColor( sf::Color::Red );
    selection.setOutlineThickness( 2.5 );

    // Make the cell selector rectangle
    sf::RectangleShape selectionCell( sf::Vector2f( tileSize * cellWidth, tileSize * cellHeight ) );
    selectionCell.setFillColor( sf::Color::Transparent );
    selectionCell.setOutlineColor( sf::Color::Magenta );
    selectionCell.setOutlineThickness( 5.f );

    // Create a lambda function for updating the rectangles
    auto UpdateSelection = [] ( sf::RectangleShape& selection, sf::RectangleShape& selectionCell, sf::Vector3i& position )
    {
        selection.setPosition( position.x, position.y );
        selectionCell.setPosition( position.x - ( position.x % ( tileSize * cellWidth ) ), position.y - ( position.y % ( tileSize * cellHeight ) ) );
    };


    // Run the program as long as the window is open
    sf::Clock clock;
    int fps = 0;

    while( window.isOpen() )
    {
        // Check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while( window.pollEvent(event) )
        {
            // Mouse moved
            if( event.type == sf::Event::MouseButtonPressed )
            {
                // Get the current mouse position in world coords
                sf::Vector2f worldPos( window.mapPixelToCoords( sf::Mouse::getPosition( window ) ) );

                selectPosition.x = worldPos.x;
                selectPosition.y = worldPos.y;

                // Snap to grid
                selectPosition.x -= selectPosition.x % tileSize;
                selectPosition.y -= selectPosition.y % tileSize;
            }

            if( event.type == sf::Event::KeyPressed )
            {
                // Z layer switching between levels
                if( event.key.code == sf::Keyboard::PageUp )
                {
                    selectPosition.z += drawLayers;
                    globalPosition.z += drawLayers;
                }


                if( event.key.code == sf::Keyboard::PageDown )
                {
                    selectPosition.z -= drawLayers;
                    globalPosition.z -= drawLayers;
                }

                // Z layer switching within levels
                if( event.key.code == sf::Keyboard::Key::Add )
                {
                    selectPosition.z += 1;
                    globalPosition.z += 1;
                }

                if( event.key.code == sf::Keyboard::Key::Subtract )
                {
                    selectPosition.z -= 1;
                    globalPosition.z -= 1;
                }

                // Set the selected cell's biome
                if( event.key.code == sf::Keyboard::B )
                    worldMap.SetBiome( selectPosition, currentTile );

                // Set the selected tile
                if( event.key.code == sf::Keyboard::E )
                    worldMap.SetTile( selectPosition, currentTile );

                // Clear the cell
                if( event.key.code == sf::Keyboard::C )
                    worldMap.SetBiome( selectPosition, 0 );

                // Toggle saving
                if( event.key.code == sf::Keyboard::Z )
                    saveChanges = !saveChanges;

                // If a movement button changed state
                if( event.key.code == sf::Keyboard::A  or
                    event.key.code == sf::Keyboard::D or
                    event.key.code == sf::Keyboard::W    or
                    event.key.code == sf::Keyboard::S )
                {
                    // How much should we move?
                    sf::Int32 increment = tileSize * 2 * zoomLevel;

                    // Vector to store delta move
                    sf::Vector2f offset( 0, 0 );

                    switch( event.key.code )
                    {
                        case sf::Keyboard::A: offset.x -= increment; break;
                        case sf::Keyboard::D: offset.x += increment; break;
                        case sf::Keyboard::W: offset.y -= increment; break;
                        case sf::Keyboard::S: offset.y += increment; break;
                        default: break;
                    }

                    view.move( offset );

                    // Make sure we don't move out of the world
                    if( view.getCenter().x - windowWidth / 2 < 0 )
                        view.setCenter( windowWidth / 2, view.getCenter().y );

                    if( view.getCenter().y - windowHeight / 2 < 0 )
                        view.setCenter( view.getCenter().x, windowHeight / 2 );

                    if( view.getCenter().x + windowWidth / 2 > mapWidth * tileSize )
                        view.setCenter( mapWidth * tileSize - windowWidth / 2, view.getCenter().y );

                    if( view.getCenter().y + windowHeight / 2 > mapWidth * tileSize )
                        view.setCenter( view.getCenter().x, mapWidth * tileSize - windowHeight / 2 );

                    globalPosition.x = view.getCenter().x;
                    globalPosition.y = view.getCenter().y;

                    worldMap.UpdateLoadedCells( globalPosition );
                }

                // If a movement button changed state
                if(   event.type == sf::Event::EventType::KeyPressed and
                     (event.key.code == sf::Keyboard::Up  or
                      event.key.code == sf::Keyboard::Left or
                      event.key.code == sf::Keyboard::Down    or
                      event.key.code == sf::Keyboard::Right ) )
                {
                    sf::Int32 increment = tileSize;
                    sf::Vector2i offset( 0, 0 );

                    switch( event.key.code )
                    {
                        case sf::Keyboard::Left:  offset.x -= increment; break;
                        case sf::Keyboard::Right: offset.x += increment; break;
                        case sf::Keyboard::Up:    offset.y -= increment; break;
                        case sf::Keyboard::Down:  offset.y += increment; break;
                        default: break;
                    }

                    selectPosition.x += offset.x;
                    selectPosition.y += offset.y;
                }
            }

            // Scrollwheel moved - adjust zoom
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




            // Catch the resize events
            if( event.type == sf::Event::Resized )
            {
                // Update the view to the new size of the window
                sf::Vector2f visibleArea( event.size.width, event.size.height );
                view.setSize( visibleArea );
            }

            // "close requested" event: we close the window
            if( event.type == sf::Event::Closed or ( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Escape) )
                window.close();

            // Work the GUI
            gui.handleEvent(event);

            // Update our selection rectangles
            UpdateSelection( selection, selectionCell, selectPosition );
        }


        // Process the tile selection window
        while( window2.pollEvent(event) )
        {
            // Mouse click
            if( event.type == sf::Event::MouseButtonPressed )
            {
                // Get the current mouse position in world coords
                sf::Vector2f mousePos( window2.mapPixelToCoords( sf::Mouse::getPosition(window2) ) );

                // Snap to grid
                mousePos.x -= (int)mousePos.x % tileSize;
                mousePos.y -= (int)mousePos.y % tileSize;

                tileSelection.setPosition( mousePos );

                // Calculate the tile ID
                currentTile = ((int)mousePos.x / tileSize ) + ( ((int)mousePos.y / tileSize ) * (tileTexture.getSize().x / tileSize ) );
            }

            // "close requested" event: we close the window
            if( event.type == sf::Event::Closed or ( event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Escape) )
                window.close();
        }

        // Set the label text
        {
            std::stringstream ss;
            if( saveChanges ) ss << "Saving on, ";
            else ss << "Saving off, ";
            ss << "Z = " << selectPosition.z;
            labelZPos->setText( ss.str() );
        }

        // clear the window with black color
        window.clear(sf::Color::Black);
        window.setView( view );

        // draw everything here...
        window.draw( worldMap );

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
