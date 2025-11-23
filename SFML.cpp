#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
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
#include "H/Graph.h"
#include "H/Menu.h"
#include "H/ButtonBox.h" // Your header-only class
using namespace std;
using namespace sf;

// ============================
// SFML drawing functions
// ============================

void drawArrow(sf::RenderWindow &window, float x1, float y1, float x2, float y2,
               sf::Color color = sf::Color::Yellow, float thickness = 1.f)
{
    // Draw line (shaft) as a rectangle for thickness
    sf::Vector2f dir(x2 - x1, y2 - y1);
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    float angle = atan2(dir.y, dir.x) * 180.f / 3.14159265f;

    sf::RectangleShape shaft(sf::Vector2f(length - 10.f, thickness)); // subtract arrowhead size
    shaft.setFillColor(color);
    shaft.setOrigin(0, thickness / 2.f);
    shaft.setPosition(x1, y1);
    shaft.setRotation(angle);
    window.draw(shaft);

    // Draw arrowhead as triangle
    const float arrowSize = 10.f;
    sf::ConvexShape arrowhead;
    arrowhead.setPointCount(3);
    arrowhead.setPoint(0, sf::Vector2f(0.f, 0.f));
    arrowhead.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2.f));
    arrowhead.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize / 2.f));
    arrowhead.setFillColor(color);
    arrowhead.setPosition(x2, y2);
    arrowhead.setRotation(angle);
    window.draw(arrowhead);
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
    if (!font.loadFromFile("Font/arial.ttf"))
    {
        cout << "ERROR: Could not load font.\n";
        return 0;
    }

    Graph g("Data/Routes.txt", "Data/PortLocations.txt", "Data/PortCharges.txt");

    auto mapX = [&](float lon)
    { return ((lon + 180.0f) / 360.0f) * windowWidth; };
    auto mapY = [&](float lat)
    { return ((90.0f - lat) / 180.0f) * windowHeight; };

    Texture mapTexture;
    if (!mapTexture.loadFromFile("Images/Map2.jpg"))
    {
        cout << "ERROR: Could not load Images/Map2.jpg\n";
        return 0;
    }
    Sprite mapSprite(mapTexture);
    mapSprite.setScale(float(windowWidth) / mapTexture.getSize().x,
                       float(windowHeight) / mapTexture.getSize().y);

    // starter image
    Texture starterTexture;
    if (!starterTexture.loadFromFile("Images/starter.jpg"))
    {
        cout << "ERROR: Could not load Images/starter.jpg\n";
        return 0;
    }
    Sprite starterSprite(starterTexture);
    starterSprite.setScale(float(windowWidth) / starterTexture.getSize().x,
                           float(windowHeight) / starterTexture.getSize().y);

    float panelWidth = windowWidth * 0.20f;
    float panelX = -panelWidth;
    bool panelOpen = false;
    float slideSpeed = 800;

    RectangleShape toggleButton, minButton, closeButton;
    Text btnIcon, minIcon, closeIcon;
    // Toggle button
    setupInputBox(toggleButton, btnIcon, font, ">",
                  Vector2f(0, windowHeight / 2 - 40), Vector2f(40, 80),
                  Color(60, 60, 60), Color::White, 40);
    // Minimize button
    setupInputBox(minButton, minIcon, font, "-",
                  Vector2f(windowWidth - 100, 10.f), Vector2f(40, 40),
                  Color(120, 120, 120), Color::White, 30);
    // Close button
    setupInputBox(closeButton, closeIcon, font, "X",
                  Vector2f(windowWidth - 50, 10.f), Vector2f(40, 40),
                  Color(180, 50, 50), Color::White, 26);

    // starter continue button (semi-transparent white, centered)
    RectangleShape continueButton(Vector2f(300.f, 70.f));
    Text continueText;
    setupInputBox(continueButton, continueText, font, "Continue",
                  Vector2f(windowWidth / 2.f - 150.f, windowHeight / 2.f - 35.f),
                  Vector2f(300.f, 70.f),
                  Color(255, 255, 255, 180), Color::Black, 28);

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

    RectangleShape fromBox, toBox;
    Text fromText, toText;
    setupInputBox(fromBox, fromText, font, "",
                  Vector2f(20, 100), Vector2f(panelWidth - 40, 40),
                  Color(50, 50, 50), Color::White, 20);

    setupInputBox(toBox, toText, font, "",
                  Vector2f(20, 200), Vector2f(panelWidth - 40, 40),
                  Color(50, 50, 50), Color::White, 20);

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
    setupInputBox(showAllPathsButton, showAllPathsText, font, "Continue",
                  Vector2f(panelX + 20, windowHeight - 60), Vector2f(panelWidth - 40, panelButtonHeight),
                  Color(70, 70, 70), Color::White, 18);

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (clickTimer > 0.f)
            clickTimer -= dt;
        if (clickTimer < 0.f)
            clickTimer = 0.f;

        // update dynamic positions
        setupInputBox(toggleButton, btnIcon, font, ">",
                      Vector2f(panelX + panelWidth, windowHeight / 2 - 40), Vector2f(40, 80),
                      Color(60, 60, 60), Color::White, 40);

        setupInputBox(fromBox, fromText, font, fromInput,
                      Vector2f(panelX + 20, 90), Vector2f(panelWidth - 40, 40),
                      Color(50, 50, 50), Color::White, 20);

        setupInputBox(toBox, toText, font, toInput,
                      Vector2f(panelX + 20, 190), Vector2f(panelWidth - 40, 40),
                      Color(50, 50, 50), Color::White, 20);

        Vector2i mposi = Mouse::getPosition(window);
        Vector2f mpf((float)mposi.x, (float)mposi.y);

        bool hoverFrom = panelOpen && fromBox.getGlobalBounds().contains(mpf);
        bool hoverTo = panelOpen && toBox.getGlobalBounds().contains(mpf);

        fromTargetScale = fromFocus ? INPUT_CLICK_SCALE : (hoverFrom ? INPUT_HOVER_SCALE : 1.0f);
        toTargetScale = toFocus ? INPUT_CLICK_SCALE : (hoverTo ? INPUT_HOVER_SCALE : 1.0f);

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

                // default: clear focus, will be re-enabled below if click matches an input
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

                    // Input boxes (give focus)
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
                        // showAllPathsButton.setFillColor(showAllPaths ? Color(120, 120, 120) : Color(70, 70, 70));
                        clickTimer = clickCooldown;
                        continue;
                    }

                    // ------------------------------------------------------------
                    // NEW SUGGESTION CLICK HANDLER (RIGHT SIDE PANEL)
                    // ------------------------------------------------------------
                    if (panelOpen && (fromFocus || toFocus))
                    {
                        // Suggestion positions aligned with drawing code
                        float autoY = fromFocus ? 90.f : 170.f;
                        float autoX = panelX + 250.f;

                        // Pick correct suggestions list
                        vector<string> &currentSuggestions = fromFocus ? randomFromSuggestions : randomToSuggestions;

                        for (int i = 0; i < (int)currentSuggestions.size() && i < 5; i++)
                        {
                            FloatRect r(autoX, autoY + i * 34, 150, 32);
                            if (r.contains(clickPos.x, clickPos.y))
                            {
                                if (fromFocus)
                                    fromInput = currentSuggestions[i];
                                else
                                    toInput = currentSuggestions[i];

                                // Lock selection & reset random state
                                fromFocus = toFocus = false;
                                fromRandomGenerated = false;
                                toRandomGenerated = false;

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
        } // end pollEvent

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

        // ---- Drawing ----
        window.clear(Color(20, 20, 40));
        window.draw(mapSprite);

        // =========================================================================================================================
        // Draw edges if showAllPaths is true
        for (PortNode *srcV : g)
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
        for (auto srcV : g)
        {
            float x = mapX(srcV->portData.lon);
            float y = mapY(srcV->portData.lat);

            CircleShape point(6);
            if (to_lower(srcV->name) == to_lower(fromInput) || to_lower(srcV->name) == to_lower(toInput))
                point.setFillColor(Color::Red);
            else
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

        if (g.findVertexByName_Lower(fromInput) && g.findVertexByName_Lower(toInput))
        {
            auto [pathEdges, pathNodes] = g.dijkstraPath(fromInput, toInput);

            // Draw edges in path (bold / highlighted)
            for (auto &[fromNode, e] : pathEdges)
            {
                PortNode *toNode = g.findVertexByName(e->dest);
                if (!toNode)
                    continue;

                float x1 = mapX(fromNode->portData.lon);
                float y1 = mapY(fromNode->portData.lat);
                float x2 = mapX(toNode->portData.lon);
                float y2 = mapY(toNode->portData.lat);

                drawArrow(window, x1, y1, x2, y2, Color::Yellow, 2.f); // bold yellow
                // cout << "Edge: " << fromNode->name << " -> " << toNode->name << "\n";
            }

            // Draw nodes in path (highlighted)
            for (PortNode *v : pathNodes)
            {
                float x = mapX(v->portData.lon);
                float y = mapY(v->portData.lat);

                CircleShape point(7); // bigger circle for path
                if (to_lower(v->name) == to_lower(fromInput) || to_lower(v->name) == to_lower(toInput))
                    point.setFillColor(Color::Red);
                else
                    point.setFillColor(Color::Yellow);

                point.setPosition(x - 7, y - 7);
                window.draw(point);
                // cout << "Node: " << v->name << "\n";
            }
        }

        // =============================================================================================================================
        // Draw panel background
        RectangleShape panel(Vector2f(panelWidth, windowHeight));
        panel.setFillColor(Color(30, 30, 30));
        panel.setPosition(panelX, 0);
        window.draw(panel);

        // Draw toggle & window control buttons
        setupInputBox(toggleButton, btnIcon, font, panelOpen ? "<" : ">",
                      Vector2f(panelX + panelWidth, windowHeight / 2 - 40), Vector2f(40, 80),
                      Color(60, 60, 60), Color::White, 40);

        window.draw(toggleButton);
        window.draw(btnIcon);

        window.draw(minButton);
        window.draw(closeButton);
        window.draw(minIcon);
        window.draw(closeIcon);

        // --------------------
        // PANEL CONTENT (inputs, labels, buttons)
        // --------------------
        if (panelOpen)
        {
            Color validNormalColor = Color(30, 136, 229); // Professional blue
            Color validFocusColor = Color(66, 165, 245);  // Lighter blue

            // FROM box
            Color FromColor;
            if (g.findVertexByName_Lower(fromInput))
                FromColor = fromFocus ? validFocusColor : validNormalColor;
            else
                FromColor = fromFocus ? inputFocusColor : inputNormalColor;

            setupInputBox(fromBox, fromText, font, fromInput,
                          Vector2f(panelX + 20, 90), Vector2f(panelWidth - 40, 40),
                          FromColor, Color::White, 20);

            window.draw(fromBox);

            window.draw(fromText);

            // TO BOX
            Color ToColor;
            if (g.findVertexByName_Lower(toInput))
                ToColor = (toFocus ? validFocusColor : validNormalColor);
            else
                ToColor = (toFocus ? inputFocusColor : inputNormalColor);

            setupInputBox(toBox, toText, font, toInput,
                          Vector2f(panelX + 20, 190), Vector2f(panelWidth - 40, 40),
                          ToColor, Color::White, 20);
            window.draw(toBox);
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

            setupInputBox(showAllPathsButton, showAllPathsText, font, "Show all Paths",
                          Vector2f(panelX + 20, windowHeight - 60), Vector2f(panelWidth - 40, panelButtonHeight),
                          showAllPaths ? Color(120, 120, 120) : Color(70, 70, 70), Color::White, 18);
            window.draw(showAllPathsButton);
            window.draw(showAllPathsText);
        } // end panelOpen

        // --------------------
        // SUGGESTIONS (draw when focused)
        // --------------------
        if (fromFocus || toFocus)
        {
            float autoY = fromFocus ? 90.f : 170.f; // match input y
            float autoX = panelX + 250.f;           // right side of input

            string &inputRef = fromFocus ? fromInput : toInput;
            vector<string> &currentSuggestions = fromFocus ? randomFromSuggestions : randomToSuggestions;

            // Recompute suggestions each frame (keeps them in sync)
            currentSuggestions.clear();

            if (inputRef.empty())
            {
                // random picks
                vector<string> pool = validNames;
                std::shuffle(pool.begin(), pool.end(), rng);
                int take = (int)min((size_t)5, pool.size());
                for (int i = 0; i < take; ++i)
                    currentSuggestions.push_back(pool[i]);
            }
            else
            {
                // case-insensitive prefix match
                for (auto &v : validNames)
                {
                    if (v.size() >= inputRef.size())
                    {
                        bool match = true;
                        for (size_t k = 0; k < inputRef.size(); ++k)
                        {
                            if (tolower(v[k]) != tolower(inputRef[k]))
                            {
                                match = false;
                                break;
                            }
                        }
                        if (match)
                            currentSuggestions.push_back(v);
                    }
                }
            }

            // Draw suggestion boxes (on the RIGHT)
            for (int i = 0; i < (int)currentSuggestions.size() && i < 5; i++)
            {
                RectangleShape box(Vector2f(150, 32));
                box.setFillColor(Color(200, 200, 200));
                box.setPosition(autoX, autoY + i * 34);
                window.draw(box);

                Text txt;
                txt.setFont(font);
                txt.setString(currentSuggestions[i]);
                txt.setCharacterSize(18);
                txt.setFillColor(Color::Black);
                txt.setPosition(autoX + 5, autoY + 5 + i * 34);
                window.draw(txt);
            }
        }

        window.display();
    } // end while

    return 0;
}
