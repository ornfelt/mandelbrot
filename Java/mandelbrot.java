
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.Color;
import java.io.File;
import java.io.IOException;

public class MandelbrotGenerator {

    private static final int WIDTH = 800;
    private static final int HEIGHT = 600;
    private static final int MAX_ITERATIONS = 1000;

    public static void main(String[] args) throws IOException {
        BufferedImage image = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_RGB);

        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
                double imag = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
                int colorValue = getColor(mandelbrot(real, imag));
                image.setRGB(x, y, colorValue);
            }
        }

        ImageIO.write(image, "png", new File("mandelbrot_color.png"));
        ImageIO.write(image, "bmp", new File("mandelbrot_color.bmp"));

        System.out.println("Mandelbrot set images saved as mandelbrot_color.png and mandelbrot_color.bmp");
    }

    private static int mandelbrot(double real, double imag) {
        double zReal = 0.0;
        double zImag = 0.0;
        int iterations = 0;

        while (zReal * zReal + zImag * zImag < 4.0 && iterations < MAX_ITERATIONS) {
            double newZReal = zReal * zReal - zImag * zImag + real;
            double newZImag = 2.0 * zReal * zImag + imag;
            zReal = newZReal;
            zImag = newZImag;
            iterations++;
        }

        return iterations;
    }

    private static int getColor(int iterations) {
        float t = (float) iterations / MAX_ITERATIONS;
        int r = (int) (9 * (1 - t) * t * t * t * 255);
        int g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);
        int b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        return new Color(r, g, b).getRGB();
    }
}
