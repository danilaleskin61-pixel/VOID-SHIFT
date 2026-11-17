
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({1000, 700}),
        "VOID SHIFT"
    );

    window.setFramerateLimit(60);

    // Загружаем текстуры
    sf::Texture backgroundTexture;
    sf::Texture spacecraft_texture;

    if (!backgroundTexture.loadFromFile(
        "assets/textures/space.jpg"))
    {
        std::cout << "Ошибка загрузки фона!\n";
        return 1;
    }

    if (!spacecraft_texture.loadFromFile(
        "assets/textures/spacecraft.png"))
    {
        std::cout << "Ошибка загрузки корабля!\n";
        return 1;
    }

    // Создаём спрайты
    sf::Sprite background(backgroundTexture);
    sf::Sprite spacecraft(spacecraft_texture);

    // Масштабируем фон
    background.setScale({
        1000.f / backgroundTexture.getSize().x,
        700.f / backgroundTexture.getSize().y
    });

    // Масштабируем корабль
    spacecraft.setScale({
        128.f / spacecraft_texture.getSize().x,
        128.f / spacecraft_texture.getSize().y
    });

    // Размещаем корабль в центре
    spacecraft.setPosition({468.f, 318.f});

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear();

        window.draw(background);
        window.draw(spacecraft);

        window.display();
    }

    return 0;
}

