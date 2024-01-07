using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using System;
using System.Threading.Tasks;

namespace mandelbrot_interactive_avalonia;

public partial class MainWindow : Window
{
    private const int Width = 1280;
    private const int Height = 800;
    private const int MaxIterations = 500;
    private WriteableBitmap bitmap;
    private double zoom = 1.0;
    private Complex move = new Complex(0, 0);

    public MainWindow()
    {
        InitializeComponent();
        InitBitmap();
        DrawMandelbrot();
    }

    private void InitBitmap()
    {
        bitmap = new WriteableBitmap(new PixelSize(Width, Height), new Vector(96, 96), Avalonia.Platform.PixelFormat.Rgba8888, Avalonia.Platform.AlphaFormat.Unpremul);
        imageControl.Source = bitmap;
    }

    private void DrawMandelbrot()
    {
        var buffer = new uint[Width * Height];
        Parallel.For(0, Height, y =>
        {
            for (int x = 0; x < Width; x++)
            {
                var c = ConvertToComplex(x, y);
                var m = Mandelbrot(c);
                buffer[y * Width + x] = GetColor(m);
            }
        });

        using (var lockedBitmap = bitmap.Lock())
        {
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    unsafe
                    {
                        uint* backBuffer = (uint*)lockedBitmap.Address;
                        backBuffer[y * lockedBitmap.RowBytes / 4 + x] = buffer[y * Width + x];
                    }
                }
            }
        }

        imageControl.InvalidateVisual();
    }

    private Complex ConvertToComplex(int x, int y)
    {
        double real = (x - Width / 2.0) / (0.5 * zoom * Width) + move.Real;
        double imaginary = (y - Height / 2.0) / (0.5 * zoom * Height) + move.Imaginary;
        return new Complex(real, imaginary);
    }

    private static uint GetColor(int iterations)
    {
        int r = (int)(9 * (1 - iterations / (float)MaxIterations) * Math.Pow(iterations / (float)MaxIterations, 3) * 255);
        int g = (int)(15 * Math.Pow(1 - iterations / (float)MaxIterations, 2) * Math.Pow(iterations / (float)MaxIterations, 2) * 255);
        int b = (int)(8.5 * Math.Pow(1 - iterations / (float)MaxIterations, 3) * (iterations / (float)MaxIterations) * 255);
        return (uint)((255 << 24) | (r << 16) | (g << 8) | b); // ARGB format
    }

    private static int Mandelbrot(Complex c)
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

    private void Window_PointerWheelChanged(object sender, Avalonia.Input.PointerWheelEventArgs e)
    {
        if (e.Delta.Y > 0)
            zoom *= 1.1;
        else if (e.Delta.Y < 0)
            zoom /= 1.1;

        DrawMandelbrot();
    }

    protected override void OnKeyDown(Avalonia.Input.KeyEventArgs e)
    {
        base.OnKeyDown(e);
        double panFactor = 0.1 / zoom;

        switch (e.Key)
        {
            case Avalonia.Input.Key.Left:
                move = new Complex(move.Real - panFactor, move.Imaginary);
                break;
            case Avalonia.Input.Key.Right:
                move = new Complex(move.Real + panFactor, move.Imaginary);
                break;
            case Avalonia.Input.Key.Up:
                move = new Complex(move.Real, move.Imaginary - panFactor);
                break;
            case Avalonia.Input.Key.Down:
                move = new Complex(move.Real, move.Imaginary + panFactor);
                break;
            case Avalonia.Input.Key.W:
                zoom *= 1.1;
                break;
            case Avalonia.Input.Key.S:
                zoom /= 1.1;
                break;
            case Avalonia.Input.Key.Escape:
                this.Close();
                break;
        }

        DrawMandelbrot();
    }
}

public struct Complex
{
    public double Real { get; }
    public double Imaginary { get; }

    public Complex(double real, double imaginary)
    {
        Real = real;
        Imaginary = imaginary;
    }

    public static Complex operator +(Complex c1, Complex c2)
        => new Complex(c1.Real + c2.Real, c1.Imaginary + c2.Imaginary);

    public static Complex operator *(Complex c1, Complex c2)
        => new Complex(c1.Real * c2.Real - c1.Imaginary * c2.Imaginary, c1.Real * c2.Imaginary + c1.Imaginary * c2.Real);

    public static Complex Zero => new Complex(0, 0);

    public static double Abs(Complex c)
        => Math.Sqrt(c.Real * c.Real + c.Imaginary * c.Imaginary);
}
