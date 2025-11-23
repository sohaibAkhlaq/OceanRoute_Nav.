#pragma once

#include <SFML/Graphics.hpp>
#include <string>

using namespace sf;
using namespace std;

class Helper
{
    RenderWindow *window;
    Font *font;
    Clock internalClock;
    float lastRingTime;

public:
    Helper();
    Helper(RenderWindow *w);
    void setWindow(RenderWindow *w);
    void setFont(Font *f);

    RenderWindow &getWindow();
    Font &getFont();
    void update();
    void draw();
    void drawCenteredText(string text, float posX, float posY, int size = 27, Color color = Color::White);
    int getClickedItemIndex(Event &event, Sprite items[], int itemCount, Clock &clickClock, float debounceTime = 0.3f);
    void centerTextInButton(Text &text, const FloatRect &buttonRect);
    void drawDotAt(Vector2f position, Color color = Color::Blue, float radius = 4.f);
    void drawRectBounds(FloatRect rect, Color color = Color::Blue, float thickness = 2.f);
    void drawTextAt(const string textStr, Vector2f position = Vector2f(300, 300), unsigned int fontSize = 20, Color color = Color::White);
};

// Implementation
inline Helper::Helper()
    : window(nullptr), font(nullptr), lastRingTime(0.0f) {}

inline Helper::Helper(RenderWindow *w)
    : window(w), font(nullptr), lastRingTime(0.0f) {}

inline void Helper::setWindow(RenderWindow *w)
{
    window = w;
}

inline void Helper::setFont(Font *f) { font = f; }

inline RenderWindow &Helper::getWindow() { return *window; }
inline Font &Helper::getFont() { return *font; }

inline void Helper::update()
{
    float dt = internalClock.restart().asSeconds();
}

inline void Helper::draw()
{
}

inline void Helper::drawCenteredText(string text, float posX, float posY, int size, Color color)
{
    Text t;
    t.setFont(*font);
    t.setString(text);
    t.setStyle(Text::Bold);
    t.setCharacterSize(size);
    t.setFillColor(color);

    FloatRect bounds = t.getLocalBounds();
    t.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2);
    t.setPosition(posX, posY);

    window->draw(t);
}

inline int Helper::getClickedItemIndex(Event &event, Sprite items[], int itemCount, Clock &clickClock, float debounceTime)
{
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
    {
        if (clickClock.getElapsedTime().asSeconds() < debounceTime)
            return -1;

        Vector2f mousePos = window->mapPixelToCoords(Mouse::getPosition(*window));

        for (int i = 0; i < itemCount; ++i)
        {
            FloatRect bounds = items[i].getGlobalBounds();

            IntRect texRect = items[i].getTextureRect();
            float textureRatioX = static_cast<float>(texRect.width) / items[i].getTexture()->getSize().x;
            float textureRatioY = static_cast<float>(texRect.height) / items[i].getTexture()->getSize().y;

            bounds.width *= textureRatioX;
            bounds.height *= textureRatioY;

            if (bounds.contains(mousePos))
            {
                clickClock.restart();
                return i;
            }
        }
    }
    return -1;
}

inline void Helper::centerTextInButton(Text &text, const FloatRect &buttonRect)
{
    FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);

    float centerX = buttonRect.left + buttonRect.width / 2.f;
    float centerY = buttonRect.top + buttonRect.height / 2.f;

    text.setPosition(centerX, centerY);
}

inline void Helper::drawDotAt(Vector2f position, Color color, float radius)
{
    CircleShape dot(radius);
    dot.setFillColor(color);
    dot.setOrigin(radius, radius);
    dot.setPosition(position);
    window->draw(dot);
}

inline void Helper::drawRectBounds(FloatRect rect, Color color, float thickness)
{
    RectangleShape outline;
    outline.setPosition(rect.left, rect.top);
    outline.setSize(Vector2f(rect.width, rect.height));
    outline.setFillColor(Color::Transparent);
    outline.setOutlineColor(color);
    outline.setOutlineThickness(thickness);
    window->draw(outline);
}

inline void Helper::drawTextAt(const string textStr, Vector2f position, unsigned int fontSize, Color color)
{
    Text text;
    text.setFont(*font);
    text.setString(textStr);
    text.setCharacterSize(fontSize);
    text.setFillColor(color);
    text.setPosition(position);
    window->draw(text);
}

// Global helper variable declaration
extern Helper helper;

