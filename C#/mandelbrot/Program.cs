// dotnet add package SixLabors.ImageSharp
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using System;
using System.Numerics;

class Program
{
    const int Width = 1920;
    const int Height = 1080;
    const int MaxIterations = 5000;

    static void Main()
    {
        using (var image = new Image<Rgba32>(Width, Height))
        {
            for (int x = 0; x < Width; x++)
            {
                for (int y = 0; y < Height; y++)
                {
                    double real = (x - Width / 2.0) * 4.0 / Width;
                    double imag = (y - Height / 2.0) * 4.0 / Height;
                    int iterations = Mandelbrot(new Complex(real, imag));
                    Rgba32 color = GetColor(iterations);
                    image[x, y] = color;
                }
            }

            image.Save("mandelbrot_color.png");
            image.SaveAsBmp("mandelbrot_color.bmp");
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

    static Rgba32 GetColor(int iterations)
    {
        float t = (float)iterations / MaxIterations;
        byte r = (byte)(9 * (1 - t) * Math.Pow(t, 3) * 255);
        byte g = (byte)(15 * Math.Pow(1 - t, 2) * Math.Pow(t, 2) * 255);
        byte b = (byte)(8.5 * Math.Pow(1 - t, 3) * t * 255);
        return new Rgba32(r, g, b, 255);  // Alpha value is 255 (fully opaque)
    }
}
