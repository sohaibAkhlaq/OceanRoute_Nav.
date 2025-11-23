#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class ButtonBox
{
private:
    sf::RectangleShape box;
    sf::Text text;
    sf::Font font;

    // Optional sprite
    sf::Sprite sprite;
    bool useSprite = false; // Flag to know if we're using sprite

public:
    // ------------------------
    // Constructor with color box (default)
    // ------------------------
    ButtonBox(const std::string& str, const sf::Font& f, float width = 200.f, float height = 40.f)
    {
        font = f;

        box.setSize(sf::Vector2f(width, height));
        box.setFillColor(sf::Color(70, 70, 70));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);

        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);

        centerText();
    }

    // ------------------------
    // Constructor with Texture
    // ------------------------
    ButtonBox(const std::string& str, const sf::Font& f, const sf::Texture& texture)
    {
        font = f;
        useSprite = true;
        sprite.setTexture(texture);

        // Resize text according to texture size
        sf::Vector2f size(texture.getSize());
        box.setSize(size); // optional invisible rect for bounds if needed
        box.setFillColor(sf::Color::Transparent);

        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);

        centerText();
    }

    // ------------------------
    // Constructor with Sprite
    // ------------------------
    ButtonBox(const std::string& str, const sf::Font& f, const sf::Sprite& s)
    {
        font = f;
        useSprite = true;
        sprite = s;

        sf::Vector2f size(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
        box.setSize(size); // optional invisible rect for bounds
        box.setFillColor(sf::Color::Transparent);

        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);

        centerText();
    }

    // ------------------------
    // Center the text inside the box or sprite
    // ------------------------
    void centerText()
    {
        sf::FloatRect txtBounds = text.getLocalBounds();
        text.setOrigin(txtBounds.left + txtBounds.width / 2.f, txtBounds.top + txtBounds.height / 2.f);

        sf::Vector2f pos;
        if (useSprite)
            pos = sprite.getPosition();
        else
            pos = box.getPosition();

        sf::Vector2f size;
        if (useSprite)
            size = sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
        else
            size = box.getSize();

        text.setPosition(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
    }

    // ------------------------
    // Set position
    // ------------------------
    void setPosition(float x, float y)
    {
        if (useSprite)
            sprite.setPosition(x, y);
        else
            box.setPosition(x, y);
        centerText();
    }

    void setPositionCenter(float cx, float cy)
    {
        sf::Vector2f size = useSprite ? sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height) : box.getSize();
        setPosition(cx - size.x / 2.f, cy - size.y / 2.f);
    }

    // ------------------------
    // Set fill color (only works for rectangle)
    // ------------------------
    void setFillColor(const sf::Color& c)
    {
        if (!useSprite) box.setFillColor(c);
    }

    // ------------------------
    // Set text color
    // ------------------------
    void setTextColor(const sf::Color& c) { text.setFillColor(c); }

    // ------------------------
    // Set scale
    // ------------------------
    void setScale(float sx, float sy)
    {
        if (useSprite) sprite.setScale(sx, sy);
        else box.setScale(sx, sy);
        text.setScale(sx, sy);
        centerText();
    }

    // ------------------------
    // Check if mouse is over
    // ------------------------
    bool contains(const sf::Vector2f& point) const
    {
        if (useSprite) return sprite.getGlobalBounds().contains(point);
        return box.getGlobalBounds().contains(point);
    }

    // ------------------------
    // Draw
    // ------------------------
    void draw(sf::RenderWindow& window)
    {
        if (useSprite) window.draw(sprite);
        else window.draw(box);
        window.draw(text);
    }

    // ------------------------
    // Change text
    // ------------------------
    void setText(const std::string& str)
    {
        text.setString(str);
        centerText();
    }

    // ------------------------
    // Get size
    // ------------------------
    sf::Vector2f getSize() const
    {
        if (useSprite) return sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
        return box.getSize();
    }
};
