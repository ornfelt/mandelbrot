import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class MandelbrotInteractive extends JFrame {
    private static final int WIDTH = 1280;
    private static final int HEIGHT = 800;
    private static final int MAX_ITERATIONS = 500;

    private float zoom = 1.0f;
    private Complex move = new Complex(0, 0);
    private BufferedImage image;
    private AtomicBoolean redraw = new AtomicBoolean(true);
    private AtomicBoolean redrawRequested = new AtomicBoolean(false);

    public MandelbrotInteractive() {
        super("Mandelbrot Set");
        setPreferredSize(new Dimension(WIDTH, HEIGHT));
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        pack();
        setLocationRelativeTo(null);
        setVisible(true);
        image = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_RGB);

        addMouseListener(new MouseAdapter() {
            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                if (e.getWheelRotation() < 0) zoom *= 1.1f;
                else zoom /= 1.1f;
                redrawRequested.set(true);
            }
        });

        addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                switch (e.getKeyCode()) {
                    case KeyEvent.VK_LEFT:
                        move = move.subtract(new Complex(0.1f / zoom, 0));
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_RIGHT:
                        move = move.add(new Complex(0.1f / zoom, 0));
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_UP:
                        move = move.subtract(new Complex(0, 0.1f / zoom));
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_DOWN:
                        move = move.add(new Complex(0, 0.1f / zoom));
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_W:
                        zoom *= 1.1f;
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_S:
                        zoom /= 1.1f;
                        redrawRequested.set(true);
                        break;
                    case KeyEvent.VK_ESCAPE:
                        System.exit(0);
                        break;
                }
            }
        });

        new Thread(() -> {
            while (true) {
                if (redrawRequested.getAndSet(false)) {
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                    redraw.set(true);
                }

                if (redraw.get()) {
                    drawMandelbrot();
                    repaint();
                    redraw.set(false);
                }
            }
        }).start();
    }

    private void drawMandelbrot() {
        int threadCount = Runtime.getRuntime().availableProcessors();
        List<Thread> threads = new ArrayList<>();
        
        for (int i = 0; i < threadCount; i++) {
            int startY = i * HEIGHT / threadCount;
            int endY = (i + 1) * HEIGHT / threadCount;
            Thread thread = new Thread(() -> {
                for (int x = 0; x < WIDTH; x++) {
                    for (int y = startY; y < endY; y++) {
                        Complex c = convertToComplex(x, y, zoom, move);
                        int value = mandelbrot(c);
                        Color color = getColor(value);
                        image.setRGB(x, y, color.getRGB());
                    }
                }
            });
            threads.add(thread);
            thread.start();
        }

        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    private Color getColor(int iterations) {
        double t = (double) iterations / MAX_ITERATIONS;
        int r = (int) (9 * (1 - t) * t * t * t * 255);
        int g = (int) (15 * (1 - t) * (1 - t) * t * t * 255);
        int b = (int) (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        return new Color(r, g, b);
    }

    private Complex convertToComplex(int x, int y, float zoom, Complex move) {
        float real = (x - WIDTH / 2.0f) / (0.5f * zoom * WIDTH) + move.real;
        float imag = (y - HEIGHT / 2.0f) / (0.5f * zoom * HEIGHT) + move.imag;
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

    @Override
    public void paint(Graphics g) {
        g.drawImage(image, 0, 0, this);
    }

    public static void main(String[] args) {
        new MandelbrotInteractive();
    }

    private static class Complex {
        final float real;
        final float imag;

        public Complex(float real, float imag) {
            this.real = real;
            this.imag = imag;
        }

        public Complex add(Complex other) {
            return new Complex(this.real + other.real, this.imag + other.imag);
        }

        public Complex subtract(Complex other) {
            return new Complex(this.real - other.real, this.imag - other.imag);
        }

        public Complex square() {
            return new Complex(this.real * this.real - this.imag * this.imag, 2 * this.real * this.imag);
        }

        public float abs() {
            return (float) Math.sqrt(real * real + imag * imag);
        }
    }
}
