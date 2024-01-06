import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.List;

public class MandelbrotInteractive extends JFrame {
    private static final int WIDTH = 1280;
    private static final int HEIGHT = 800;
    private static final int MAX_ITERATIONS = 50;
    private float zoom = 1.0f;
    private Complex move = new Complex(0, 0);
    private BufferedImage image;
    private boolean redraw = true;
    private boolean redrawRequested = true; // Set to true to draw initially

    public MandelbrotInteractive() {
        super("Mandelbrot Set");
        setPreferredSize(new Dimension(WIDTH, HEIGHT));
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        pack();
        setVisible(true);
        image = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_RGB);

        // Add mouse wheel listener for zoom
        addMouseWheelListener(new MouseAdapter() {
            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                if (e.getPreciseWheelRotation() < 0) zoom *= 1.1f;
                else zoom /= 1.1f;
                redrawRequested = true;
            }
        });

        // Add key listener for movement
        addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                switch (e.getKeyCode()) {
                    case KeyEvent.VK_LEFT:
                        move = move.subtract(new Complex(0.1f / zoom, 0));
                        redrawRequested = true;
                        break;
                    case KeyEvent.VK_RIGHT:
                        move = move.add(new Complex(0.1f / zoom, 0));
                        redrawRequested = true;
                        break;
                    case KeyEvent.VK_UP:
                        move = move.subtract(new Complex(0, 0.1f / zoom));
                        redrawRequested = true;
                        break;
                    case KeyEvent.VK_DOWN:
                        move = move.add(new Complex(0, 0.1f / zoom));
                        redrawRequested = true;
                        break;
                    case KeyEvent.VK_ESCAPE:
                        System.exit(0);
                        break;
                }
            }
        });

        // Ensure the component is focusable for key listener
        setFocusable(true);
        requestFocusInWindow();

        // Initial draw
        drawMandelbrot();
    }

    private void drawMandelbrot() {
        // Ensuring the drawing is done on the Event Dispatch Thread
        SwingUtilities.invokeLater(() -> {
            int threadCount = Runtime.getRuntime().availableProcessors();
            List<Thread> threads = new ArrayList<>();

            for (int i = 0; i < threadCount; i++) {
                int startY = i * HEIGHT / threadCount;
                int endY = (i + 1) * HEIGHT / threadCount;
                Thread thread = new Thread(() -> computeMandelbrotSection(startY, endY));
                threads.add(thread);
                thread.start();
            }

            for (Thread thread : threads) {
                try {
                    thread.join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            repaint(); // Trigger repaint after drawing
        });
    }

    private void computeMandelbrotSection(int startY, int endY) {
        for (int x = 0; x < WIDTH; x++) {
            for (int y = startY; y < endY; y++) {
                Complex c = convertToComplex(x, y);
                int value = mandelbrot(c);
                Color color = getColor(value);
                image.setRGB(x, y, color.getRGB());
            }
        }
    }

    private Complex convertToComplex(int x, int y) {
        double real = (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + move.real;
        double imag = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + move.imag;
        return new Complex(real, imag);
    }

    private int mandelbrot(Complex c) {
        Complex z = new Complex(0, 0);
        int iter = 0;

        while (z.abs() < 2 && iter < MAX_ITERATIONS) {
            z = z.square().add(c);
            iter++;
        }

        return iter;
    }

    private Color getColor(int iterations) {
        double t = (double) iterations / MAX_ITERATIONS;
        int r = (int) (9 * (1 - t) * t * t * t * 255);
        int g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);
        int b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        return new Color(r, g, b);
    }

    @Override
    public void paint(Graphics g) {
        super.paint(g);
        if (redrawRequested) {
            drawMandelbrot();
            redrawRequested = false;
        }
        g.drawImage(image, 0, 0, this);
    }

    private static class Complex {
        final double real;
        final double imag;

        Complex(double real, double imag) {
            this.real = real;
            this.imag = imag;
        }

        Complex add(Complex other) {
            return new Complex(this.real + other.real, this.imag + other.imag);
        }

        Complex subtract(Complex other) {
            return new Complex(this.real - other.real, this.imag - other.imag);
        }

        Complex square() {
            return new Complex(this.real * this.real - this.imag * this.imag, 2 * this.real * this.imag);
        }

        double abs() {
            return Math.sqrt(this.real * this.real + this.imag * this.imag);
        }
    }

    public static void main(String[] args) {
        new MandelbrotInteractive().setVisible(true);
    }
}
