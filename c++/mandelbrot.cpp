#include <SFML/Graphics.hpp>
#include <complex>

// g++ -o mandelbrot mandelbrot.cpp -lsfml-graphics -lsfml-window -lsfml-system && ./mandelbrot

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int MAX_ITERATIONS = 1000;

sf::Color getColor(int iterations) {
    int r, g, b;

    // Example coloring
    double t = (double)iterations / MAX_ITERATIONS;

    r = (int)(9 * (1-t) * t * t * t * 255);
    g = (int)(15 * (1-t) * (1-t) * t * t * 255);
    b =  (int)(8.5 * (1-t) * (1-t) * (1-t) * t * 255);

    return sf::Color(r, g, b);
}

sf::Color getColor2(int iterations) {
    double t = (double)iterations / MAX_ITERATIONS;
    int r = static_cast<int>((0.5 * sin(t * 3.14159) + 0.5) * 255);
    int g = static_cast<int>((0.5 * cos(t * 3.14159) + 0.5) * 255);
    int b = static_cast<int>(t * 255);
    return sf::Color(r, g, b);
}

int mandelbrot(double real, double imag) {
    std::complex<double> point(real, imag);
    std::complex<double> z(0, 0);
    int iterations = 0;

    while (abs(z) < 2 && iterations < MAX_ITERATIONS) {
        z = z * z + point;
        iterations++;
    }

    return iterations;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color(0, 0, 0));

    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
            double imag = (y - HEIGHT / 2.0) * 4.0 / WIDTH;
            int value = mandelbrot(real, imag);
            sf::Color color = getColor(value);
            //sf::Color color = getColor2(value);
            image.setPixel(x, y, color);
        }
    }

    image.saveToFile("mandelbrot.png");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Graceful exit
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        sf::Texture texture;
        texture.loadFromImage(image);
        sf::Sprite sprite(texture);
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
