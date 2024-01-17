#include <iostream>
#include <SFML/Graphics.hpp>
#include <complex>

#include <thread>
#include <vector>
#include <chrono>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <fstream>

#define USE_FUTURE 1

# if USE_FUTURE
#include <future>
#endif

// g++ -o mandelbrot_interactive mandelbrot_interactive.cpp -lsfml-graphics -lsfml-window -lsfml-system -lpthread && ./mandelbrot_interactive
// Using starting coordinates for pan and zoom (see last_coordinates.txt)
// g++ -o mandelbrot_interactive mandelbrot_interactive.cpp -lsfml-graphics -lsfml-window -lsfml-system -lpthread && ./mandelbrot_interactive 1 -0.3 0
// g++ -o mandelbrot_interactive mandelbrot_interactive.cpp -lsfml-graphics -lsfml-window -lsfml-system -lpthread && ./mandelbrot_interactive 26854.6 -1.24993 -0.0125627

const int WIDTH = 1280;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 1000;

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

std::atomic<bool> redraw(false);
std::atomic<bool> redrawRequested(false);

void delayedRedraw(int delayInMilliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delayInMilliseconds));
    if (redrawRequested.load()) {
        redraw.store(true);
        redrawRequested.store(false);
    }
}

void saveCoordinates(float zoom, std::complex<float> move, const std::string& filename) {
    std::ofstream outFile(filename);
    if (outFile) {
        outFile << zoom << " " << move.real() << " " << move.imag() << std::endl;
    }
    outFile.close();
}

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color(0, 0, 0));
    sf::Texture texture;
    sf::Sprite sprite;

    float zoom = 1.0f;
    std::complex<float> move(0, 0);
    std::thread redrawThread;
    redraw.store(true);

    // Use zoom / pan from command-line argument
    if (argc == 4) {
        zoom = std::stof(argv[1]);
        move = std::complex<float>(std::stof(argv[2]), std::stof(argv[3]));
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) 
                window.close();

            // Handle zoom in and out
            if (event.type == sf::Event::MouseWheelMoved) {
                if (event.mouseWheel.delta > 0) zoom *= 1.1f;
                else zoom /= 1.1f;
                redrawRequested.store(true);
            }

            // Handle pan
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                move -= std::complex<float>(0.1f / zoom, 0);
                redrawRequested.store(true);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                move += std::complex<float>(0.1f / zoom, 0);
                redrawRequested.store(true);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                move -= std::complex<float>(0, 0.1f / zoom);
                redrawRequested.store(true);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                move += std::complex<float>(0, 0.1f / zoom);
                redrawRequested.store(true);
            }

            // Graceful exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }

            // Check if a redraw is requested and if the thread is not already running
            if (redrawRequested.load() && !redraw.load()) {
                if (redrawThread.joinable()) {
                    redrawThread.join(); // Ensure the previous thread is finished
                }
                redrawThread = std::thread(delayedRedraw, 1000); // Start a new thread for the delay
            }
        }

        if (redraw.load()) {
            //const int threadCount = 1; // Number of threads to use
            unsigned int threadCount = std::thread::hardware_concurrency();
            std::cout << "Using " << threadCount << " threads" << std::endl;
#if USE_FUTURE
            std::vector<std::future<void>> futures;

            for (unsigned int i = 0; i < threadCount; ++i) {
                int startY = i * HEIGHT / threadCount;
                int endY = (i + 1) * HEIGHT / threadCount;
                futures.emplace_back(std::async(std::launch::async, computeMandelbrotSection, std::ref(image), zoom, move, startY, endY));
            }

            for (auto& f : futures) {
                f.get(); // Wait for all threads to complete
            }
#else
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
#endif

            texture.loadFromImage(image);
            sprite.setTexture(texture);
            redraw.store(false);
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    if (redrawThread.joinable()) {
        redrawThread.join(); // Make sure to join the thread before exiting
    }

    image.saveToFile("mandelbrot_interactive.png");
    saveCoordinates(zoom, move, "last_coordinates.txt");

    return 0;
}
