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

// Helper function to display total cost & duration of a path
void drawPathSummary(RenderWindow &window, const Font &font,
                     const vector<PathStep> &path, float panelX, float panelWidth, float y)
{
    int totalCost = 0;
    long long totalMinutes = 0;

    for (auto &step : path)
    {
        bool isTravel = (!step.isWaiting && step.edge != nullptr);
        bool isWait = step.isWaiting;

        long long duration = step.arriveTime - step.departTime;
        if (duration < 0)
            duration += 1440;

        if (isTravel)
            totalCost += step.travelCost;
        else if (isWait)
            totalCost += step.waitingCost;

        totalMinutes += duration;
    }

    string summary = "Cost: $" + to_string(totalCost) +
                     "\nDuration: " + minutesToString(totalMinutes);

    Text text(summary, font, 16);
    text.setFillColor(Color::White);

    FloatRect bounds = text.getLocalBounds();
    float centerX = panelX + (panelWidth - bounds.width) / 2.f;
    text.setPosition(centerX, y);

    window.draw(text);
}

// Dropdown & SelectBox
struct Dropdown
{
    vector<string> items;
    vector<bool> selected; // For multi-select
    float x = 0, y = 0, width = 300.f, itemHeight = 44.f;
    int maxVisible = 8;
    float alpha = 0.f, scrollOffset = 0.f;
    int hovered = -1;
    bool open = false;
    bool multiSelect = false;

    FloatRect getBounds() const { return FloatRect(x, y, width, itemHeight * maxVisible + 10); }

    void clearSelection()
    {
        fill(selected.begin(), selected.end(), false);
    }

    vector<string> getSelectedItems() const
    {
        vector<string> result;
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (selected[i])
            {
                result.push_back(items[i]);
            }
        }
        return result;
    }

    string getDisplayText() const
    {
        auto selectedItems = getSelectedItems();
        if (selectedItems.empty())
            return "";

        string result;
        for (size_t i = 0; i < min(selectedItems.size(), size_t(2)); ++i)
        {
            if (i > 0)
                result += ", ";
            result += selectedItems[i];
        }
        if (selectedItems.size() > 2)
        {
            result += " ... (+" + to_string(selectedItems.size() - 2) + ")";
        }
        return result;
    }
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
vector<vector<PathStep>> allPaths;

// Scrollable details panel
RectangleShape detailsBox, detailsScrollTrack;
RectangleShape detailsScrollThumb;
float detailsScrollOffset = 0.f;
const float detailsLineHeight = 78.f;

// Customization panel variables
vector<string> preferredCompanies;
vector<string> avoidPortsList;
long long maxVoyageMinutes = 0;
string maxVoyageInput = "";
bool applyCustomization = false;

// Input boxes for customization
struct InputBox
{
    RectangleShape box;
    Text text;
    string content;
    string placeholder;
    bool active = false;
};

InputBox maxVoyageInputBox;

// Customization dropdowns
SelectBox companiesSelect, avoidPortsSelect;

int main()
{
    VideoMode dm = VideoMode::getDesktopMode();
    RenderWindow window(VideoMode(dm.width, dm.height), "Port Map with Directed Routes", Style::Fullscreen);
    window.setFramerateLimit(60);

    Font font;
    if (!font.loadFromFile("Font/dejavu-sans.book.ttf"))
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
    mapSprite.setColor(Color(150, 150, 150));
    starterSprite.setScale(float(dm.width) / starterTexture.getSize().x, float(dm.height) / starterTexture.getSize().y);

    float panelWidth = dm.width * 0.20f;
    float panelX = -panelWidth;
    bool panelOpen = false;
    float slideSpeed = 1200.f;

    // Collapsible Route Details Panel with smooth slide
    bool detailsPanelOpen = false;
    bool detailsPanelClosing = false;
    float detailsPanelOffset = -panelWidth;
    const float detailsSlideSpeed = 1800.f;

    // Customize Panel variables
    bool customizePanelOpen = false;
    bool customizePanelClosing = false;
    float customizePanelOffset = -panelWidth;
    const float customizeSlideSpeed = 1800.f;

    RectangleShape toggleBtn, minBtn, closeBtn, continueBtn, showAllBtn;
    Text toggleTxt, minTxt, closeTxt, continueTxt, showAllTxt;

    setupInputBox(toggleBtn, toggleTxt, font, ">", Vector2f(0, dm.height / 2 - 40), Vector2f(40, 80), Color(60, 60, 60), Color::White, 40);
    setupInputBox(minBtn, minTxt, font, "-", Vector2f(dm.width - 100, 10), Vector2f(40, 40), Color(120, 120, 120), Color::White, 30);
    setupInputBox(closeBtn, closeTxt, font, "X", Vector2f(dm.width - 50, 10), Vector2f(40, 40), Color(180, 50, 50), Color::White, 26);
    setupInputBox(continueBtn, continueTxt, font, "Continue", Vector2f(dm.width / 2 - 150, dm.height / 2 - 35), Vector2f(300, 70), Color(255, 255, 255, 180), Color::Black, 28);

    RectangleShape detailsToggleBtn, detailsBackBtn;
    Text detailsToggleTxt, detailsBackTxt;

    setupInputBox(detailsToggleBtn, detailsToggleTxt, font, "Route Details >", Vector2f(0, 0), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);
    setupInputBox(detailsBackBtn, detailsBackTxt, font, "< Back", Vector2f(0, 0), Vector2f(80, 50), Color(100, 60, 60), Color::White, 20);

    RectangleShape customizeToggleBtn, customizeBackBtn, applyCustomizationBtn, resetCustomizationBtn;
    Text customizeToggleTxt, customizeBackTxt, applyCustomizationTxt, resetCustomizationTxt;

    setupInputBox(customizeToggleBtn, customizeToggleTxt, font, "Customize Path >", Vector2f(0, 0), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);
    setupInputBox(customizeBackBtn, customizeBackTxt, font, "< Back", Vector2f(0, 0), Vector2f(80, 50), Color(100, 60, 60), Color::White, 20);
    setupInputBox(applyCustomizationBtn, applyCustomizationTxt, font, "Apply Filters", Vector2f(0, 0), Vector2f(panelWidth - 100, 50), Color(70, 120, 70), Color::White, 18);
    setupInputBox(resetCustomizationBtn, resetCustomizationTxt, font, "Reset", Vector2f(0, 0), Vector2f(80, 50), Color(120, 70, 70), Color::White, 18);

    // Initialize customization dropdowns
    companiesSelect.placeholder = "Select companies...";
    companiesSelect.box = RectangleShape(Vector2f(panelWidth - 60, 40));
    companiesSelect.arrow = Text("V", font, 28);
    companiesSelect.arrow.setFillColor(Color(180, 180, 180));
    companiesSelect.dropdown.multiSelect = true;

    avoidPortsSelect.placeholder = "Select ports to avoid...";
    avoidPortsSelect.box = RectangleShape(Vector2f(panelWidth - 60, 40));
    avoidPortsSelect.arrow = Text("V", font, 28);
    avoidPortsSelect.arrow.setFillColor(Color(180, 180, 180));
    avoidPortsSelect.dropdown.multiSelect = true;

    // Initialize max voyage input box
    maxVoyageInputBox.box = RectangleShape(Vector2f(panelWidth - 60, 40));
    maxVoyageInputBox.box.setFillColor(Color(50, 50, 70));
    maxVoyageInputBox.box.setOutlineThickness(2);
    maxVoyageInputBox.box.setOutlineColor(Color(80, 80, 100));
    maxVoyageInputBox.placeholder = "e.g., 480 (8 hours)";
    maxVoyageInputBox.text = Text("", font, 16);
    maxVoyageInputBox.text.setFillColor(Color::White);

    fromSelect.placeholder = "Select departure port...";
    toSelect.placeholder = "Select destination port...";
    modeSelect.placeholder = "All Possible Paths";
    modeSelect.selected = "All Possible Paths";

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

    modeSelect.dropdown.items = {"All Possible Paths", "Optimal Path", "Fastest Path"};

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

    // Get all companies from graph
    vector<string> allCompanies = g.getAllCompanyNames();

    // Initialize dropdown items
    companiesSelect.dropdown.items = allCompanies;
    companiesSelect.dropdown.selected.resize(allCompanies.size(), false);

    avoidPortsSelect.dropdown.items = allPorts;
    avoidPortsSelect.dropdown.selected.resize(allPorts.size(), false);

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

        {
            if (panelOpen && panelX < 0)
                panelX += slideSpeed * dt;
            if (!panelOpen && panelX > -panelWidth)
                panelX -= slideSpeed * dt;
            panelX = clamp(panelX, -panelWidth, 0.f);
        }

        // Smooth slide for details panel
        {
            if (detailsPanelOpen && !detailsPanelClosing && detailsPanelOffset < 0)
                detailsPanelOffset += detailsSlideSpeed * dt;

            if (detailsPanelClosing && detailsPanelOffset > -panelWidth)
                detailsPanelOffset -= detailsSlideSpeed * dt;

            if (detailsPanelClosing && detailsPanelOffset <= -panelWidth + 10.f)
            {
                detailsPanelOpen = false;
                detailsPanelClosing = false;
                detailsPanelOffset = -panelWidth;
            }

            detailsPanelOffset = clamp(detailsPanelOffset, -panelWidth, 0.f);
        }

        // Smooth slide for customize panel
        {
            if (customizePanelOpen && !customizePanelClosing && customizePanelOffset < 0)
                customizePanelOffset += customizeSlideSpeed * dt;

            if (customizePanelClosing && customizePanelOffset > -panelWidth)
                customizePanelOffset -= customizeSlideSpeed * dt;

            if (customizePanelClosing && customizePanelOffset <= -panelWidth + 10.f)
            {
                customizePanelOpen = false;
                customizePanelClosing = false;
                customizePanelOffset = -panelWidth;
            }

            customizePanelOffset = clamp(customizePanelOffset, -panelWidth, 0.f);
        }

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
                else if (companiesSelect.dropdown.open && companiesSelect.dropdown.getBounds().contains(mouse))
                    active = &companiesSelect.dropdown;
                else if (avoidPortsSelect.dropdown.open && avoidPortsSelect.dropdown.getBounds().contains(mouse))
                    active = &avoidPortsSelect.dropdown;

                if (active && active->items.size() > size_t(active->maxVisible))
                {
                    active->scrollOffset -= event.mouseWheelScroll.delta * 35.f;
                    float maxScroll = (active->items.size() - active->maxVisible) * active->itemHeight;
                    active->scrollOffset = clamp(active->scrollOffset, 0.f, max(0.f, maxScroll));
                }

                if (detailsPanelOpen && detailsBox.getGlobalBounds().contains(mouse))
                {
                    int steps = allPaths.empty() ? 0 : allPaths[currentPathIndex].size();
                    float contentHeight = steps * detailsLineHeight + 300;
                    float viewHeight = dm.height - 220;

                    if (contentHeight > viewHeight)
                    {
                        float maxOffset = contentHeight - viewHeight;
                        detailsScrollOffset -= event.mouseWheelScroll.delta * 45.f;
                        detailsScrollOffset = clamp(detailsScrollOffset, 0.f, maxOffset);
                    }
                }
            }

            // Handle text input for max voyage time
            if (event.type == Event::TextEntered && (customizePanelOpen || customizePanelClosing))
            {
                if (maxVoyageInputBox.active)
                {
                    if (event.text.unicode == '\b')
                    {
                        if (!maxVoyageInputBox.content.empty())
                            maxVoyageInputBox.content.pop_back();
                    }
                    else if (event.text.unicode >= 32 && event.text.unicode < 128)
                    {
                        // Only allow digits
                        if (event.text.unicode >= '0' && event.text.unicode <= '9')
                        {
                            maxVoyageInputBox.content += static_cast<char>(event.text.unicode);
                        }
                    }
                    maxVoyageInputBox.text.setString(maxVoyageInputBox.content);
                }
            }

            if (event.type == Event::MouseButtonPressed &&
                event.mouseButton.button == Mouse::Left &&
                clickTimer <= 0.f)
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
                    fromSelect.dropdown.open = toSelect.dropdown.open =
                        modeSelect.dropdown.open = dateSelect.dropdown.open = false;
                    companiesSelect.dropdown.open = avoidPortsSelect.dropdown.open = false;
                    activeDropdown = nullptr;
                    detailsPanelOpen = false;
                    customizePanelOpen = false;
                    continue;
                }

                if (!panelOpen)
                    continue;

                // Handle customization panel interactions - ONLY if customization panel is actually open
                if (customizePanelOpen && !customizePanelClosing)
                {
                    // Check input box clicks
                    maxVoyageInputBox.active = maxVoyageInputBox.box.getGlobalBounds().contains(pos);
                    maxVoyageInputBox.box.setOutlineColor(maxVoyageInputBox.active ? Color(100, 180, 255) : Color(80, 80, 100));

                    // Check dropdown clicks
                    bool hitCompanies = companiesSelect.box.getGlobalBounds().contains(pos);
                    bool hitAvoidPorts = avoidPortsSelect.box.getGlobalBounds().contains(pos);

                    if (hitCompanies && activeDropdown == nullptr)
                    {
                        companiesSelect.dropdown.open = !companiesSelect.dropdown.open;
                        avoidPortsSelect.dropdown.open = false;
                        activeDropdown = companiesSelect.dropdown.open ? &companiesSelect : nullptr;
                        companiesSelect.dropdown.scrollOffset = 0;
                        continue;
                    }
                    if (hitAvoidPorts && activeDropdown == nullptr)
                    {
                        avoidPortsSelect.dropdown.open = !avoidPortsSelect.dropdown.open;
                        companiesSelect.dropdown.open = false;
                        activeDropdown = avoidPortsSelect.dropdown.open ? &avoidPortsSelect : nullptr;
                        avoidPortsSelect.dropdown.scrollOffset = 0;
                        continue;
                    }

                    // Handle dropdown item selection for multi-select
                    auto handleMultiSelect = [&](SelectBox &selectBox)
                    {
                        if (!selectBox.dropdown.open)
                            return false;

                        int start = (int)(selectBox.dropdown.scrollOffset / selectBox.dropdown.itemHeight);

                        for (int i = 0; i < selectBox.dropdown.maxVisible && start + i < (int)selectBox.dropdown.items.size(); ++i)
                        {
                            FloatRect r(selectBox.dropdown.x + 8, selectBox.dropdown.y + 8 + i * selectBox.dropdown.itemHeight,
                                        selectBox.dropdown.width - 16, selectBox.dropdown.itemHeight - 8);

                            if (r.contains(pos))
                            {
                                int idx = start + i;
                                selectBox.dropdown.selected[idx] = !selectBox.dropdown.selected[idx];
                                selectBox.selected = selectBox.dropdown.getDisplayText();
                                return true;
                            }
                        }
                        return false;
                    };

                    if (handleMultiSelect(companiesSelect) || handleMultiSelect(avoidPortsSelect))
                    {
                        continue;
                    }

                    // Close customization dropdowns if clicking outside
                    bool clickedInsideCompanies = companiesSelect.dropdown.open && companiesSelect.dropdown.getBounds().contains(pos);
                    bool clickedInsideAvoidPorts = avoidPortsSelect.dropdown.open && avoidPortsSelect.dropdown.getBounds().contains(pos);

                    if (!clickedInsideCompanies && !clickedInsideAvoidPorts &&
                        !hitCompanies && !hitAvoidPorts &&
                        (companiesSelect.dropdown.open || avoidPortsSelect.dropdown.open))
                    {
                        companiesSelect.dropdown.open = false;
                        avoidPortsSelect.dropdown.open = false;
                        activeDropdown = nullptr;
                        continue;
                    }

                    if (customizeBackBtn.getGlobalBounds().contains(pos))
                    {
                        customizePanelClosing = true;
                    }
                    else if (applyCustomizationBtn.getGlobalBounds().contains(pos))
                    {
                        // Get selected companies
                        preferredCompanies = companiesSelect.dropdown.getSelectedItems();

                        // Get selected avoid ports
                        avoidPortsList = avoidPortsSelect.dropdown.getSelectedItems();

                        // Parse max voyage minutes
                        if (!maxVoyageInputBox.content.empty())
                        {
                            try
                            {
                                maxVoyageMinutes = stoll(maxVoyageInputBox.content);
                            }
                            catch (...)
                            {
                                maxVoyageMinutes = 0;
                            }
                        }
                        else
                        {
                            maxVoyageMinutes = 0;
                        }

                        applyCustomization = true;

                        // Recalculate paths with filters
                        if (!fromSelect.selected.empty() && !toSelect.selected.empty())
                        {
                            allPaths = g.filteredPaths(
                                fromSelect.selected,
                                toSelect.selected,
                                dateSelect.selected,
                                preferredCompanies,
                                avoidPortsList,
                                maxVoyageMinutes);
                            currentPathIndex = 0;
                            detailsScrollOffset = 0;

                            // Force immediate UI update by clearing any pending panel states
                            if (allPaths.empty())
                            {
                                // Close details panel if no paths available
                                detailsPanelOpen = false;
                                detailsPanelClosing = false;
                                detailsPanelOffset = -panelWidth;
                            }
                        }
                    }
                    else if (resetCustomizationBtn.getGlobalBounds().contains(pos))
                    {
                        // Reset all inputs
                        companiesSelect.dropdown.clearSelection();
                        companiesSelect.selected = "";
                        avoidPortsSelect.dropdown.clearSelection();
                        avoidPortsSelect.selected = "";
                        maxVoyageInputBox.content = "";
                        maxVoyageInputBox.text.setString("");
                        preferredCompanies.clear();
                        avoidPortsList.clear();
                        maxVoyageMinutes = 0;
                        applyCustomization = false;

                        // Recalculate paths without filters
                        if (!fromSelect.selected.empty() && !toSelect.selected.empty())
                        {
                            if (modeSelect.selected == "All Possible Paths")
                                allPaths = g.allValidPaths(fromSelect.selected, toSelect.selected, dateSelect.selected);
                            else if (modeSelect.selected == "Optimal Path")
                            {
                                auto D_PATH = g.dijkstraPath(fromSelect.selected, toSelect.selected, dateSelect.selected);
                                allPaths.push_back(D_PATH);
                            }
                            else
                            {
                                auto D_PATH = g.dijkstraFastestPath(fromSelect.selected, toSelect.selected, dateSelect.selected);
                                allPaths.push_back(D_PATH);
                            }
                            currentPathIndex = 0;
                            detailsScrollOffset = 0;
                        }
                    }

                    // IMPORTANT: Only continue to other interactions if we haven't handled anything in customization panel
                    // This prevents blocking other UI elements when customization panel is open
                    bool handledCustomizationAction =
                        hitCompanies || hitAvoidPorts ||
                        companiesSelect.dropdown.open || avoidPortsSelect.dropdown.open ||
                        customizeBackBtn.getGlobalBounds().contains(pos) ||
                        applyCustomizationBtn.getGlobalBounds().contains(pos) ||
                        resetCustomizationBtn.getGlobalBounds().contains(pos) ||
                        maxVoyageInputBox.active;

                    if (handledCustomizationAction)
                    {
                        continue;
                    }
                }

                // BLOCK ALL OTHER BUTTONS WHEN DETAILS PANEL IS OPEN OR CLOSING
                if (detailsPanelOpen || detailsPanelClosing)
                {
                    if (detailsBackBtn.getGlobalBounds().contains(pos))
                    {
                        detailsPanelClosing = true;
                        detailsScrollOffset = 0.f;
                    }
                    continue;
                }

                // ------------------ BLOCK certain buttons if dropdown is open ------------------
                bool anyDropdownOpen =
                    fromSelect.dropdown.open ||
                    toSelect.dropdown.open ||
                    modeSelect.dropdown.open ||
                    dateSelect.dropdown.open ||
                    companiesSelect.dropdown.open ||
                    avoidPortsSelect.dropdown.open;

                if (anyDropdownOpen)
                {
                    // Check if click is inside any open dropdown or its corresponding box
                    bool clickedInsideDropdown = false;

                    if (fromSelect.dropdown.open && (fromSelect.dropdown.getBounds().contains(pos) || fromSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;
                    else if (toSelect.dropdown.open && (toSelect.dropdown.getBounds().contains(pos) || toSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;
                    else if (modeSelect.dropdown.open && (modeSelect.dropdown.getBounds().contains(pos) || modeSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;
                    else if (dateSelect.dropdown.open && (dateSelect.dropdown.getBounds().contains(pos) || dateSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;
                    else if (companiesSelect.dropdown.open && (companiesSelect.dropdown.getBounds().contains(pos) || companiesSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;
                    else if (avoidPortsSelect.dropdown.open && (avoidPortsSelect.dropdown.getBounds().contains(pos) || avoidPortsSelect.box.getGlobalBounds().contains(pos)))
                        clickedInsideDropdown = true;

                    if (!clickedInsideDropdown)
                    {
                        fromSelect.dropdown.open = toSelect.dropdown.open =
                            modeSelect.dropdown.open = dateSelect.dropdown.open =
                                companiesSelect.dropdown.open = avoidPortsSelect.dropdown.open = false;

                        activeDropdown = nullptr;
                        continue;
                    }
                }

                // ----------------- Dropdown open clicks -----------------
                bool hitFrom = fromSelect.box.getGlobalBounds().contains(pos);
                bool hitTo = toSelect.box.getGlobalBounds().contains(pos);
                bool hitMode = modeSelect.box.getGlobalBounds().contains(pos);
                bool hitDate = dateSelect.box.getGlobalBounds().contains(pos);

                if (hitDate && activeDropdown == nullptr)
                {
                    dateSelect.dropdown.open = !dateSelect.dropdown.open;
                    fromSelect.dropdown.open = toSelect.dropdown.open =
                        modeSelect.dropdown.open = companiesSelect.dropdown.open =
                            avoidPortsSelect.dropdown.open = false;

                    activeDropdown = dateSelect.dropdown.open ? &dateSelect : nullptr;
                    dateSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitFrom && activeDropdown == nullptr)
                {
                    fromSelect.dropdown.open = !fromSelect.dropdown.open;
                    toSelect.dropdown.open = modeSelect.dropdown.open =
                        dateSelect.dropdown.open = companiesSelect.dropdown.open =
                            avoidPortsSelect.dropdown.open = false;

                    activeDropdown = fromSelect.dropdown.open ? &fromSelect : nullptr;
                    fromSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitTo && activeDropdown == nullptr)
                {
                    toSelect.dropdown.open = !toSelect.dropdown.open;
                    fromSelect.dropdown.open = modeSelect.dropdown.open =
                        dateSelect.dropdown.open = companiesSelect.dropdown.open =
                            avoidPortsSelect.dropdown.open = false;

                    activeDropdown = toSelect.dropdown.open ? &toSelect : nullptr;
                    toSelect.dropdown.scrollOffset = 0;
                    continue;
                }
                if (hitMode && activeDropdown == nullptr)
                {
                    modeSelect.dropdown.open = !modeSelect.dropdown.open;
                    fromSelect.dropdown.open = toSelect.dropdown.open =
                        dateSelect.dropdown.open = companiesSelect.dropdown.open =
                            avoidPortsSelect.dropdown.open = false;

                    activeDropdown = modeSelect.dropdown.open ? &modeSelect : nullptr;
                    modeSelect.dropdown.scrollOffset = 0;
                    continue;
                }

                // ----------------- Only allow these if dropdown NOT open -----------------
                if (!anyDropdownOpen)
                {
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

                    if (!allPaths.empty() && detailsToggleBtn.getGlobalBounds().contains(pos))
                    {
                        detailsPanelOpen = true;
                        detailsScrollOffset = 0.f;
                        continue;
                    }

                    if (!allPaths.empty() && modeSelect.selected == "All Possible Paths" && customizeToggleBtn.getGlobalBounds().contains(pos))
                    {
                        customizePanelOpen = true;
                        continue;
                    }
                }

                // ----------------- Dropdown item picking -----------------
                auto pick = [&](Dropdown &dd, string &target)
                {
                    if (!dd.open)
                        return false;

                    int start = (int)(dd.scrollOffset / dd.itemHeight);

                    for (int i = 0; i < dd.maxVisible && start + i < (int)dd.items.size(); ++i)
                    {
                        FloatRect r(dd.x + 8, dd.y + 8 + i * dd.itemHeight,
                                    dd.width - 16, dd.itemHeight - 8);

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
                            customizePanelOpen = false;

                            // FIX: Reset customization when user changes selection
                            applyCustomization = false;

                            return true;
                        }
                    }
                    return false;
                };

                pick(dateSelect.dropdown, dateSelect.selected) ||
                    pick(fromSelect.dropdown, fromSelect.selected) ||
                    pick(toSelect.dropdown, toSelect.selected) ||
                    pick(modeSelect.dropdown, modeSelect.selected);

                if (modeSelect.selected != "All Possible Paths")
                {
                }
            }
        }

        auto updateDrop = [&](SelectBox &selectBox, bool wantOpen)
        {
            selectBox.dropdown.open = wantOpen;
            selectBox.dropdown.alpha += (wantOpen ? 30.f : -30.f) * dt;
            selectBox.dropdown.alpha = clamp(selectBox.dropdown.alpha, 0.f, 1.f);
            if (!wantOpen && selectBox.dropdown.alpha < 0.01f)
                return;

            selectBox.dropdown.x = customizePanelOffset + 30;
            selectBox.dropdown.width = panelWidth - 60;

            // Update display text
            selectBox.selected = selectBox.dropdown.getDisplayText();

            selectBox.dropdown.hovered = -1;
            int start = (int)(selectBox.dropdown.scrollOffset / selectBox.dropdown.itemHeight);
            for (int i = 0; i < selectBox.dropdown.maxVisible && start + i < (int)selectBox.dropdown.items.size(); ++i)
            {
                FloatRect r(selectBox.dropdown.x + 8, selectBox.dropdown.y + 8 + i * selectBox.dropdown.itemHeight,
                            selectBox.dropdown.width - 16, selectBox.dropdown.itemHeight - 8);
                if (r.contains(mouse))
                {
                    selectBox.dropdown.hovered = start + i;
                    break;
                }
            }
        };

        auto updateMainDrop = [&](Dropdown &dd, bool wantOpen, float yPos)
        {
            dd.open = wantOpen;
            dd.alpha += (wantOpen ? 30.f : -30.f) * dt;
            dd.alpha = clamp(dd.alpha, 0.f, 1.f);
            if (!wantOpen && dd.alpha < 0.01f)
                return;

            dd.x = panelX + 20;
            dd.y = yPos;
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
            updateMainDrop(dateSelect.dropdown, dateSelect.dropdown.open, 120);
            updateMainDrop(fromSelect.dropdown, fromSelect.dropdown.open, 220);
            updateMainDrop(toSelect.dropdown, toSelect.dropdown.open, 320);
            updateMainDrop(modeSelect.dropdown, modeSelect.dropdown.open, 420);
        }

        if (customizePanelOpen)
        {
            updateDrop(companiesSelect, companiesSelect.dropdown.open);
            updateDrop(avoidPortsSelect, avoidPortsSelect.dropdown.open);
        }

        if (panelOpen && !fromSelect.selected.empty() && !toSelect.selected.empty())
        {
            bool needRecalc = allPaths.empty();

            if (needRecalc && !applyCustomization)
            {
                allPaths.clear();
                if (modeSelect.selected == "All Possible Paths")
                    allPaths = g.allValidPaths(fromSelect.selected, toSelect.selected, dateSelect.selected);
                else if (modeSelect.selected == "Optimal Path")
                {
                    auto D_PATH = g.dijkstraPath(fromSelect.selected, toSelect.selected, dateSelect.selected);
                    allPaths.push_back(D_PATH);
                }
                else
                {
                    auto D_PATH = g.dijkstraFastestPath(fromSelect.selected, toSelect.selected, dateSelect.selected);
                    allPaths.push_back(D_PATH);
                }
                currentPathIndex = 0;
                detailsScrollOffset = 0;
            }

            // FIX: Reset customization if paths become empty after changing selection
            if (applyCustomization && allPaths.empty())
            {
                applyCustomization = false;
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

        // Draw all routes first (background)
        if (showAllPaths)
        {
            for (PortNode *v : g)
            {
                float x1 = mapX(v->portData.lon), y1 = mapY(v->portData.lat);
                for (Edge *e = v->head; e; e = e->next)
                {
                    PortNode *d = g.findVertexByName(e->dest);
                    if (d)
                    {
                        Color arrowColor = Color(255, 255, 255, 80); // Default semi-transparent white

                        // Check if this route should be avoided (either port is in avoid list)
                        if (find(avoidPortsList.begin(), avoidPortsList.end(), v->name) != avoidPortsList.end() ||
                            find(avoidPortsList.begin(), avoidPortsList.end(), d->name) != avoidPortsList.end())
                        {
                            arrowColor = Color(255, 100, 100, 120); // Light red for avoided routes
                        }
                        // Check if this route uses preferred company
                        else if (!preferredCompanies.empty() && applyCustomization &&
                                 find(preferredCompanies.begin(), preferredCompanies.end(), e->company) != preferredCompanies.end())
                        {
                            arrowColor = Color(100, 255, 100, 120); // Light green for preferred company routes
                        }
                        else if (!preferredCompanies.empty() && applyCustomization)
                        {
                            arrowColor = Color(255, 100, 100, 120);
                        }

                        drawArrow(window, x1, y1, mapX(d->portData.lon), mapY(d->portData.lat), arrowColor, 1.f);
                    }
                }
            }
        }

        // Draw ports on top
        for (auto v : g)
        {
            float x = mapX(v->portData.lon), y = mapY(v->portData.lat);
            CircleShape c(6);

            // Highlight avoided ports in red
            if (find(avoidPortsList.begin(), avoidPortsList.end(), v->name) != avoidPortsList.end())
            {
                c.setFillColor(Color::Red); // Avoided ports in red
            }
            else
            {
                c.setFillColor((v->name == fromSelect.selected || v->name == toSelect.selected) ? Color::Yellow : Color(80, 90, 170));
            }

            c.setPosition(x - 6, y - 6);
            window.draw(c);

            Text t(v->name, font, 14);
            t.setFillColor(Color::White);
            t.setPosition(v->portData.left ? x - 12 - t.getLocalBounds().width : x + 12, y - 8);
            window.draw(t);
        }

        // Draw selected path on top of everything
        // Draw selected path on top of everything - ONLY if paths exist
        if (!allPaths.empty() && currentPathIndex < allPaths.size())
        {
            auto &path = allPaths[currentPathIndex];

            for (auto &step : path)
            {
                if (step.isWaiting || step.to == nullptr)
                    continue;

                Color routeColor = Color::Cyan;
                if (applyCustomization)
                {
                    // Highlight filtered routes
                    if (step.edge && find(preferredCompanies.begin(), preferredCompanies.end(), step.edge->company) != preferredCompanies.end())
                    {
                        routeColor = Color::Green; // Preferred company routes in solid green
                    }
                    else if (modeSelect.selected == "Optimal Path")
                    {
                        routeColor = Color::Yellow;
                    }
                }
                else if (modeSelect.selected == "Optimal Path")
                {
                    routeColor = Color::Yellow;
                }

                drawArrow(
                    window,
                    mapX(step.from->portData.lon), mapY(step.from->portData.lat),
                    mapX(step.to->portData.lon), mapY(step.to->portData.lat),
                    routeColor,
                    5.f);
            }
        }
        else if (allPaths.empty() && applyCustomization)
        {
            // No paths available with current filters - ensure nothing is drawn
            // This prevents the previous path from remaining visible
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

            if (!allPaths.empty() && currentPathIndex < allPaths.size())
            {
                drawPathSummary(window, font, allPaths[currentPathIndex], panelX, panelWidth, 505);
            }
            else if (applyCustomization && allPaths.empty())
            {
                // Show "No paths found" message when filters result in no paths
                Text noPathText("No paths found with current filters", font, 16);
                noPathText.setFillColor(Color::Red);
                noPathText.setPosition(panelX + 35, 505);
                window.draw(noPathText);
            }

            if (!allPaths.empty() && !detailsPanelOpen && !customizePanelOpen)
            {
                setupInputBox(detailsToggleBtn, detailsToggleTxt, font, "Route Details >", Vector2f(panelX + 20, 560), Vector2f(panelWidth - 40, 50), Color(70, 70, 90), Color::White, 20);
                window.draw(detailsToggleBtn);
                window.draw(detailsToggleTxt);
            }

            if (!allPaths.empty() && !detailsPanelOpen && !customizePanelOpen)
            {
                setupInputBox(customizeToggleBtn, customizeToggleTxt, font, "Customize Path >",
                              Vector2f(panelX + 20, 620), Vector2f(panelWidth - 40, 50),
                              Color(70, 90, 70), Color::White, 20);
                window.draw(customizeToggleBtn);
                window.draw(customizeToggleTxt);
            }

            setupInputBox(showAllBtn, showAllTxt, font, "Show all routes",
                          Vector2f(panelX + 20, dm.height - 100), Vector2f(panelWidth - 40, 50),
                          showAllPaths ? Color(100, 100, 200) : Color(70, 70, 70), Color::White, 18);
            window.draw(showAllBtn);
            window.draw(showAllTxt);

            // DETAILS PANEL
            if ((detailsPanelOpen || detailsPanelClosing) && !allPaths.empty())
            {
                RectangleShape detailsOverlay(Vector2f(panelWidth, dm.height));
                detailsOverlay.setPosition(detailsPanelOffset, 0);
                detailsOverlay.setFillColor(Color(20, 20, 30, 250));
                window.draw(detailsOverlay);

                setupInputBox(detailsBackBtn, detailsBackTxt, font, "< Back",
                              Vector2f(detailsPanelOffset + 20, 20),
                              Vector2f(80, 50),
                              Color(100, 60, 60),
                              Color::White, 20);

                window.draw(detailsBackBtn);
                window.draw(detailsBackTxt);

                Text title("Route Details", font, 28);
                title.setFillColor(Color::White);
                title.setStyle(Text::Bold);
                title.setPosition(detailsPanelOffset + 20, 80);
                window.draw(title);

                detailsBox = RectangleShape(Vector2f(panelWidth - 40, dm.height - 200));
                detailsBox.setPosition(detailsPanelOffset + 20, 130);
                detailsBox.setFillColor(Color(40, 40, 50, 240));
                window.draw(detailsBox);

                FloatRect view(detailsPanelOffset + 30, 140, panelWidth - 60, dm.height - 220);

                float y = 140 - detailsScrollOffset;
                auto &path = allPaths[currentPathIndex];

                int totalCost = 0;
                long long totalMinutes = 0;

                for (int i = 0; i < path.size(); ++i)
                {
                    const PathStep &step = path[i];

                    bool isTravel = (!step.isWaiting && step.edge != nullptr);
                    bool isWait = step.isWaiting;

                    long long duration = step.arriveTime - step.departTime;
                    if (duration < 0)
                        duration += 1440;

                    if (isTravel)
                        totalCost += step.travelCost;
                    else if (isWait)
                        totalCost += step.waitingCost;

                    totalMinutes += duration;

                    if (y + 85 > view.top && y < view.top + view.height)
                    {
                        RectangleShape rowBg(Vector2f(panelWidth - 60, detailsLineHeight - 10));
                        rowBg.setPosition(detailsPanelOffset + 30, y + 5);
                        rowBg.setFillColor(i % 2 == 0 ? Color(50, 50, 60, 180) : Color(45, 45, 55, 180));
                        window.draw(rowBg);

                        if (isWait)
                        {
                            Text leg("WAIT at " + step.from->name, font, 18);
                            leg.setFillColor(Color(255, 200, 150));
                            leg.setPosition(detailsPanelOffset + 40, y + 8);
                            window.draw(leg);

                            Text timeInfo("Duration: " + minutesToString(duration), font, 16);
                            timeInfo.setFillColor(Color(200, 200, 200));
                            timeInfo.setPosition(detailsPanelOffset + 40, y + 32);
                            window.draw(timeInfo);

                            Text cost("Port Fee: $" + to_string(step.waitingCost), font, 16);
                            cost.setFillColor(Color(255, 170, 170));
                            cost.setPosition(detailsPanelOffset + 40, y + 52);
                            window.draw(cost);
                        }
                        else
                        {
                            Text leg(step.from->name + " -> " + step.to->name, font, 18);
                            leg.setFillColor(Color::White);
                            leg.setPosition(detailsPanelOffset + 40, y + 8);
                            window.draw(leg);

                            string dateStr = step.edge->date;
                            Text timeInfo(dateStr + " | " +
                                              minutesToTimeString(step.departTime) +
                                              " -> " +
                                              minutesToTimeString(step.arriveTime) +
                                              " (" + minutesToString(duration) + ")",
                                          font, 16);
                            timeInfo.setFillColor(Color(200, 200, 200));
                            timeInfo.setPosition(detailsPanelOffset + 40, y + 32);
                            window.draw(timeInfo);

                            Text cost(step.edge->company + " - $" + to_string(step.travelCost), font, 16);
                            cost.setFillColor(Color(180, 255, 180));
                            cost.setPosition(detailsPanelOffset + 40, y + 52);
                            window.draw(cost);
                        }
                    }
                    y += detailsLineHeight;
                }

                Text total("TOTAL: $" + to_string(totalCost) +
                               "\nDuration: " + minutesToString(totalMinutes),
                           font, 22);
                total.setFillColor(Color::White);
                total.setStyle(Text::Bold);
                total.setPosition(detailsPanelOffset + 35, y + 20);
                if (y + 60 < view.top + view.height)
                    window.draw(total);

                if (!path.empty() && path.back().edge)
                {
                    Text arrive("Final Arrival: " +
                                    path.back().edge->date +
                                    " at " +
                                    path.back().edge->arr,
                                font, 18);

                    arrive.setFillColor(Color(200, 255, 200));
                    arrive.setPosition(detailsPanelOffset + 35, y + 70);

                    if (y + 100 < view.top + view.height)
                        window.draw(arrive);
                }

                float contentHeight = path.size() * detailsLineHeight + 300;

                if (contentHeight > dm.height - 200)
                {
                    float trackH = dm.height - 260;
                    detailsScrollTrack = RectangleShape(Vector2f(10, trackH));
                    detailsScrollTrack.setPosition(detailsPanelOffset + panelWidth - 35, 160);
                    detailsScrollTrack.setFillColor(Color(70, 70, 80, 180));
                    window.draw(detailsScrollTrack);

                    float ratio = (dm.height - 200) / contentHeight;
                    float thumbH = max(50.f, ratio * trackH);
                    float maxTravel = trackH - thumbH;
                    float scrollable = contentHeight - (dm.height - 200);
                    float thumbY = 160 + (detailsScrollOffset / scrollable) * maxTravel;

                    detailsScrollThumb = RectangleShape(Vector2f(10, thumbH));
                    detailsScrollThumb.setFillColor(Color(150, 150, 200));
                    detailsScrollThumb.setPosition(detailsPanelOffset + panelWidth - 35, thumbY);
                    window.draw(detailsScrollThumb);
                }
            }

            // CUSTOMIZATION PANEL
            if ((customizePanelOpen || customizePanelClosing) && !allPaths.empty())
            {
                RectangleShape customizeOverlay(Vector2f(panelWidth, dm.height));
                customizeOverlay.setPosition(customizePanelOffset, 0);
                customizeOverlay.setFillColor(Color(20, 30, 20, 250));
                window.draw(customizeOverlay);

                setupInputBox(customizeBackBtn, customizeBackTxt, font, "< Back",
                              Vector2f(customizePanelOffset + 20, 20),
                              Vector2f(80, 50),
                              Color(100, 60, 60),
                              Color::White, 20);
                window.draw(customizeBackBtn);
                window.draw(customizeBackTxt);

                Text customizeTitle("Customize Path", font, 28);
                customizeTitle.setFillColor(Color::White);
                customizeTitle.setStyle(Text::Bold);
                customizeTitle.setPosition(customizePanelOffset + 20, 80);
                window.draw(customizeTitle);

                // Position customization dropdowns
                companiesSelect.box.setPosition(customizePanelOffset + 30, 140);
                avoidPortsSelect.box.setPosition(customizePanelOffset + 30, 220);
                companiesSelect.dropdown.y = 190;
                avoidPortsSelect.dropdown.y = 270;

                // Preferred Companies Section
                Text companiesLabel("Preferred Shipping Companies:", font, 18);
                companiesLabel.setFillColor(Color::White);
                companiesLabel.setPosition(customizePanelOffset + 30, 110);
                window.draw(companiesLabel);

                companiesSelect.box.setFillColor(companiesSelect.dropdown.open ? Color(70, 70, 90) : Color(90, 90, 150));
                companiesSelect.box.setOutlineThickness(2);
                companiesSelect.box.setOutlineColor(companiesSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));
                window.draw(companiesSelect.box);

                companiesSelect.text = Text(companiesSelect.selected.empty() ? companiesSelect.placeholder : companiesSelect.selected, font, 16);
                companiesSelect.text.setFillColor(companiesSelect.selected.empty() ? Color(150, 150, 150) : Color::White);
                companiesSelect.text.setPosition(customizePanelOffset + 40, 150);
                window.draw(companiesSelect.text);

                companiesSelect.arrow.setPosition(customizePanelOffset + panelWidth - 90, 145);
                window.draw(companiesSelect.arrow);

                // Avoid Ports Section
                Text avoidPortsLabel("Ports to Avoid:", font, 18);
                avoidPortsLabel.setFillColor(Color::White);
                avoidPortsLabel.setPosition(customizePanelOffset + 30, 190);
                window.draw(avoidPortsLabel);

                avoidPortsSelect.box.setFillColor(avoidPortsSelect.dropdown.open ? Color(70, 70, 90) : Color(90, 90, 150));
                avoidPortsSelect.box.setOutlineThickness(2);
                avoidPortsSelect.box.setOutlineColor(avoidPortsSelect.dropdown.open ? Color(100, 180, 255) : Color(80, 80, 100));
                window.draw(avoidPortsSelect.box);

                avoidPortsSelect.text = Text(avoidPortsSelect.selected.empty() ? avoidPortsSelect.placeholder : avoidPortsSelect.selected, font, 16);
                avoidPortsSelect.text.setFillColor(avoidPortsSelect.selected.empty() ? Color(150, 150, 150) : Color::White);
                avoidPortsSelect.text.setPosition(customizePanelOffset + 40, 230);
                window.draw(avoidPortsSelect.text);

                avoidPortsSelect.arrow.setPosition(customizePanelOffset + panelWidth - 90, 225);
                window.draw(avoidPortsSelect.arrow);

                // Max Voyage Time Section
                Text maxVoyageLabel("Max Voyage Time (minutes):", font, 18);
                maxVoyageLabel.setFillColor(Color::White);
                maxVoyageLabel.setPosition(customizePanelOffset + 30, 280);
                window.draw(maxVoyageLabel);

                Text maxVoyageHint("(e.g., 480 for 8 hours, 0 for no limit)", font, 14);
                maxVoyageHint.setFillColor(Color(180, 180, 180));
                maxVoyageHint.setPosition(customizePanelOffset + 30, 305);
                window.draw(maxVoyageHint);

                maxVoyageInputBox.box.setPosition(customizePanelOffset + 30, 330);
                window.draw(maxVoyageInputBox.box);

                maxVoyageInputBox.text.setPosition(customizePanelOffset + 40, 340);
                if (maxVoyageInputBox.content.empty() && !maxVoyageInputBox.active)
                {
                    Text placeholder(maxVoyageInputBox.placeholder, font, 16);
                    placeholder.setFillColor(Color(150, 150, 150));
                    placeholder.setPosition(customizePanelOffset + 40, 340);
                    window.draw(placeholder);
                }
                else
                {
                    window.draw(maxVoyageInputBox.text);
                }

                // Action Buttons
                setupInputBox(applyCustomizationBtn, applyCustomizationTxt, font, "Apply Filters",
                              Vector2f(customizePanelOffset + 30, 400),
                              Vector2f(panelWidth - 100, 50),
                              Color(70, 120, 70), Color::White, 18);
                window.draw(applyCustomizationBtn);
                window.draw(applyCustomizationTxt);

                setupInputBox(resetCustomizationBtn, resetCustomizationTxt, font, "Reset",
                              Vector2f(customizePanelOffset + panelWidth - 110, 400),
                              Vector2f(80, 50),
                              Color(120, 70, 70), Color::White, 18);
                window.draw(resetCustomizationBtn);
                window.draw(resetCustomizationTxt);

                // Current Filters Status
                Text filtersStatus("Current Filters:", font, 16);
                filtersStatus.setFillColor(Color::White);
                filtersStatus.setStyle(Text::Bold);
                filtersStatus.setPosition(customizePanelOffset + 30, 470);
                window.draw(filtersStatus);

                string statusText = "";
                if (!preferredCompanies.empty())
                {
                    statusText += "Companies: ";
                    for (size_t i = 0; i < preferredCompanies.size(); ++i)
                    {
                        if (i > 0)
                            statusText += ", ";
                        statusText += preferredCompanies[i];
                    }
                    statusText += "\n";
                }
                if (!avoidPortsList.empty())
                {
                    statusText += "Avoid Ports: ";
                    for (size_t i = 0; i < avoidPortsList.size(); ++i)
                    {
                        if (i > 0)
                            statusText += ", ";
                        statusText += avoidPortsList[i];
                    }
                    statusText += "\n";
                }
                if (maxVoyageMinutes > 0)
                {
                    statusText += "Max Voyage: " + to_string(maxVoyageMinutes) + " minutes\n";
                }
                if (statusText.empty())
                {
                    statusText = "No filters applied";
                }

                Text statusDisplay(statusText, font, 14);
                statusDisplay.setFillColor(Color(200, 220, 200));
                statusDisplay.setPosition(customizePanelOffset + 30, 495);
                window.draw(statusDisplay);
            }
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

                // Highlight selected items in multi-select dropdowns
                if (dd.multiSelect && dd.selected[idx])
                {
                    RectangleShape h(Vector2f(dd.width - 10, dd.itemHeight - 8));
                    h.setPosition(dd.x + 5, iy);
                    h.setFillColor(Color(70, 180, 70, 180)); // Green for selected
                    window.draw(h);
                }
                else if (idx == dd.hovered)
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

                // Add checkmark for selected items in multi-select
                if (dd.multiSelect && dd.selected[idx])
                {
                    Text check("✓", font, 19);
                    check.setFillColor(Color::White);
                    check.setPosition(dd.x + dd.width - 30, iy + 8);
                    window.draw(check);
                }
            }
        };

        drawDrop(dateSelect.dropdown);
        drawDrop(fromSelect.dropdown);
        drawDrop(toSelect.dropdown);
        drawDrop(modeSelect.dropdown);
        drawDrop(companiesSelect.dropdown);
        drawDrop(avoidPortsSelect.dropdown);

        window.display();
    }

    return 0;
}