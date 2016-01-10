#include <TGUI/TGUI.hpp>
#include <string>
#include <iostream>
#include <sstream>

#include "Map.hpp"

sf::Vector3i Settings::globalPosition( 0, 0, 0 );
bool Settings::showFPS = false;
bool Settings::saveChanges = false;

int main()
{
    using namespace Settings;

    // Create the window
    std::string windowTitle = "Map Editor ";
    windowTitle.append( version );
    sf::RenderWindow window( sf::VideoMode( windowWidth, windowHeight ), windowTitle );
    window.setFramerateLimit( 60 );

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

    sf::Int32 currentTile = 0;

    // Create the view
    sf::View view( sf::Vector2f( windowWidth / 2, windowHeight / 2 ), sf::Vector2f( windowWidth, windowHeight ) );
    window.setView( view );

    float zoomLevel = 1.f;
    view.zoom( zoomLevel );

    // Make the selection rectangle
    sf::RectangleShape selection( sf::Vector2f( tileSize, tileSize ) );
    selection.setFillColor( sf::Color::Transparent );
    selection.setOutlineColor( sf::Color::Red );
    selection.setOutlineThickness( 3.f );

    // Make the cell selector rectangle
    sf::RectangleShape selectionCell( sf::Vector2f( tileSize * cellWidth, tileSize * cellHeight ) );
    selectionCell.setFillColor( sf::Color::Transparent );
    selectionCell.setOutlineColor( sf::Color::Magenta );
    selectionCell.setOutlineThickness( 15.f );

    // Make the cell selector rectangle
    sf::RectangleShape mapBorder( sf::Vector2f( mapWidth * tileSize * cellWidth, mapHeight * tileSize * cellHeight ) );
    mapBorder.setFillColor( sf::Color::Transparent );
    mapBorder.setOutlineColor( sf::Color::Green );
    mapBorder.setOutlineThickness( 19.f );

    // Create a lambda function for updating the rectangles
    auto UpdateSelection = [] ( sf::RectangleShape& selection, sf::RectangleShape& selectionCell, sf::Vector3i& position )
    {
        selection.setPosition( position.x, position.y );
        selectionCell.setPosition( position.x - ( position.x % ( tileSize * cellWidth ) ), position.y - ( position.y % ( tileSize * cellHeight ) ) );
    };


    // Run the program as long as the window is open
    sf::Clock clock;
    int fps = 0;

    sf::Vector3i previousSelection;
    sf::Vector2f viewOffset, rightClickHolding;
    std::string region, regionSubtitle;


    while( window.isOpen() )
    {
        previousSelection = selectPosition;

        // Check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while( window.pollEvent(event) )
        {
            // Mouse clicked
            if( event.type == sf::Event::MouseButtonPressed )
            {
                if( event.mouseButton.button == sf::Mouse::Left )
                {
                    // Get the current mouse position in world coords
                    sf::Vector2f worldPos( window.mapPixelToCoords( sf::Mouse::getPosition( window ) ) );

                    selectPosition.x = worldPos.x;
                    selectPosition.y = worldPos.y;

                    // Snap to grid
                    selectPosition.x -= selectPosition.x % tileSize;
                    selectPosition.y -= selectPosition.y % tileSize;

                    std::cout << "Selection: " << selectPosition.x << ", " << selectPosition.y << std::endl;
                }

                if( event.mouseButton.button == sf::Mouse::Right )
                {
                    rightClickHolding = sf::Vector2f( event.mouseButton.x, event.mouseButton.y );
                }
            }

            // Mouse released
            if( event.type == sf::Event::MouseButtonReleased )
            {
                if( event.mouseButton.button == sf::Mouse::Right )
                {
                    rightClickHolding = sf::Vector2f( 0, 0 );
                }
            }

            if( event.type == sf::Event::MouseMoved )
            {
                // Are we panning the view?
                if( sf::Mouse::isButtonPressed( sf::Mouse::Button::Right ) and rightClickHolding != sf::Vector2f( 0, 0 ) )
                {
                    // Find the change in mouse position
                    sf::Vector2f deltaMouse = sf::Vector2f( rightClickHolding.x - event.mouseMove.x, rightClickHolding.y - event.mouseMove.y );

                    //std::cout << "PRINT\n";
                    //std::cout << rightClickHolding.x << " " << rightClickHolding.y << "\n";
                    //std::cout << event.mouseMove.x << " " << event.mouseMove.y << "\n";

                    float increment = tileSize * zoomLevel;
                    float mouseSensitivity = tileSize / 4;

                    // If we've moved at least a tile, update the view
                    if( abs( deltaMouse.x / mouseSensitivity ) >= 1.0  or abs( deltaMouse.y / mouseSensitivity ) >= 1.0 )
                    {
                        // We will 'use up' the mouse movement during this calculation, so we need to change
                        // the pinned/grabbing position to reflect this used movement.
                        rightClickHolding -= deltaMouse;

                        // Snap to the grid; round up if negative, down if positive
                        sf::Vector2f deltaTiles;
                        if( deltaMouse.x > 0 )
                            deltaTiles.x = floorf( deltaMouse.x / mouseSensitivity );
                        else
                            deltaTiles.x = ceilf( deltaMouse.x / mouseSensitivity );

                        if( deltaMouse.y > 0 )
                            deltaTiles.y = floorf( deltaMouse.y / mouseSensitivity );
                        else
                            deltaTiles.y = ceilf( deltaMouse.y / mouseSensitivity );

                        // Set the offset (to be moved later)
                        viewOffset = deltaTiles * increment;
                    }
                }
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

                    viewOffset = offset;
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

                // Zoom out
                if( event.mouseWheel.delta == -1 and zoomLevel < 64.f )
                    zoomFactor = 2;

                // Zoom in
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

        // Update the view
        if( viewOffset.x != 0 or viewOffset.y != 0 )
        {
            view.move( viewOffset );

            // Make sure we don't move out of the world
            if( view.getCenter().x - windowWidth / 2 < 0 )
                view.setCenter( windowWidth / 2, view.getCenter().y );

            if( view.getCenter().y - windowHeight / 2 < 0 )
                view.setCenter( view.getCenter().x, windowHeight / 2 );

            if( view.getCenter().x + windowWidth / 2 > mapWidth * cellWidth * tileSize )
                view.setCenter( mapWidth * cellWidth * tileSize - windowWidth / 2, view.getCenter().y );

            if( view.getCenter().y + windowHeight / 2 > mapHeight * cellHeight * tileSize )
                view.setCenter( view.getCenter().x, mapWidth * cellHeight * tileSize - windowHeight / 2 );

            globalPosition.x = view.getCenter().x;
            globalPosition.y = view.getCenter().y;

            worldMap.UpdateLoadedCells( globalPosition );

            viewOffset = sf::Vector2f( 0, 0 );
        }

        // Set the label text
        {
            std::stringstream ss;
            if( saveChanges ) ss << "Saving on, ";
            else ss << "Saving off, ";

            ss << "Z = " << selectPosition.z;

            if( previousSelection != selectPosition )
            {
                region         = worldMap.GetRegion( selectPosition, false );
                regionSubtitle = worldMap.GetRegion( selectPosition, true  );
            }

            ss << "   Region = " << region << ", " << regionSubtitle;

            labelZPos->setText( ss.str() );
        }

        // clear the window with black color
        window.clear(sf::Color::Black);
        window.setView( view );

        // draw everything here...
        window.draw( worldMap );

        window.draw( selection );
        window.draw( selectionCell );
        window.draw( mapBorder );

        gui.draw();

        // end the current frame
        window.display();

        window2.clear(sf::Color::Blue);
        window2.draw( tileSprite );
        window2.draw( tileSelection );
        window2.display();

        if( clock.getElapsedTime().asSeconds() > 1.0f and showFPS )
        {
            clock.restart();
            std::cout << fps << "\n";
            fps = 0;
        }

        else fps++;
    }

    return 0;
}
