#include <iostream>
#include <fstream>
#include <complex>

// g++ -o mandelbrot_bmp mandelbrot_bmp.cpp && ./mandelbrot_bmp

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int MAX_ITERATIONS = 5000;

struct RGB {
    unsigned char r, g, b;
};

RGB getColor(int iterations) {
    RGB color;
    double t = (double)iterations / MAX_ITERATIONS;

    // Modify this color scheme as needed
    color.r = static_cast<unsigned char>(9 * (1 - t) * t * t * t * 255);
    color.g = static_cast<unsigned char>(15 * (1 - t) * (1 - t) * t * t * 255);
    color.b = static_cast<unsigned char>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return color;
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

void saveBitmap(const std::string& filename, const RGB* colors) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file for writing." << std::endl;
        return;
    }

    unsigned char fileHeader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
    unsigned char infoHeader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};

    int fileSize = 54 + 3 * WIDTH * HEIGHT;
    fileHeader[2] = (unsigned char)(fileSize);
    fileHeader[3] = (unsigned char)(fileSize >> 8);
    fileHeader[4] = (unsigned char)(fileSize >> 16);
    fileHeader[5] = (unsigned char)(fileSize >> 24);

    infoHeader[4] = (unsigned char)(WIDTH);
    infoHeader[5] = (unsigned char)(WIDTH >> 8);
    infoHeader[6] = (unsigned char)(WIDTH >> 16);
    infoHeader[7] = (unsigned char)(WIDTH >> 24);

    infoHeader[8] = (unsigned char)(HEIGHT);
    infoHeader[9] = (unsigned char)(HEIGHT >> 8);
    infoHeader[10] = (unsigned char)(HEIGHT >> 16);
    infoHeader[11] = (unsigned char)(HEIGHT >> 24);

    file.write(reinterpret_cast<char*>(fileHeader), 14);
    file.write(reinterpret_cast<char*>(infoHeader), 40);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            RGB color = colors[y * WIDTH + x];
            unsigned char pixel[3] = {color.b, color.g, color.r};  // BMP uses BGR format
            file.write(reinterpret_cast<char*>(pixel), 3);
        }
    }

    file.close();
}

int main() {
    RGB colors[WIDTH * HEIGHT];

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
            double imag = (y - HEIGHT / 2.0) * 4.0 / WIDTH;
            int value = mandelbrot(real, imag);
            colors[y * WIDTH + x] = getColor(value);
        }
    }

    saveBitmap("mandelbrot.bmp", colors);
    std::cout << "Mandelbrot set image saved as mandelbrot.bmp" << std::endl;

    return 0;
}
