using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

public class MandelbrotForm : Form
{
    private const int Width = 1280;
    private const int Height = 800;
    private const int MaxIterations = 50;
    private const bool UseMultipleThreads = true;
    private double zoom = 1.0;
    private Complex move = new Complex(0, 0);
    private bool redraw = true;
    private Bitmap surface;

    public MandelbrotForm()
    {
        this.ClientSize = new Size(Width, Height);
        this.Text = "Mandelbrot Set";
        surface = new Bitmap(Width, Height);
        this.Paint += new PaintEventHandler(OnPaint);
        this.MouseDown += new MouseEventHandler(OnMouseDown);
        this.KeyDown += new KeyEventHandler(OnKeyDown);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        if (redraw)
        {
            ComputeMandelbrot();
            redraw = false;
        }
        e.Graphics.DrawImage(surface, 0, 0);
    }

    private void OnMouseDown(object sender, MouseEventArgs e)
    {
        if (e.Button == MouseButtons.Left) zoom *= 1.1;
        else if (e.Button == MouseButtons.Right) zoom /= 1.1;
        redraw = true;
        this.Invalidate();
    }

    private void OnKeyDown(object sender, KeyEventArgs e)
    {
        switch (e.KeyCode)
        {
            case Keys.Left:
                move -= new Complex(0.1 / zoom, 0);
                redraw = true;
                break;
            case Keys.Right:
                move += new Complex(0.1 / zoom, 0);
                redraw = true;
                break;
            case Keys.Up:
                move -= new Complex(0, 0.1 / zoom);
                redraw = true;
                break;
            case Keys.Down:
                move += new Complex(0, 0.1 / zoom);
                redraw = true;
                break;
            case Keys.Escape:
                this.Close();
                break;
        }
        this.Invalidate();
    }

    private void ComputeMandelbrot()
    {
        int threadCount = UseMultipleThreads ? Environment.ProcessorCount : 1;
        Thread[] threads = new Thread[threadCount];

        for (int i = 0; i < threadCount; i++)
        {
            int startY = i * Height / threadCount;
            int endY = (i + 1) * Height / threadCount;
            threads[i] = new Thread(() => ComputeMandelbrotSection(startY, endY));
            threads[i].Start();
        }

        foreach (Thread thread in threads)
        {
            thread.Join();
        }
    }

    private void ComputeMandelbrotSection(int startY, int endY)
    {
        for (int x = 0; x < Width; x++)
        {
            for (int y = startY; y < endY; y++)
            {
                Complex c = ConvertToComplex(x, y, zoom, move);
                int value = Mandelbrot(c);
                Color color = GetColor(value);
                surface.SetPixel(x, y, color);
            }
        }
    }

    private Color GetColor(int iterations)
    {
        double t = (double)iterations / MaxIterations;
        int r = (int)(9 * (1 - t) * t * t * t * 255);
        int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
        int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        return Color.FromArgb(r, g, b);
    }

    private Complex ConvertToComplex(int x, int y, double zoom, Complex move)
    {
        double real = (x - Width / 2.0) / (0.5 * zoom * Width) + move.Real;
        double imag = (y - Height / 2.0) / (0.5 * zoom * Height) + move.Imaginary;
        return new Complex(real, imag);
    }

    private int Mandelbrot(Complex c)
    {
        Complex z = Complex.Zero;
        int iter = 0;

        while (Complex.Abs(z) < 2 && iter < MaxIterations)
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

public struct Complex
{
    public double Real { get; set; }
    public double Imaginary { get; set; }

    public Complex(double real, double imaginary)
    {
        this.Real = real;
        this.Imaginary = imaginary;
    }

    public static Complex operator +(Complex c1, Complex c2)
    {
        return new Complex(c1.Real + c2.Real, c1.Imaginary + c2.Imaginary);
    }

    public static Complex operator -(Complex c1, Complex c2)
    {
        return new Complex(c1.Real - c2.Real, c1.Imaginary - c2.Imaginary);
    }

    public static Complex operator *(Complex c1, Complex c2)
    {
        double real = c1.Real * c2.Real - c1.Imaginary * c2.Imaginary;
        double imaginary = c1.Real * c2.Imaginary + c1.Imaginary * c2.Real;
        return new Complex(real, imaginary);
    }

    public static double Abs(Complex c)
    {
        return Math.Sqrt(c.Real * c.Real + c.Imaginary * c.Imaginary);
    }

    public static Complex Zero => new Complex(0, 0);
}
