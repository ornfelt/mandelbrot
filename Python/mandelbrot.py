from PIL import Image

# pip install pillow

WIDTH = 800
HEIGHT = 600
MAX_ITERATIONS = 1000

def mandelbrot(c):
    z = 0
    n = 0
    while abs(z) <= 2 and n < MAX_ITERATIONS:
        z = z*z + c
        n += 1
    return n

def get_color(iterations):
    t = iterations / MAX_ITERATIONS
    r = int(9 * (1 - t) * t**3 * 255)
    g = int(15 * (1 - t)**2 * t**2 * 255)
    b = int(8.5 * (1 - t)**3 * t * 255)
    return (r, g, b)

def main():
    img = Image.new('RGB', (WIDTH, HEIGHT), color = 'black')
    pixels = img.load()

    for x in range(WIDTH):
        for y in range(HEIGHT):
            real = (x - WIDTH / 2) * 4 / WIDTH
            imag = (y - HEIGHT / 2) * 4 / HEIGHT
            c = complex(real, imag)
            color = get_color(mandelbrot(c))
            pixels[x, y] = color

    img.save('mandelbrot_color.png')
    img.save('mandelbrot_color.bmp')

    print("Mandelbrot set images saved as mandelbrot_color.png and mandelbrot_color.bmp")

if __name__ == "__main__":
    main()
