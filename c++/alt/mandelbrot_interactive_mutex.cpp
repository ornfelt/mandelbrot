#include <SFML/Graphics.hpp>
#include <complex>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

// g++ -o mandelbrot_interactive_mutex mandelbrot_interactive_mutex.cpp -lsfml-graphics -lsfml-window -lsfml-system -pthread && ./mandelbrot_interactive_mutex

/**
The drawing logic for this code works in the following way:

Detecting an Update Request:
When an event occurs (such as zooming or panning), the handleEvent function in
the main loop sets updateRequested to true and notifies the condition variable
cv. The redrawThreadFunction wakes up upon this notification.

Setting the Last Update Time:
As soon as the redrawThreadFunction wakes up, it records the current time as
lastUpdate. This timestamp represents the moment of the last update request.

Waiting for 500 ms or a New Event:
The thread then enters a loop where it waits until either:
500ms has passed since lastUpdate (using cv.wait_until).
A new update request arrives (if updateRequested becomes true again).
If a new event occurs before 500 ms has passed, the handleEvent function sets
updateRequested to true again, and the condition variable cv is notified. This
causes the thread to reset the lastUpdate time and start the 1-second wait
again.

Redrawing After 500 ms of No New Events:
If no new event occurs within the 1-second window, the thread proceeds to
execute the redraw logic, updating the Mandelbrot set with the latest zoom and
pan settings. After redrawing, the thread goes back to waiting for the next
event.

By resetting the lastUpdate time with every new event and using cv.wait_until,
the thread effectively delays the redraw until there are no new events for 500
ms. This approach avoids the issue of the program being unresponsive or sluggish
due to frequent and rapid redrawing when multiple events occur in quick
succession.
*/

const int WIDTH = 1280;
const int HEIGHT = 800;
const int MAX_ITERATIONS = 500;

bool updateRequested = true;
bool redrawPending = false;
std::mutex mtx;
std::condition_variable cv;

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

bool handleEvent(const sf::Event& event, float& zoom, std::complex<float>& move) {
    bool updateRequested = false;

    if (event.type == sf::Event::MouseWheelMoved) {
        if (event.mouseWheel.delta > 0) zoom *= 1.1f;
        else zoom /= 1.1f;
        updateRequested = true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        move -= std::complex<float>(0.1f / zoom, 0);
        updateRequested = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        move += std::complex<float>(0.1f / zoom, 0);
        updateRequested = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        move -= std::complex<float>(0, 0.1f / zoom);
        updateRequested = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        move += std::complex<float>(0, 0.1f / zoom);
        updateRequested = true;
    }

    return updateRequested;
}

void redrawThreadFunction(sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::RenderWindow& window, float& zoom, std::complex<float>& move) {
    std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return updateRequested; });

        // Update the time of the last update request
        lastUpdate = std::chrono::steady_clock::now();
        updateRequested = false;

        while (std::chrono::steady_clock::now() < lastUpdate + std::chrono::seconds(1)) {
            //cv.wait_until(lock, lastUpdate + std::chrono::seconds(1));
            cv.wait_until(lock, lastUpdate + std::chrono::milliseconds(500));
        }

        if (window.isOpen() && !updateRequested) {
            // Redraw logic
            //const int threadCount = 4; // Number of threads to use
            unsigned int threadCount = std::thread::hardware_concurrency();
            std::vector<std::thread> threads;
            for (int i = 0; i < threadCount; ++i) {
                int startY = i * HEIGHT / threadCount;
                int endY = (i + 1) * HEIGHT / threadCount;
                threads.emplace_back(computeMandelbrotSection, std::ref(image), zoom, move, startY, endY);
            }
            for (auto& t : threads) {
                t.join();
            }

            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color(0, 0, 0));
    sf::Texture texture;
    sf::Sprite sprite;

    float zoom = 1.0f;
    std::complex<float> move(0, 0);

    std::thread redrawThread(redrawThreadFunction, std::ref(image), std::ref(texture), std::ref(sprite), std::ref(window), std::ref(zoom), std::ref(move));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Graceful exit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }

            if (handleEvent(event, zoom, move)) {
                std::lock_guard<std::mutex> lock(mtx);
                redrawPending = true;
                updateRequested = true;
                cv.notify_one();
            }
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    redrawThread.join();
    image.saveToFile("mandelbrot_interactive_mutex.png");

    return 0;
}
