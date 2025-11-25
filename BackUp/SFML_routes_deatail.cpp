#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "H/Graph.h"
#include "H/Menu.h"
#include "H/ButtonBox.h"

using namespace std;
using namespace sf;

// Helper: convert "HH:MM" to minutes since midnight
int timeToMinutes(const string &t)
{
    int h = (t[0] - '0') * 10 + (t[1] - '0');
    int m = (t[3] - '0') * 10 + (t[4] - '0');
    return h * 60 + m;
}

// Helper: minutes to "Xh Ym"
string minutesToString(int mins)
{
    if (mins < 0)
        mins = 0;
    int h = mins / 60;
    int m = mins % 60;
    stringstream ss;
    if (h > 0)
        ss << h << "h ";
    if (m > 0 || h == 0)
        ss << m << "m";
    return ss.str().empty() ? "0m" : ss.str();
}

// Arrow drawing
void drawArrow(RenderWindow &window, float x1, float y1, float x2, float y2,
               Color color = Color::Yellow, float thickness = 1.f)
{
    Vector2f dir(x2 - x1, y2 - y1);
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 1.f)
        return;
    float angle = atan2(dir.y, dir.x) * 180.f / 3.14159265f;

    RectangleShape shaft(Vector2f(length - 12.f, thickness));
    shaft.setFillColor(color);
    shaft.setOrigin(0, thickness / 2.f);
    shaft.setPosition(x1, y1);
    shaft.setRotation(angle);
    window.draw(shaft);

    ConvexShape head(3);
    head.setPoint(0, Vector2f(0, 0));
    head.setPoint(1, Vector2f(-12, 6));
    head.setPoint(2, Vector2f(-12, -6));
    head.setFillColor(color);
    head.setPosition(x2, y2);
    head.setRotation(angle);
    window.draw(head);
}

// Dropdown & SelectBox
struct Dropdown
{
    vector<string> items;
    float x = 0, y = 0, width = 300.f, itemHeight = 44.f;
    int maxVisible = 8;
    float alpha = 0.f, scrollOffset = 0.f;
    int hovered = -1;
    bool open = false;
    FloatRect getBounds() const { return FloatRect(x, y, width, itemHeight * maxVisible + 10); }
};

struct SelectBox
{
    string selected, placeholder;
    RectangleShape box;
    Text text, arrow;
    Dropdown dropdown;
};

SelectBox fromSelect, toSelect, modeSelect, dateSelect;
SelectBox *activeDropdown = nullptr;

// Path navigation
RectangleShape prevBtn, nextBtn;
Text prevTxt, nextTxt;
int currentPathIndex = 0;
vector<pair<vector<pair<PortNode *, Edge *>>, vector<PortNode *>>> allPaths;

// Scrollable details panel
RectangleShape detailsBox, detailsScrollTrack;
RectangleShape detailsScrollThumb;
float detailsScrollOffset = 0.f;
const float detailsMaxHeight = 500.f;
const float detailsLineHeight = 78.f;

// NEW: Collapsible Route Details Panel
bool detailsPanelOpen = false;
RectangleShape detailsToggleBtn, detailsBackBtn;
Text detailsToggleTxt, detailsBackTxt;

int main()
{
    VideoMode dm = VideoMode::getDesktopMode();
    RenderWindow window(VideoMode(dm.width, dm.height), "Port Map with Directed Routes", Style::Fullscreen);
    window.setFramerateLimit(60);

    Font font;
    if (!font.loadFromFile("Font/arial.ttf"))
    {
        cout << "Font not found!\n";
        return 0;
    }

    Graph g("Data/Routes3.txt", "Data/PortLocations.txt", "Data/PortCharges.txt");

    Texture mapTexture, starterTexture;
    if (!mapTexture.loadFromFile("Images/Map2.jpg") || !starterTexture.loadFromFile("Images/starter.jpg"))
        return 0;

    Sprite mapSprite(mapTexture), starterSprite(starterTexture);
    mapSprite.setScale(float(dm.width) / mapTexture.getSize().x, float(dm.height) / mapTexture.getSize().y);
    starterSprite.setScale(float(dm.width) / starterTexture.getSize().x, float(dm.height) / starterTexture.getSize().y);

    float panelWidth = dm.width * 0.20f;
    float panelX = -panelWidth;
    bool panelOpen = false;
    float slideSpeed = 1200.f;

    RectangleShape toggleBtn, minBtn, closeBtn, continueBtn, showAllBtn;
    Text toggleTxt, minTxt, closeTxt, continueTxt, showAllTxt;

    setupInputBox(toggleBtn, toggleTxt, font, ">", Vector2f(0, dm.height / 2 - 40), Vector2f(40, 80), Color(60, 60, 60), Color::White, 40);
    setupInputBox(minBtn, minTxt, font, "-", Vector2f(dm.width - 100, 10), Vector2f(40, 40), Color(120, 120, 120), Color::White, 30);
    setupInputBox(closeBtn, closeTxt, font, "X", Vector2f(dm.width - 50, 10), Vector2f(40, 40), Color(180, 50, 50), Color::White, 26);
    setupInputBox(continueBtn, continueTxt, font, "Continue", Vector2f(dm.width / 2 - 150, dm.height / 2 - 35), Vector2f(300, 70), Color(255, 255, 255, 180), Color::Black, 28);

    // NEW: Route Details Toggle Button
    setupInputBox(detailsToggleBtn, detailsToggleTxt, font, "Route Details >", Vector2f(0, 0), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);
    setupInputBox(detailsBackBtn, detailsBackTxt, font, "< Back", Vector2f(0, 0), Vector2f(80, 50), Color(100, 60, 60), Color::White, 20);

    fromSelect.placeholder = "Select departure port...";
    toSelect.placeholder = "Select destination port...";
    modeSelect.placeholder = "All Possible Paths";
    modeSelect.selected = "All Possible Paths";

    // Date Select Setup
    dateSelect.placeholder = "Select date...";
    dateSelect.selected = "1 / 12 / 2024";
    vector<string> days;
    for (int d = 1; d <= 31; ++d)
        days.push_back(to_string(d) + " / 12 / 2024");

    fromSelect.box = toSelect.box = modeSelect.box = dateSelect.box = RectangleShape(Vector2f(panelWidth - 40, 56));
    fromSelect.arrow = toSelect.arrow = modeSelect.arrow = dateSelect.arrow = Text("V", font, 28);
    fromSelect.arrow.setFillColor(Color(180, 180, 180));
    toSelect.arrow.setFillColor(Color(180, 180, 180));
    modeSelect.arrow.setFillColor(Color(180, 180, 180));
    dateSelect.arrow.setFillColor(Color(180, 180, 180));

    modeSelect.dropdown.items = {"All Possible Paths", "Optimal Path"};

    prevBtn = RectangleShape(Vector2f(50, 50));
    nextBtn = RectangleShape(Vector2f(50, 50));
    prevTxt = Text("<", font, 32);
    prevTxt.setFillColor(Color::White);
    nextTxt = Text(">", font, 32);
    nextTxt.setFillColor(Color::White);

    vector<string> allPorts;
    for (PortNode *v = g.getVertices(); v; v = v->next)
        allPorts.push_back(v->name);
    sort(allPorts.begin(), allPorts.end());

    bool showAllPaths = false;
    Clock clock, starterClock;
    float clickTimer = 0.f;
    const float clickCooldown = 0.2f;

    enum
    {
        STARTER,
        NORMAL
    } state = STARTER;

    auto mapX = [&](float lon)
    { return ((lon + 180.f) / 360.f) * dm.width; };
    auto mapY = [&](float lat)
    { return ((90.f - lat) / 180.f) * dm.height; };

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        clickTimer -= dt;

        Vector2i mi = Mouse::getPosition(window);
        Vector2f mouse(mi.x, mi.y);

        if (panelOpen && panelX < 0)
            panelX += slideSpeed * dt;
        if (!panelOpen && panelX > -panelWidth)
            panelX -= slideSpeed * dt;
        panelX = clamp(panelX, -panelWidth, 0.f);

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape))
                window.close();

            if (event.type == Event::MouseWheelScrolled && event.mouseWheelScroll.wheel == Mouse::VerticalWheel)
            {
                Dropdown *active = nullptr;
                if (fromSelect.dropdown.open && fromSelect.dropdown.getBounds().contains(mouse))
                    active = &fromSelect.dropdown;
                else if (toSelect.dropdown.open && toSelect.dropdown.getBounds().contains(mouse))
                    active = &toSelect.dropdown;
                else if (modeSelect.dropdown.open && modeSelect.dropdown.getBounds().contains(mouse))
                    active = &modeSelect.dropdown;
                else if (dateSelect.dropdown.open && dateSelect.dropdown.getBounds().contains(mouse))
                    active = &dateSelect.dropdown;

                if (active && active->items.size() > size_t(active->maxVisible))
                {
                    active->scrollOffset -= event.mouseWheelScroll.delta * 35.f;
                    float maxScroll = (active->items.size() - active->maxVisible) * active->itemHeight;
                    active->scrollOffset = clamp(active->scrollOffset, 0.f, max(0.f, maxScroll));
                }

                if (detailsPanelOpen && detailsBox.getGlobalBounds().contains(mouse))
                {
                    int edges = allPaths.empty() ? 0 : allPaths[currentPathIndex].first.size();
                    float contentHeight = edges * detailsLineHeight + 180;
                    if (contentHeight > detailsMaxHeight)
                    {
                        float maxOffset = contentHeight - detailsMaxHeight;
                        detailsScrollOffset -= event.mouseWheelScroll.delta * 45.f;
                        detailsScrollOffset = clamp(detailsScrollOffset, 0.f, maxOffset);
                    }
                }
            }

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left && clickTimer <= 0.f)
            {
                Vector2f pos(event.mouseButton.x, event.mouseButton.y);
                clickTimer = clickCooldown;

                if (closeBtn.getGlobalBounds().contains(pos))
                    window.close();
                if (minBtn.getGlobalBounds().contains(pos))
                {
#ifdef _WIN32
                    ShowWindow((HWND)window.getSystemHandle(), SW_MINIMIZE);
#endif
                }
                if (state == STARTER && continueBtn.getGlobalBounds().contains(pos))
                    state = NORMAL;
                if (toggleBtn.getGlobalBounds().contains(pos))
                {
                    panelOpen = !panelOpen;
                    fromSelect.dropdown.open = toSelect.dropdown.open = modeSelect.dropdown.open = dateSelect.dropdown.open = false;
                    activeDropdown = nullptr;
                    detailsPanelOpen = false;
                    continue;
                }

                if (!panelOpen)
                    continue;

                // NEW: Toggle Details Panel
                if (!allPaths.empty() && detailsToggleBtn.getGlobalBounds().contains(pos))
                {
                    detailsPanelOpen = !detailsPanelOpen;
                    continue;
                }
                if (detailsPanelOpen && detailsBackBtn.getGlobalBounds().contains(pos))
                {
                    detailsPanelOpen = false;
                    continue;
                }

                if (activeDropdown != nullptr)
                {
                    bool inside = activeDropdown->dropdown.getBounds().contains(pos) ||
                                  activeDropdown->box.getGlobalBounds().contains(pos);
                    if (!inside)
                    {
                        fromSelect.dropdown.open = toSelect.dropdown.open = modeSelect.dropdown.open = dateSelect.dropdown.open = false;
                        activeDropdown = nullptr;
                        continue;
                    }
                }

                bool hitFrom = fromSelect.box.getGlobalBounds().contains(pos);
                bool hitTo = toSelect.box.getGlobalBounds().contains(pos);
                bool hitMode = modeSelect.box.getGlobalBounds().contains(pos);
                bool hitDate = dateSelect.box.getGlobalBounds().contains(pos);

                if (hitDate && activeDropdown == nullptr)
                {
                    dateSelect.dropdown.open = !dateSelect.dropdown.open;
                    fromSelect.dropdown.open = toSelect.dropdown.open = modeSelect.dropdown.open = false;
                    activeDropdown = dateSelect.dropdown.open ? &dateSelect : nullptr;
                    dateSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitFrom && activeDropdown == nullptr)
                {
                    fromSelect.dropdown.open = !fromSelect.dropdown.open;
                    toSelect.dropdown.open = modeSelect.dropdown.open = dateSelect.dropdown.open = false;
                    activeDropdown = fromSelect.dropdown.open ? &fromSelect : nullptr;
                    fromSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitTo && activeDropdown == nullptr)
                {
                    toSelect.dropdown.open = !toSelect.dropdown.open;
                    fromSelect.dropdown.open = modeSelect.dropdown.open = dateSelect.dropdown.open = false;
                    activeDropdown = toSelect.dropdown.open ? &toSelect : nullptr;
                    toSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitMode && activeDropdown == nullptr)
                {
                    modeSelect.dropdown.open = !modeSelect.dropdown.open;
                    fromSelect.dropdown.open = toSelect.dropdown.open = dateSelect.dropdown.open = false;
                    activeDropdown = modeSelect.dropdown.open ? &modeSelect : nullptr;
                    modeSelect.dropdown.scrollOffset = 0;
                    continue;
                }

                if (showAllBtn.getGlobalBounds().contains(pos))
                {
                    showAllPaths = !showAllPaths;
                    continue;
                }

                if (modeSelect.selected == "All Possible Paths" && allPaths.size() > 1)
                {
                    if (prevBtn.getGlobalBounds().contains(pos))
                        currentPathIndex = (currentPathIndex - 1 + allPaths.size()) % allPaths.size();
                    if (nextBtn.getGlobalBounds().contains(pos))
                        currentPathIndex = (currentPathIndex + 1) % allPaths.size();
                }

                auto pick = [&](Dropdown &dd, string &target)
                {
                    if (!dd.open)
                        return false;
                    int start = (int)(dd.scrollOffset / dd.itemHeight);
                    for (int i = 0; i < dd.maxVisible && start + i < (int)dd.items.size(); ++i)
                    {
                        FloatRect r(dd.x + 8, dd.y + 8 + i * dd.itemHeight, dd.width - 16, dd.itemHeight - 8);
                        if (r.contains(pos))
                        {
                            target = dd.items[start + i];
                            dd.open = false;
                            activeDropdown = nullptr;
                            dd.scrollOffset = 0;
                            allPaths.clear();
                            currentPathIndex = 0;
                            detailsScrollOffset = 0;
                            detailsPanelOpen = false;
                            return true;
                        }
                    }
                    return false;
                };

                pick(dateSelect.dropdown, dateSelect.selected) ||
                    pick(fromSelect.dropdown, fromSelect.selected) ||
                    pick(toSelect.dropdown, toSelect.selected) ||
                    pick(modeSelect.dropdown, modeSelect.selected);
            }
        }

        auto updateDrop = [&](Dropdown &dd, bool wantOpen)
        {
            dd.open = wantOpen;
            dd.alpha += (wantOpen ? 30.f : -30.f) * dt;
            dd.alpha = clamp(dd.alpha, 0.f, 1.f);
            if (!wantOpen && dd.alpha < 0.01f)
                return;

            dd.x = panelX + 20;
            dd.y = (&dd == &dateSelect.dropdown) ? 120 : (&dd == &fromSelect.dropdown) ? 220
                                                     : (&dd == &toSelect.dropdown)     ? 320
                                                     : (&dd == &modeSelect.dropdown)   ? 420
                                                                                       : 0;
            dd.width = panelWidth - 40;
            dd.items = (&dd == &modeSelect.dropdown) ? modeSelect.dropdown.items : (&dd == &dateSelect.dropdown) ? days
                                                                                                                 : allPorts;

            dd.hovered = -1;
            int start = (int)(dd.scrollOffset / dd.itemHeight);
            for (int i = 0; i < dd.maxVisible && start + i < (int)dd.items.size(); ++i)
            {
                FloatRect r(dd.x + 8, dd.y + 8 + i * dd.itemHeight, dd.width - 16, dd.itemHeight - 8);
                if (r.contains(mouse))
                {
                    dd.hovered = start + i;
                    break;
                }
            }
        };

        if (panelOpen)
        {
            updateDrop(dateSelect.dropdown, dateSelect.dropdown.open);
            updateDrop(fromSelect.dropdown, fromSelect.dropdown.open);
            updateDrop(toSelect.dropdown, toSelect.dropdown.open);
            updateDrop(modeSelect.dropdown, modeSelect.dropdown.open);
        }

        if (panelOpen && !fromSelect.selected.empty() && !toSelect.selected.empty())
        {
            bool needRecalc = allPaths.empty() ||
                              (modeSelect.selected == "All Possible Paths" && allPaths.size() <= 1) ||
                              (modeSelect.selected == "Optimal Path");

            if (needRecalc)
            {
                allPaths.clear();
                if (modeSelect.selected == "All Possible Paths")
                    allPaths = g.allValidPaths(fromSelect.selected, toSelect.selected, dateSelect.selected);
                else
                {
                    auto [edges, nodes] = g.dijkstraPath(fromSelect.selected, toSelect.selected, dateSelect.selected);
                    if (!edges.empty())
                        allPaths.push_back({edges, nodes});
                }
                currentPathIndex = 0;
                detailsScrollOffset = 0;
            }
        }

        if (state == STARTER)
        {
            if (starterClock.getElapsedTime().asSeconds() > 10.f)
                state = NORMAL;
            window.clear(Color(20, 20, 40));
            window.draw(starterSprite);
            window.draw(continueBtn);
            window.draw(continueTxt);
            window.draw(minBtn);
            window.draw(minTxt);
            window.draw(closeBtn);
            window.draw(closeTxt);
            window.display();
            continue;
        }

        window.clear(Color(10, 15, 30));
        window.draw(mapSprite);

        if (showAllPaths)
        {
            for (PortNode *v : g)
            {
                float x1 = mapX(v->portData.lon), y1 = mapY(v->portData.lat);
                for (Edge *e = v->head; e; e = e->next)
                {
                    PortNode *d = g.findVertexByName(e->dest);
                    if (d)
                        drawArrow(window, x1, y1, mapX(d->portData.lon), mapY(d->portData.lat), Color(255, 255, 255, 80), 1.f);
                }
            }
        }

        for (auto v : g)
        {
            float x = mapX(v->portData.lon), y = mapY(v->portData.lat);
            CircleShape c(6);
            c.setFillColor((v->name == fromSelect.selected || v->name == toSelect.selected) ? Color::Red : Color(80, 90, 170));
            c.setPosition(x - 6, y - 6);
            window.draw(c);

            Text t(v->name, font, 14);
            t.setFillColor(Color::Black);
            t.setPosition(v->portData.left ? x - 12 - t.getLocalBounds().width : x + 12, y - 8);
            window.draw(t);
        }

        if (!allPaths.empty())
        {
            auto &path = allPaths[currentPathIndex];
            for (auto &e : path.first)
            {
                PortNode *to = g.findVertexByName(e.second->dest);
                if (to)
                    drawArrow(window,
                              mapX(e.first->portData.lon), mapY(e.first->portData.lat),
                              mapX(to->portData.lon), mapY(to->portData.lat),
                              modeSelect.selected == "Optimal Path" ? Color::Yellow : Color::Cyan, 5.f);
            }
        }

        RectangleShape panel(Vector2f(panelWidth, dm.height));
        panel.setFillColor(Color(30, 30, 30, 245));
        panel.setPosition(panelX, 0);
        window.draw(panel);

        setupInputBox(toggleBtn, toggleTxt, font, panelOpen ? "<" : ">", Vector2f(panelX + panelWidth, dm.height / 2 - 40), Vector2f(40, 80), Color(60, 60, 60), Color::White, 40);
        window.draw(toggleBtn);
        window.draw(toggleTxt);
        window.draw(minBtn);
        window.draw(minTxt);
        window.draw(closeBtn);
        window.draw(closeTxt);

        if (panelOpen)
        {
            // === INPUT FIELDS (unchanged) ===
            dateSelect.box.setPosition(panelX + 20, 100);
            fromSelect.box.setPosition(panelX + 20, 200);
            toSelect.box.setPosition(panelX + 20, 300);
            modeSelect.box.setPosition(panelX + 20, 400);

            dateSelect.box.setFillColor(dateSelect.dropdown.open ? Color(70, 70, 90) : Color(90, 90, 150));
            fromSelect.box.setFillColor(fromSelect.dropdown.open ? Color(70, 70, 90) : fromSelect.selected.empty() ? Color(50, 50, 50)
                                                                                                                   : Color(90, 90, 150));
            toSelect.box.setFillColor(toSelect.dropdown.open ? Color(70, 70, 90) : toSelect.selected.empty() ? Color(50, 50, 50)
                                                                                                             : Color(90, 90, 150));
            modeSelect.box.setFillColor(modeSelect.dropdown.open ? Color(70, 70, 90) : Color(90, 90, 150));

            dateSelect.box.setOutlineThickness(2);
            fromSelect.box.setOutlineThickness(2);
            toSelect.box.setOutlineThickness(2);
            modeSelect.box.setOutlineThickness(2);

            dateSelect.box.setOutlineColor(dateSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));
            fromSelect.box.setOutlineColor(fromSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));
            toSelect.box.setOutlineColor(toSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));
            modeSelect.box.setOutlineColor(modeSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));

            window.draw(dateSelect.box);
            window.draw(fromSelect.box);
            window.draw(toSelect.box);
            window.draw(modeSelect.box);

            dateSelect.text = Text(dateSelect.selected, font, 20);
            dateSelect.text.setFillColor(Color::White);
            dateSelect.text.setPosition(panelX + 35, 113);
            fromSelect.text = Text(fromSelect.selected.empty() ? fromSelect.placeholder : fromSelect.selected, font, 20);
            fromSelect.text.setFillColor(fromSelect.selected.empty() ? Color(150, 150, 150) : Color::White);
            fromSelect.text.setPosition(panelX + 35, 213);
            toSelect.text = Text(toSelect.selected.empty() ? toSelect.placeholder : toSelect.selected, font, 20);
            toSelect.text.setFillColor(toSelect.selected.empty() ? Color(150, 150, 150) : Color::White);
            toSelect.text.setPosition(panelX + 35, 313);
            modeSelect.text = Text(modeSelect.selected, font, 20);
            modeSelect.text.setFillColor(Color::White);
            modeSelect.text.setPosition(panelX + 35, 413);

            window.draw(dateSelect.text);
            window.draw(fromSelect.text);
            window.draw(toSelect.text);
            window.draw(modeSelect.text);

            dateSelect.arrow.setPosition(panelX + panelWidth - 60, 108);
            fromSelect.arrow.setPosition(panelX + panelWidth - 60, 208);
            toSelect.arrow.setPosition(panelX + panelWidth - 60, 308);
            modeSelect.arrow.setPosition(panelX + panelWidth - 60, 408);
            window.draw(dateSelect.arrow);
            window.draw(fromSelect.arrow);
            window.draw(toSelect.arrow);
            window.draw(modeSelect.arrow);

            Text dl("Date:", font, 24);
            dl.setFillColor(Color::White);
            dl.setPosition(panelX + 20, 70);
            window.draw(dl);
            Text fl("From:", font, 24);
            fl.setFillColor(Color::White);
            fl.setPosition(panelX + 20, 170);
            window.draw(fl);
            Text tl("To:", font, 24);
            tl.setFillColor(Color::White);
            tl.setPosition(panelX + 20, 270);
            window.draw(tl);
            Text ml("Mode:", font, 24);
            ml.setFillColor(Color::White);
            ml.setPosition(panelX + 20, 370);
            window.draw(ml);

            if (modeSelect.selected == "All Possible Paths" && !allPaths.empty())
            {
                Text counter(to_string(currentPathIndex + 1) + "/" + to_string(allPaths.size()), font, 18);
                counter.setFillColor(Color::White);
                counter.setPosition(panelX + 35, 470);
                window.draw(counter);

                prevBtn.setPosition(panelX + 20, 500);
                nextBtn.setPosition(panelX + panelWidth - 70, 500);
                prevBtn.setFillColor(Color(70, 70, 70));
                nextBtn.setFillColor(Color(70, 70, 70));
                window.draw(prevBtn);
                window.draw(nextBtn);
                prevTxt.setPosition(panelX + 35, 505);
                nextTxt.setPosition(panelX + panelWidth - 55, 505);
                window.draw(prevTxt);
                window.draw(nextTxt);
            }

            // NEW: Route Details Toggle Button (only shown when path exists and details not open)
            if (!allPaths.empty() && !detailsPanelOpen)
            {
                // detailsToggleBtn.setPosition(panelX + 20, 520);
                setupInputBox(detailsToggleBtn, detailsToggleTxt, font, "Route Details >", Vector2f(panelX + 20, 560), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);

                window.draw(detailsToggleBtn);
                window.draw(detailsToggleTxt);
            }

            // NEW: Full Route Details Sub-Panel
            if (detailsPanelOpen && !allPaths.empty())
            {
                // Background overlay
                RectangleShape overlay(Vector2f(panelWidth, dm.height));
                overlay.setPosition(panelX, 0);
                overlay.setFillColor(Color(20, 20, 30, 250));
                window.draw(overlay);

                // Back button
                // detailsBackBtn.setPosition(panelX + 20, 20);
                //  setupInputBox(detailsToggleBtn, detailsToggleTxt, font, "Route Details >", Vector2f(0, 0), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);
                setupInputBox(detailsBackBtn, detailsBackTxt, font, "< Back", Vector2f(panelX + 20, 20), Vector2f(80, 50), Color(100, 60, 60), Color::White, 20);

                window.draw(detailsBackBtn);
                window.draw(detailsBackTxt);

                // Title
                Text title("Route Details", font, 28);
                title.setFillColor(Color::White);
                title.setStyle(Text::Bold);
                title.setPosition(panelX + 20, 80);
                window.draw(title);

                // Scrollable content
                detailsBox = RectangleShape(Vector2f(panelWidth - 40, dm.height - 200));
                detailsBox.setPosition(panelX + 20, 130);
                detailsBox.setFillColor(Color(40, 40, 50, 240));
                window.draw(detailsBox);

                FloatRect view(panelX + 30, 140, panelWidth - 60, dm.height - 220);
                float y = 140 - detailsScrollOffset;

                auto &path = allPaths[currentPathIndex];
                int edges = path.first.size();
                int totalCost = 0;
                int totalMinutes = 0;

                for (int i = 0; i < edges; ++i)
                {
                    PortNode *from = path.first[i].first;
                    Edge *e = path.first[i].second;
                    PortNode *to = g.findVertexByName(e->dest);

                    int depMin = timeToMinutes(e->dep);
                    int arrMin = timeToMinutes(e->arr);
                    if (arrMin < depMin)
                        arrMin += 1440;
                    int duration = arrMin - depMin;

                    totalCost += e->cost;
                    totalMinutes += duration;

                    if (y + 85 > view.top && y < view.top + view.height)
                    {
                        RectangleShape rowBg(Vector2f(panelWidth - 60, detailsLineHeight - 10));
                        rowBg.setPosition(panelX + 30, y + 5);
                        rowBg.setFillColor(i % 2 == 0 ? Color(50, 50, 60, 180) : Color(45, 45, 55, 180));
                        window.draw(rowBg);

                        Text leg(from->name + " to " + to->name, font, 18);
                        leg.setFillColor(Color::White);
                        leg.setPosition(panelX + 40, y + 8);
                        window.draw(leg);

                        Text timeInfo(e->date + " | " + e->dep + " to " + e->arr + " (" + minutesToString(duration) + ")", font, 16);
                        timeInfo.setFillColor(Color(200, 200, 200));
                        timeInfo.setPosition(panelX + 40, y + 30);
                        window.draw(timeInfo);

                        Text costComp(e->company + " - $" + to_string(e->cost), font, 16);
                        costComp.setFillColor(Color(180, 255, 180));
                        costComp.setPosition(panelX + 40, y + 50);
                        window.draw(costComp);
                    }
                    y += detailsLineHeight;
                }

                Text total("TOTAL: $" + to_string(totalCost) + " | Duration: " + minutesToString(totalMinutes), font, 22);
                total.setFillColor(Color::White);
                total.setStyle(Text::Bold);
                total.setPosition(panelX + 35, y + 20);
                if (y + 60 < view.top + view.height)
                    window.draw(total);

                if (edges > 0)
                {
                    string arrival = "Final Arrival: " + path.first.back().second->date + " at " + path.first.back().second->arr;
                    Text arrive(arrival, font, 18);
                    arrive.setFillColor(Color(200, 255, 200));
                    arrive.setPosition(panelX + 35, y + 60);
                    if (y + 100 < view.top + view.height)
                        window.draw(arrive);
                }

                // Scrollbar
                float contentHeight = edges * detailsLineHeight + 300;
                if (contentHeight > dm.height - 200)
                {
                    float trackH = dm.height - 260;
                    detailsScrollTrack = RectangleShape(Vector2f(10, trackH));
                    detailsScrollTrack.setPosition(panelX + panelWidth - 35, 160);
                    detailsScrollTrack.setFillColor(Color(70, 70, 80, 180));
                    window.draw(detailsScrollTrack);

                    float ratio = (dm.height - 200) / contentHeight;
                    float thumbH = max(50.f, ratio * trackH);
                    float maxTravel = trackH - thumbH;
                    float scrollable = contentHeight - (dm.height - 200);
                    float thumbY = 160 + (detailsScrollOffset / scrollable) * maxTravel;

                    detailsScrollThumb = RectangleShape(Vector2f(10, thumbH));
                    detailsScrollThumb.setFillColor(Color(150, 150, 200));
                    detailsScrollThumb.setPosition(panelX + panelWidth - 35, thumbY);
                    window.draw(detailsScrollThumb);
                }
            }

            setupInputBox(showAllBtn, showAllTxt, font, "Show all routes",
                          Vector2f(panelX + 20, dm.height - 100), Vector2f(panelWidth - 40, 50),
                          showAllPaths ? Color(100, 100, 200) : Color(70, 70, 70), Color::White, 18);
            window.draw(showAllBtn);
            window.draw(showAllTxt);
        }

        auto drawDrop = [&](const Dropdown &dd)
        {
            if (!dd.open || dd.alpha < 0.01f)
                return;
            RectangleShape shadow(Vector2f(dd.width + 16, dd.itemHeight * dd.maxVisible + 20));
            shadow.setPosition(dd.x - 8, dd.y + 10);
            shadow.setFillColor(Color(0, 0, 0, 130));
            window.draw(shadow);

            RectangleShape bg(Vector2f(dd.width, dd.itemHeight * dd.maxVisible + 10));
            bg.setPosition(dd.x, dd.y);
            bg.setFillColor(Color(45, 45, 75, (uint8_t)(255 * dd.alpha)));
            bg.setOutlineThickness(2);
            bg.setOutlineColor(Color(100, 150, 255, (uint8_t)(220 * dd.alpha)));
            window.draw(bg);

            int start = (int)(dd.scrollOffset / dd.itemHeight);
            for (int i = 0; i < dd.maxVisible && start + i < (int)dd.items.size(); ++i)
            {
                int idx = start + i;
                float iy = dd.y + 5 + i * dd.itemHeight;
                if (idx == dd.hovered)
                {
                    RectangleShape h(Vector2f(dd.width - 10, dd.itemHeight - 8));
                    h.setPosition(dd.x + 5, iy);
                    h.setFillColor(Color(70, 130, 255, 180));
                    window.draw(h);
                }
                Text t(dd.items[idx], font, 19);
                t.setFillColor(Color::White);
                t.setPosition(dd.x + 16, iy + 8);
                window.draw(t);
            }
        };

        drawDrop(dateSelect.dropdown);
        drawDrop(fromSelect.dropdown);
        drawDrop(toSelect.dropdown);
        drawDrop(modeSelect.dropdown);

        window.display();
    }

    return 0;
}