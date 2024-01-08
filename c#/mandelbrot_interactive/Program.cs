using System;
using System.Drawing;
using System.Numerics;
using System.Windows.Forms;
using System.Threading;

public class MandelbrotForm : Form
{
    private const int WIDTH = 1280;
    private const int HEIGHT = 800;
    private const int MAX_ITERATIONS = 500;
    private Bitmap image;
    private float zoom = 1.0f;
    private Complex move = new Complex(0, 0);
    private bool redraw = true;

    public MandelbrotForm()
    {
        this.ClientSize = new Size(WIDTH, HEIGHT);
        this.Text = "Mandelbrot Set";
        image = new Bitmap(WIDTH, HEIGHT);

        this.MouseWheel += new MouseEventHandler(OnMouseWheel);
        this.KeyDown += new KeyEventHandler(OnKeyDown);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        if (redraw)
        {
            DrawMandelbrot();
            redraw = false;
        }

        e.Graphics.DrawImage(image, 0, 0);
        base.OnPaint(e);
    }

    private void OnMouseWheel(object sender, MouseEventArgs e)
    {
        if (e.Delta > 0) zoom *= 1.1f;
        else zoom /= 1.1f;
        redraw = true;
        this.Invalidate(); // Causes the form to be redrawn
    }

    private void OnKeyDown(object sender, KeyEventArgs e)
    {
        switch (e.KeyCode)
        {
            case Keys.Left:
                move = new Complex(move.Real - 0.1f / zoom, move.Imaginary);
                break;
            case Keys.Right:
                move = new Complex(move.Real + 0.1f / zoom, move.Imaginary);
                break;
            case Keys.Up:
                move = new Complex(move.Real, move.Imaginary - 0.1f / zoom);
                break;
            case Keys.Down:
                move = new Complex(move.Real, move.Imaginary + 0.1f / zoom);
                break;
            case Keys.Escape:
                this.Close();
                break;
        }

        redraw = true;
        this.Invalidate();
    }

    private void DrawMandelbrot()
    {
        // Parallelize the drawing for improved performance
        Parallel.For(0, HEIGHT, y =>
        {
            for (int x = 0; x < WIDTH; x++)
            {
                Complex c = ConvertToComplex(x, y);
                int value = Mandelbrot(c);
                Color color = GetColor(value);
                lock (image)
                {
                    image.SetPixel(x, y, color);
                }
            }
        });
    }

    private Color GetColor(int iterations)
    {
        double t = (double)iterations / MAX_ITERATIONS;
        int r = (int)(9 * (1-t) * t * t * t * 255);
        int g = (int)(15 * (1-t) * (1-t) * t * t * 255);
        int b = (int)(8.5 * (1-t) * (1-t) * (1-t) * t * 255);
        return Color.FromArgb(r, g, b);
    }

    private Complex ConvertToComplex(int x, int y)
    {
        double real = (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + move.Real;
        double imag = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + move.Imaginary;
        return new Complex(real, imag);
    }

    private int Mandelbrot(Complex c)
    {
        Complex z = new Complex(0, 0);
        int iter = 0;

        while (Complex.Abs(z) < 2 && iter < MAX_ITERATIONS)
        {
            z = z * z + c;
            iter++;
        }

        return iter;
    }

    [STAThread]
    static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new MandelbrotForm());
    }
}
