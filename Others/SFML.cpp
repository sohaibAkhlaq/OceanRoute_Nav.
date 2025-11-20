#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace sf;

struct City
{
    string name;
    float lat, lon; // latitude, longitude
    bool left = false; // true if 'L' appears after coordinates
};

int main()
{
    const int windowWidth = 1800;
    const int windowHeight = 900;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "City Map Viewer (Global Coordinates)");

    // Load font
    Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        cout << "ERROR: Could not load font.\n";
        return 0;
    }

    // Load city data from file
    ifstream file("PortLocations.txt");
    if (!file.is_open())
    {
        cout << "ERROR: Could not open locations.txt\n";
        return 0;
    }

    vector<City> cities;
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        City c;
        string flag;
        ss >> c.name >> c.lat >> c.lon >> flag;
        if (!ss.fail())
        {
            if (flag == "L")
                c.left = true;
            cities.push_back(c);
        }
    }
    file.close();

    if (cities.empty())
    {
        cout << "No cities loaded!\n";
        return 0;
    }

    // Mapping functions for global coordinates
    auto mapX = [&](float lon)
    {
        // Longitude -180..180 → X 0..windowWidth
        return ((lon + 180.0f) / 360.0f) * windowWidth;
    };

    auto mapY = [&](float lat)
    {
        // Latitude -90..90 → Y windowHeight..0 (invert)
        return ((90.0f - lat) / 180.0f) * windowHeight;
    };

    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("Map2.jpg"))
    {
        cout << "ERROR: Could not load Map.jpg\n";
        return 0;
    }
    sf::Sprite mapSprite(mapTexture);

    // Resize sprite to window size (optional)
    mapSprite.setScale(
        float(windowWidth) / mapTexture.getSize().x,
        float(windowHeight) / mapTexture.getSize().y);

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        window.clear(Color(20, 20, 40)); // Dark background

        // -----------------------------
        // Draw axes
        // -----------------------------
        window.draw(mapSprite);
        float axisThickness = 2.0f;

        // X-axis (equator)
        RectangleShape xAxis(Vector2f(windowWidth, axisThickness));
        xAxis.setFillColor(Color::White);
        xAxis.setPosition(0, mapY(0)); // Y = 0 latitude
        // window.draw(xAxis);

        // Y-axis (prime meridian)
        RectangleShape yAxis(Vector2f(axisThickness, windowHeight));
        yAxis.setFillColor(Color::White);
        yAxis.setPosition(mapX(0), 0); // X = 0 longitude
        // window.draw(yAxis);

        // -----------------------------
        // Draw longitude tick marks
        // -----------------------------
        for (int lon = -180; lon <= 180; lon += 30)
        {
            float x = mapX(lon);
            float yCenter = mapY(0);

            // Tick mark
            RectangleShape tick(Vector2f(1, 10));
            tick.setFillColor(Color::White);
            tick.setPosition(x, yCenter - 5);
            // window.draw(tick);

            // Label
            Text label;
            label.setFont(font);
            label.setString(to_string(lon));
            label.setCharacterSize(14);
            label.setFillColor(Color::White);
            label.setPosition(x - 10, yCenter + 10);
            // window.draw(label);
        }

        // -----------------------------
        // Draw latitude tick marks
        // -----------------------------
        for (int lat = -90; lat <= 90; lat += 30)
        {
            float y = mapY(lat);
            float xCenter = mapX(0);

            // Tick mark
            RectangleShape tick(Vector2f(10, 1));
            tick.setFillColor(Color::White);
            tick.setPosition(xCenter - 5, y);
            // window.draw(tick);

            // Label
            Text label;
            label.setFont(font);
            label.setString(to_string(lat));
            label.setCharacterSize(14);
            label.setFillColor(Color::White);
            label.setPosition(xCenter + 10, y - 8);
            // window.draw(label);
        }

        // -----------------------------
        // Draw cities
        // -----------------------------
        for (auto &c : cities)
        {
            float x = mapX(c.lon);
            float y = mapY(c.lat);

            // Draw city point
            CircleShape point(5);
            point.setFillColor(Color::Red);
            point.setPosition(x - 5, y - 5);
            window.draw(point);

            // Draw city name
            Text text;
            text.setFont(font);
            text.setString(c.name);
            text.setCharacterSize(14);
            text.setFillColor(Color::Black);
            if (c.left)
                text.setPosition(x - 8 - text.getLocalBounds().width, y - 10); // show left
            else
                text.setPosition(x + 8, y - 10); // default right
            window.draw(text);
        }

        window.display();
    }

    return 0;
}
