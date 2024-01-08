#include <SFML/Graphics.hpp>
#include <complex>

#define USE_MUL_THREADS 1

#if USE_MUL_THREADS
#include <thread>
#include <vector>
#include <chrono>
#endif

// g++ -o mandelbrot_interactive mandelbrot_interactive.cpp -lsfml-graphics -lsfml-window -lsfml-system && ./mandelbrot_interactive
// If USE_MUL_THREADS is set:
// g++ -o mandelbrot_interactive mandelbrot_interactive.cpp -lsfml-graphics -lsfml-window -lsfml-system -pthread && ./mandelbrot_interactive

const int WIDTH = 1280;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 500;

sf::Color getColor(int iterations) {
    int r, g, b;

    // Modify this color scheme as needed
    double t = (double)iterations / MAX_ITERATIONS;

    r = (int)(9 * (1-t) * t * t * t * 255);
    g = (int)(15 * (1-t) * (1-t) * t * t * 255);
    b =  (int)(8.5 * (1-t) * (1-t) * (1-t) * t * 255);

    return sf::Color(r, g, b);
}

std::complex<float> convertToComplex(int x, int y, float zoom, std::complex<float> move) {
    float real = (x - WIDTH / 2.0f) / (0.5f * zoom * WIDTH) + move.real();
    float imag = (y - HEIGHT / 2.0f) / (0.5f * zoom * HEIGHT) + move.imag();
    return std::complex<float>(real, imag);
}

int mandelbrot(std::complex<float> c) {
    std::complex<float> z(0, 0);
    int iter = 0;

    while (abs(z) < 2 && iter < MAX_ITERATIONS) {
        z = z * z + c;
        iter++;
    }

    return iter;
}

#if USE_MUL_THREADS
void computeMandelbrotSection(sf::Image& image, float zoom, std::complex<float> move, int startY, int endY) {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = startY; y < endY; y++) {
            std::complex<float> c = convertToComplex(x, y, zoom, move);
            int value = mandelbrot(c);
            sf::Color color = getColor(value);
            image.setPixel(x, y, color);
        }
    }
}
#endif

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color(0, 0, 0));
    sf::Texture texture;
    sf::Sprite sprite;

    float zoom = 1.0f;
    std::complex<float> move(0, 0);
    bool redraw = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) 
                window.close();

            // Handle zoom in and out
            if (event.type == sf::Event::MouseWheelMoved) {
                if (event.mouseWheel.delta > 0) zoom *= 1.1f;
                else zoom /= 1.1f;
                redraw = true;
            }

            // Handle pan
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                move -= std::complex<float>(0.1f / zoom, 0);
                redraw = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                move += std::complex<float>(0.1f / zoom, 0);
                redraw = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                move -= std::complex<float>(0, 0.1f / zoom);
                redraw = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                move += std::complex<float>(0, 0.1f / zoom);
                redraw = true;
            }

            // Graceful exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        if (redraw) {
#if USE_MUL_THREADS
        //const int threadCount = 4; // Number of threads to use
        unsigned int threadCount = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;

        // Divide the work among threads
        for (int i = 0; i < threadCount; ++i) {
            int startY = i * HEIGHT / threadCount;
            int endY = (i + 1) * HEIGHT / threadCount;
            threads.emplace_back(computeMandelbrotSection, std::ref(image), zoom, move, startY, endY);
        }

        // Join threads
        for (auto& t : threads) {
            t.join();
        }
#else
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                    std::complex<float> c = convertToComplex(x, y, zoom, move);
                    int value = mandelbrot(c);
                    sf::Color color = getColor(value);
                    image.setPixel(x, y, color);
                }
            }
#endif
            texture.loadFromImage(image);
            sprite.setTexture(texture);
            redraw = false;
        }

        //sf::Texture texture;
        //texture.loadFromImage(image);
        //sf::Sprite sprite(texture);
        window.clear();
        window.draw(sprite);
        window.display();
    }

    image.saveToFile("mandelbrot_interactive.png");

    return 0;
}
