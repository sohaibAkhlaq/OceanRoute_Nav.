#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct ClickPoint
{
    float lon, lat;
    int id; // Click number
};

int main()
{
   
    sf::VideoMode dm = sf::VideoMode::getDesktopMode();

    const unsigned int windowWidth = dm.width;
    const unsigned int windowHeight = dm.height;

    // Exclusive fullscreen — exact native resolution, covers taskbar
    // sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight),
    //                         "Port Map with Directed Routes",
    //                         sf::Style::Fullscreen);

     // const int windowWidth = 1800 / 1;
    // const int windowHeight = 900 / 1;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Map2 Click Tool");

    // Load Map2.jpg
    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("Map8.jpg"))
    {
        cout << "ERROR: Could not load Map2.jpg\n";
        return 0;
    }
    sf::Sprite mapSprite(mapTexture);
    mapSprite.setScale(
        float(windowWidth) / mapTexture.getSize().x,
        float(windowHeight) / mapTexture.getSize().y);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("Font/arial.ttf"))
    {
        cout << "ERROR: Could not load font.\n";
        return 0;
    }

    // Mapping functions
    auto mapX = [&](float lon)
    { return ((lon + 180.0f) / 360.0f) * windowWidth; };
    auto mapY = [&](float lat)
    { return ((90.0f - lat) / 180.0f) * windowHeight; };
    auto invMapX = [&](float x)
    { return (x / windowWidth) * 360.0f - 180.0f; };
    auto invMapY = [&](float y)
    { return 90.0f - (y / windowHeight) * 180.0f; };

    vector<ClickPoint> clicks;
    vector<ClickPoint> undoneClicks;
    int clickCount = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();

            // Mouse click to add point
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                float x = event.mouseButton.x;
                float y = event.mouseButton.y;

                float lon = invMapX(x);
                float lat = invMapY(y);

                clickCount++;
                clicks.push_back({lon, lat, clickCount});
                undoneClicks.clear(); // clear redo stack
                cout << "Click #" << clickCount << ": Lon = " << lon << ", Lat = " << lat << endl;
            }

            // Undo / Redo
            if (event.type == sf::Event::KeyPressed)
            {
                bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
                                   sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

                if (ctrlPressed && event.key.code == sf::Keyboard::Z)
                {
                    // Undo last click
                    if (!clicks.empty())
                    {
                        undoneClicks.push_back(clicks.back());
                        cout << "Undo Click #" << clicks.back().id << endl;
                        clicks.pop_back();
                    }
                }

                if (ctrlPressed && event.key.code == sf::Keyboard::Y)
                {
                    // Redo last undone click
                    if (!undoneClicks.empty())
                    {
                        clicks.push_back(undoneClicks.back());
                        cout << "Redo Click #" << undoneClicks.back().id << endl;
                        undoneClicks.pop_back();
                    }
                }
            }
        }

        window.clear();
        window.draw(mapSprite);

        // Draw axes
        float axisThickness = 2.0f;
        sf::RectangleShape xAxis(sf::Vector2f(windowWidth, axisThickness));
        xAxis.setFillColor(sf::Color::White);
        xAxis.setPosition(0, mapY(0));
        window.draw(xAxis);

        sf::RectangleShape yAxis(sf::Vector2f(axisThickness, windowHeight));
        yAxis.setFillColor(sf::Color::White);
        yAxis.setPosition(mapX(0), 0);
        window.draw(yAxis);

        // Longitude ticks
        for (int lon = -180; lon <= 180; lon += 30)
        {
            float x = mapX(lon);
            float yCenter = mapY(0);
            sf::RectangleShape tick(sf::Vector2f(1, 10));
            tick.setFillColor(sf::Color::White);
            tick.setPosition(x, yCenter - 5);
            window.draw(tick);

            sf::Text label;
            label.setFont(font);
            label.setString(to_string(lon));
            label.setCharacterSize(14);
            label.setFillColor(sf::Color::White);
            label.setPosition(x - 10, yCenter + 10);
            window.draw(label);
        }

        // Latitude ticks
        for (int lat = -90; lat <= 90; lat += 30)
        {
            float y = mapY(lat);
            float xCenter = mapX(0);
            sf::RectangleShape tick(sf::Vector2f(10, 1));
            tick.setFillColor(sf::Color::White);
            tick.setPosition(xCenter - 5, y);
            window.draw(tick);

            sf::Text label;
            label.setFont(font);
            label.setString(to_string(lat));
            label.setCharacterSize(14);
            label.setFillColor(sf::Color::White);
            label.setPosition(xCenter + 10, y - 8);
            window.draw(label);
        }

        // Draw clicks
        for (auto &p : clicks)
        {
            float x = mapX(p.lon);
            float y = mapY(p.lat);
            sf::CircleShape point(5);
            point.setFillColor(sf::Color::Red);
            point.setPosition(x - 5, y - 5);
            window.draw(point);

            sf::Text numberLabel;
            numberLabel.setFont(font);
            numberLabel.setString(to_string(p.id));
            numberLabel.setCharacterSize(14);
            numberLabel.setFillColor(sf::Color::Red);
            numberLabel.setPosition(x + 8, y - 10);
            window.draw(numberLabel);
        }

        window.display();
    }

    cout << "\n\n\n ===== FInnal ====\n";
    for (auto click : clicks)
    {
        cout << "Click #" << click.id << ": Lon = " << click.lon << ", Lat = " << click.lat << endl;
    }

    return 0;
}
