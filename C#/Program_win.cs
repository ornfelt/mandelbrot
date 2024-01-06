// dotnet add package System.Drawing.Common
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Numerics;

class MandelbrotGenerator
{
    const int Width = 800;
    const int Height = 600;
    const int MaxIterations = 1000;

    static void Main()
    {
        using (var bitmap = new Bitmap(Width, Height))
        {
            for (int x = 0; x < Width; x++)
            {
                for (int y = 0; y < Height; y++)
                {
                    double real = (x - Width / 2.0) * 4.0 / Width;
                    double imag = (y - Height / 2.0) * 4.0 / Height;
                    int iterations = Mandelbrot(new Complex(real, imag));
                    Color color = GetColor(iterations);
                    bitmap.SetPixel(x, y, color);
                }
            }

            bitmap.Save("mandelbrot_color.png", ImageFormat.Png);
            bitmap.Save("mandelbrot_color.bmp", ImageFormat.Bmp);
            Console.WriteLine("Mandelbrot set image saved as mandelbrot_color.png and mandelbrot_color.bmp");
        }
    }

    static int Mandelbrot(Complex c)
    {
        Complex z = 0;
        int iterations = 0;

        while (Complex.Abs(z) <= 2 && iterations < MaxIterations)
        {
            z = z * z + c;
            iterations++;
        }

        return iterations;
    }

    static Color GetColor(int iterations)
    {
        double t = (double)iterations / MaxIterations;
        int r = (int)(9 * (1 - t) * Math.Pow(t, 3) * 255);
        int g = (int)(15 * Math.Pow(1 - t, 2) * Math.Pow(t, 2) * 255);
        int b = (int)(8.5 * Math.Pow(1 - t, 3) * t * 255);
        return Color.FromArgb(r, g, b);
    }
}
