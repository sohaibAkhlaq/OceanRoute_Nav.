#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <cmath>
#include <fstream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Graph.h"
using namespace std;
using namespace sf;

// ============================
// SFML drawing functions
// ============================

void drawArrow(RenderWindow &window, float x1, float y1, float x2, float y2, Color color = Color::Yellow)
{
    // Line
    sf::Vertex line[] =
        {
            sf::Vertex(Vector2f(x1, y1), color),
            sf::Vertex(Vector2f(x2, y2), color)};
    window.draw(line, 2, Lines);

    // Arrowhead
    const float arrowSize = 10.0f;
    float angle = atan2(y2 - y1, x2 - x1);

    float x3 = x2 - arrowSize * cos(angle - M_PI / 6);
    float y3 = y2 - arrowSize * sin(angle - M_PI / 6);

    float x4 = x2 - arrowSize * cos(angle + M_PI / 6);
    float y4 = y2 - arrowSize * sin(angle + M_PI / 6);

    sf::Vertex arrowhead[] =
        {
            sf::Vertex(Vector2f(x2, y2), color),
            sf::Vertex(Vector2f(x3, y3), color),
            sf::Vertex(Vector2f(x2, y2), color),
            sf::Vertex(Vector2f(x4, y4), color)};
    window.draw(arrowhead, 4, Lines);
}

// ============================
// Main
// ============================

int main()
{
    enum AppState
    {
        STARTER,
        NORMAL,
        PANEL_OPEN,
        PANEL_CLOSED
    };
    AppState state = STARTER;

    sf::VideoMode dm = sf::VideoMode::getDesktopMode();

    const unsigned int windowWidth = dm.width;
    const unsigned int windowHeight = dm.height;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight),
                            "Port Map with Directed Routes",
                            sf::Style::Fullscreen);

    // const int windowWidth = 1200*1.45;
    // const int windowHeight = 675*1.45;
    // RenderWindow window(VideoMode(windowWidth, windowHeight), "Port Map with Directed Routes");

    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(false);

    Font font;
    if (!font.loadFromFile("Font/font.ttf"))
    {
        cout << "ERROR: Could not load font.\n";
        return 0;
    }

    Graph g("Routes.txt", "PortLocations.txt", "PortCharges.txt");

    auto mapX = [&](float lon)
    { return ((lon + 180.0f) / 360.0f) * windowWidth; };
    auto mapY = [&](float lat)
    { return ((90.0f - lat) / 180.0f) * windowHeight; };

    Texture mapTexture;
    if (!mapTexture.loadFromFile("Map2.jpg"))
    {
        cout << "ERROR: Could not load Map2.jpg\n";
        return 0;
    }
    Sprite mapSprite(mapTexture);
    mapSprite.setScale(float(windowWidth) / mapTexture.getSize().x,
                       float(windowHeight) / mapTexture.getSize().y);

    // starter image
    Texture starterTexture;
    if (!starterTexture.loadFromFile("starter.jpg"))
    {
        cout << "ERROR: Could not load starter.jpg\n";
        return 0;
    }
    Sprite starterSprite(starterTexture);
    starterSprite.setScale(float(windowWidth) / starterTexture.getSize().x,
                           float(windowHeight) / starterTexture.getSize().y);

    float panelWidth = windowWidth * 0.20f;
    float panelX = -panelWidth;
    bool panelOpen = false;
    float slideSpeed = 800;

    RectangleShape toggleButton(Vector2f(40, 80));
    toggleButton.setFillColor(Color(60, 60, 60));
    toggleButton.setPosition(0, windowHeight / 2 - 40);

    Text btnIcon;
    btnIcon.setFont(font);
    btnIcon.setString(">");
    btnIcon.setCharacterSize(40);
    btnIcon.setFillColor(Color::White);
    btnIcon.setPosition(10, windowHeight / 2 - 35);

    // minimize and close buttons (rectangles so you can replace with sprites later)
    RectangleShape minButton(Vector2f(40, 40));
    minButton.setFillColor(Color(120, 120, 120));
    minButton.setPosition(float(windowWidth - 100), 10.f);

    RectangleShape closeButton(Vector2f(40, 40));
    closeButton.setFillColor(Color(180, 50, 50));
    closeButton.setPosition(float(windowWidth - 50), 10.f);

    Text minIcon;
    minIcon.setFont(font);
    minIcon.setString("-");
    minIcon.setCharacterSize(30);
    minIcon.setFillColor(Color::White);
    minIcon.setPosition(float(windowWidth - 92), 6.f);

    Text closeIcon;
    closeIcon.setFont(font);
    closeIcon.setString("X");
    closeIcon.setCharacterSize(26);
    closeIcon.setFillColor(Color::White);
    closeIcon.setPosition(float(windowWidth - 42), 8.f);

    // starter continue button (semi-transparent white, centered)
    RectangleShape continueButton(Vector2f(300.f, 70.f));
    continueButton.setFillColor(Color(255, 255, 255, 180));
    continueButton.setPosition(windowWidth / 2.f - 150.f, windowHeight / 2.f - 35.f);

    Text continueText;
    continueText.setFont(font);
    continueText.setString("Continue");
    continueText.setCharacterSize(28);
    continueText.setFillColor(Color::Black);
    // center text inside the continue button
    {
        FloatRect tb = continueText.getLocalBounds();
        continueText.setPosition(continueButton.getPosition().x + (continueButton.getSize().x - tb.width) / 2.f - tb.left,
                                 continueButton.getPosition().y + (continueButton.getSize().y - tb.height) / 2.f - tb.top);
    }

    sf::Clock clock;
    sf::Clock starterClock;
    const float starterTimeout = 10.f;

    bool clickLocked = false;
    const float clickCooldown = 0.20f;
    float clickTimer = 0.f;

    vector<string> validNames;
    {
        PortNode *v = g.getVertices();
        while (v)
        {
            validNames.push_back(v->name);
            v = v->next;
        }
    }

    RectangleShape fromBox(Vector2f(panelWidth - 40, 40));
    fromBox.setFillColor(Color(50, 50, 50));
    fromBox.setPosition(20, 100);

    RectangleShape toBox(Vector2f(panelWidth - 40, 40));
    toBox.setFillColor(Color(50, 50, 50));
    toBox.setPosition(20, 180);

    Text fromText;
    fromText.setFont(font);
    fromText.setCharacterSize(20);
    fromText.setFillColor(Color::White);
    fromText.setPosition(25, 105);

    Text toText;
    toText.setFont(font);
    toText.setCharacterSize(20);
    toText.setFillColor(Color::White);
    toText.setPosition(25, 185);

    bool fromFocus = false;
    bool toFocus = false;

    string fromInput = "";
    string toInput = "";

    // hover/focus animation variables
    const float INPUT_HOVER_SCALE = 1.05f;
    const float INPUT_CLICK_SCALE = 1.08f;
    const float INPUT_FOCUS_ANIM_TIME = 0.25f; // 0.25 seconds
    float fromScale = 1.0f;
    float toScale = 1.0f;
    float fromTargetScale = 1.0f;
    float toTargetScale = 1.0f;

    // colors
    Color inputNormalColor = Color(50, 50, 50);
    Color inputFocusColor = Color(70, 70, 70);

    // seed random for suggestions
    srand((unsigned)time(nullptr));

    // suggestion state: generate random suggestions only once per focus activation
    vector<string> randomFromSuggestions;
    vector<string> randomToSuggestions;
    bool fromRandomGenerated = false;
    bool toRandomGenerated = false;
    static std::random_device rd;
    static std::mt19937 rng(rd());

    bool showAllPaths = false;            // Toggle for showing all paths
    RectangleShape showAllPathsButton;    // Button rectangle
    Text showAllPathsText;                // Button label
    const float panelButtonHeight = 40.f; // Height of buttons inside the panel

    // Button for Show All Paths
    showAllPathsButton.setSize(Vector2f(panelWidth - 40, panelButtonHeight));
    showAllPathsButton.setFillColor(Color(70, 70, 70));
    showAllPathsButton.setPosition(panelX + 20, 250.f); // position inside panel

    showAllPathsText.setFont(font);
    showAllPathsText.setString("Show All Paths");
    showAllPathsText.setCharacterSize(18);
    showAllPathsText.setFillColor(Color::White);
    showAllPathsText.setPosition(panelX + 25, 255.f);

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (clickTimer > 0.f)
            clickTimer -= dt;
        if (clickTimer < 0.f)
            clickTimer = 0.f;

        // update dynamic positions
        toggleButton.setPosition(panelX + panelWidth, windowHeight / 2 - 40);
        btnIcon.setPosition(panelX + panelWidth + 10, windowHeight / 2 - 35);

        minButton.setPosition(float(windowWidth - 100), 10.f);
        closeButton.setPosition(float(windowWidth - 50), 10.f);
        minIcon.setPosition(float(windowWidth - 92), 6.f);
        closeIcon.setPosition(float(windowWidth - 42), 8.f);

        fromBox.setPosition(panelX + 20, 90);
        toBox.setPosition(panelX + 20, 170);

        Vector2i mposi = Mouse::getPosition(window);
        Vector2f mpf((float)mposi.x, (float)mposi.y);

        bool hoverFrom = panelOpen && fromBox.getGlobalBounds().contains(mpf);
        bool hoverTo = panelOpen && toBox.getGlobalBounds().contains(mpf);

        fromTargetScale = fromFocus ? INPUT_CLICK_SCALE : hoverFrom ? INPUT_HOVER_SCALE
                                                                    : 1.0f;
        toTargetScale = toFocus ? INPUT_CLICK_SCALE : hoverTo ? INPUT_HOVER_SCALE
                                                              : 1.0f;

        if (INPUT_FOCUS_ANIM_TIME > 0.f)
        {
            float t = dt / INPUT_FOCUS_ANIM_TIME;
            if (t > 1.f)
                t = 1.f;
            fromScale += (fromTargetScale - fromScale) * t;
            toScale += (toTargetScale - toScale) * t;
        }
        else
        {
            fromScale = fromTargetScale;
            toScale = toTargetScale;
        }

        fromBox.setScale(fromScale, fromScale);
        toBox.setScale(toScale, toScale);

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left && clickTimer <= 0.f)
            {
                Vector2f clickPos((float)event.mouseButton.x, (float)event.mouseButton.y);

                // Close / Minimize
                if (closeButton.getGlobalBounds().contains(clickPos))
                {
                    window.close();
                    clickTimer = clickCooldown;
                    continue;
                }
                if (minButton.getGlobalBounds().contains(clickPos))
                {
#ifdef _WIN32
                    ShowWindow((HWND)window.getSystemHandle(), SW_MINIMIZE);
#endif
                    clickTimer = clickCooldown;
                    continue;
                }

                fromFocus = false;
                toFocus = false;

                if (state == STARTER)
                {
                    if (continueButton.getGlobalBounds().contains(clickPos))
                    {
                        state = NORMAL;
                        starterClock.restart();
                        clickTimer = clickCooldown;
                        continue;
                    }
                }
                else
                {
                    // Toggle panel
                    if (toggleButton.getGlobalBounds().contains(clickPos))
                    {
                        panelOpen = !panelOpen;
                        state = panelOpen ? PANEL_OPEN : PANEL_CLOSED;
                        fromFocus = toFocus = false;
                        fromRandomGenerated = toRandomGenerated = false;
                        clickTimer = clickCooldown;
                        continue;
                    }

                    // Input boxes
                    if (panelOpen && fromBox.getGlobalBounds().contains(clickPos))
                    {
                        fromFocus = true;
                        toFocus = false;
                        clickTimer = clickCooldown;
                        continue;
                    }
                    if (panelOpen && toBox.getGlobalBounds().contains(clickPos))
                    {
                        toFocus = true;
                        fromFocus = false;
                        clickTimer = clickCooldown;
                        continue;
                    }

                    // Show All Paths button
                    if (panelOpen && showAllPathsButton.getGlobalBounds().contains(clickPos))
                    {
                        showAllPaths = !showAllPaths;
                        showAllPathsButton.setFillColor(showAllPaths ? Color(120, 120, 120) : Color(70, 70, 70));
                        clickTimer = clickCooldown;
                        continue;
                    }

                    // Suggestion click handling
                    if (panelOpen && (fromFocus || toFocus))
                    {
                        vector<PortNode *> vertexList;
                        PortNode *v = g.getVertices();
                        while (v)
                        {
                            vertexList.push_back(v);
                            v = v->next;
                        }

                        vector<string> clickSuggestions;
                        if (fromFocus)
                        {
                            if (fromInput.empty())
                            {
                                if (!fromRandomGenerated)
                                {
                                    vector<string> pool = validNames;
                                    if (!pool.empty())
                                    {
                                        std::shuffle(pool.begin(), pool.end(), rng);
                                        int take = (int)min((size_t)5, pool.size());
                                        randomFromSuggestions.clear();
                                        for (int i = 0; i < take; ++i)
                                            randomFromSuggestions.push_back(pool[i]);
                                    }
                                    fromRandomGenerated = true;
                                }
                                clickSuggestions = randomFromSuggestions;
                            }
                            else
                            {
                                for (auto &vtx : vertexList)
                                {
                                    string n = vtx->name;
                                    if (n.size() >= fromInput.size() && equal(fromInput.begin(), fromInput.end(), n.begin(),
                                                                              [](char a, char b)
                                                                              { return tolower(a) == tolower(b); }))
                                        clickSuggestions.push_back(n);
                                }
                            }
                        }
                        else if (toFocus)
                        {
                            if (toInput.empty())
                            {
                                if (!toRandomGenerated)
                                {
                                    vector<string> pool = validNames;
                                    if (!pool.empty())
                                    {
                                        std::shuffle(pool.begin(), pool.end(), rng);
                                        int take = (int)min((size_t)5, pool.size());
                                        randomToSuggestions.clear();
                                        for (int i = 0; i < take; ++i)
                                            randomToSuggestions.push_back(pool[i]);
                                    }
                                    toRandomGenerated = true;
                                }
                                clickSuggestions = randomToSuggestions;
                            }
                            else
                            {
                                for (auto &vtx : vertexList)
                                {
                                    string n = vtx->name;
                                    if (n.size() >= toInput.size() && equal(toInput.begin(), toInput.end(), n.begin(),
                                                                            [](char a, char b)
                                                                            { return tolower(a) == tolower(b); }))
                                        clickSuggestions.push_back(n);
                                }
                            }
                        }

                        float autoYClick = (fromFocus ? 130.f : 210.f);
                        for (int i = 0; i < (int)clickSuggestions.size() && i < 5; i++)
                        {
                            FloatRect r(panelX + 20, autoYClick + i * 34, panelWidth - 40, 32);
                            if (r.contains(clickPos))
                            {
                                if (fromFocus)
                                {
                                    fromInput = clickSuggestions[i];
                                    fromFocus = false;
                                    fromRandomGenerated = false;
                                }
                                if (toFocus)
                                {
                                    toInput = clickSuggestions[i];
                                    toFocus = false;
                                    toRandomGenerated = false;
                                }
                                clickTimer = clickCooldown;
                                break;
                            }
                        }
                    }
                }
            }

            if (event.type == Event::TextEntered)
            {
                if (fromFocus)
                {
                    if (event.text.unicode == 8 && !fromInput.empty())
                        fromInput.pop_back();
                    else if (event.text.unicode < 128 && (isalnum(event.text.unicode) || event.text.unicode == ' '))
                        fromInput += (char)event.text.unicode;
                }
                if (toFocus)
                {
                    if (event.text.unicode == 8 && !toInput.empty())
                        toInput.pop_back();
                    else if (event.text.unicode < 128 && (isalnum(event.text.unicode) || event.text.unicode == ' '))
                        toInput += (char)event.text.unicode;
                }
            }
        }

        if (!fromFocus)
            fromRandomGenerated = false;
        if (!toFocus)
            toRandomGenerated = false;

        // STARTER state
        if (state == STARTER)
        {
            if (starterClock.getElapsedTime().asSeconds() >= starterTimeout)
                state = NORMAL;

            window.clear(Color(20, 20, 40));
            window.draw(starterSprite);
            window.draw(continueButton);
            window.draw(continueText);
            window.draw(minButton);
            window.draw(closeButton);
            window.draw(minIcon);
            window.draw(closeIcon);
            window.display();
            continue;
        }

        // Panel animation
        if (panelOpen && panelX < 0)
            panelX += slideSpeed * dt;
        if (!panelOpen && panelX > -panelWidth)
            panelX -= slideSpeed * dt;
        panelX = std::clamp(panelX, -panelWidth, 0.f);

        window.clear(Color(20, 20, 40));
        window.draw(mapSprite);

        vector<PortNode *> vertexList;
        PortNode *v = g.getVertices();
        while (v)
        {
            vertexList.push_back(v);
            v = v->next;
        }

        // Draw edges if showAllPaths is true
        for (auto &srcV : vertexList)
        {
            float x1 = mapX(srcV->portData.lon);
            float y1 = mapY(srcV->portData.lat);
            Edge *e = srcV->head;
            while (e)
            {
                PortNode *destV = g.findVertexByName(e->dest);
                if (destV && showAllPaths)
                {
                    float x2 = mapX(destV->portData.lon);
                    float y2 = mapY(destV->portData.lat);
                    drawArrow(window, x1, y1, x2, y2, Color::White);
                }
                e = e->next;
            }
        }

        // Draw nodes
        for (auto &srcV : vertexList)
        {
            float x = mapX(srcV->portData.lon);
            float y = mapY(srcV->portData.lat);

            CircleShape point(6);
            point.setFillColor(Color(80, 90, 170));
            point.setPosition(x - 5, y - 5);
            window.draw(point);

            Text label;
            label.setFont(font);
            label.setString(srcV->name);
            label.setCharacterSize(14);
            label.setFillColor(Color::Black);
            label.setPosition(srcV->portData.left ? x - 10 - label.getLocalBounds().width : x + 10, y - 7);
            window.draw(label);
        }

        // Draw panel background
        RectangleShape panel(Vector2f(panelWidth, windowHeight));
        panel.setFillColor(Color(30, 30, 30));
        panel.setPosition(panelX, 0);
        window.draw(panel);

        btnIcon.setString(panelOpen ? "<" : ">");
        toggleButton.setPosition(panelX + panelWidth, windowHeight / 2 - 40);
        btnIcon.setPosition(panelX + panelWidth + 10, windowHeight / 2 - 35);
        window.draw(toggleButton);
        window.draw(btnIcon);

        window.draw(minButton);
        window.draw(closeButton);
        window.draw(minIcon);
        window.draw(closeIcon);

        if (panelOpen)
        {
            // From / To input boxes and labels
            fromBox.setFillColor(fromFocus ? inputFocusColor : inputNormalColor);
            fromBox.setPosition(panelX + 20, 90);
            fromBox.setScale(fromScale, fromScale);
            window.draw(fromBox);

            fromText.setFont(font);
            fromText.setString(fromInput);
            fromText.setCharacterSize(18);
            fromText.setFillColor(Color::White);
            fromText.setPosition(panelX + 25, 95);
            window.draw(fromText);

            toBox.setFillColor(toFocus ? inputFocusColor : inputNormalColor);
            toBox.setPosition(panelX + 20, 170);
            toBox.setScale(toScale, toScale);
            window.draw(toBox);

            toText.setFont(font);
            toText.setString(toInput);
            toText.setCharacterSize(18);
            toText.setFillColor(Color::White);
            toText.setPosition(panelX + 25, 175);
            window.draw(toText);

            // Labels
            Text fromLabel, toLabel;
            fromLabel.setFont(font);
            fromLabel.setString("From:");
            fromLabel.setCharacterSize(20);
            fromLabel.setFillColor(Color::White);
            fromLabel.setPosition(panelX + 20, 50);
            window.draw(fromLabel);
            toLabel.setFont(font);
            toLabel.setString("To:");
            toLabel.setCharacterSize(20);
            toLabel.setFillColor(Color::White);
            toLabel.setPosition(panelX + 20, 150);
            window.draw(toLabel);

            // Show All Paths button
            showAllPathsButton.setPosition(panelX + 20, 250.f);
            showAllPathsText.setPosition(panelX + 25, 255.f);
            window.draw(showAllPathsButton);
            window.draw(showAllPathsText);

            // inside the panel drawing / suggestion loop
            if (fromFocus || toFocus)
            {
                float autoY = fromFocus ? 130.f : 210.f;
                string &inputRef = fromFocus ? fromInput : toInput; // fix
                vector<string> &currentSuggestions = fromFocus ? randomFromSuggestions : randomToSuggestions;

                if (inputRef.empty())
                {
                    if (!fromFocus ? !toRandomGenerated : !fromRandomGenerated)
                    {
                        vector<string> pool = validNames;
                        if (!pool.empty())
                        {
                            std::shuffle(pool.begin(), pool.end(), rng);
                            int take = (int)min((size_t)5, pool.size());
                            currentSuggestions.clear();
                            for (int i = 0; i < take; ++i)
                                currentSuggestions.push_back(pool[i]);
                        }
                        if (fromFocus)
                            fromRandomGenerated = true;
                        else
                            toRandomGenerated = true;
                    }
                }
                else
                {
                    for (auto &vtx : vertexList)
                    {
                        string n = vtx->name;
                        if (n.size() >= inputRef.size() &&
                            equal(inputRef.begin(), inputRef.end(), n.begin(),
                                  [](char a, char b)
                                  { return tolower(a) == tolower(b); }))
                        {
                            currentSuggestions.push_back(n);
                        }
                    }
                }

                for (int i = 0; i < (int)currentSuggestions.size() && i < 5; i++)
                {
                    RectangleShape sugBox(Vector2f(panelWidth - 40, 32));
                    sugBox.setFillColor(Color(200, 200, 200));
                    sugBox.setPosition(panelX + 20, autoY + i * 34);
                    window.draw(sugBox);

                    Text sugText;
                    sugText.setFont(font);
                    sugText.setString(currentSuggestions[i]);
                    sugText.setCharacterSize(18);
                    sugText.setFillColor(Color::Black);
                    sugText.setPosition(panelX + 25, autoY + 5 + i * 34);
                    window.draw(sugText);
                }
            }
        }

        window.display();
    }

    return 0;
}
