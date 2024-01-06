import pygame
import numpy as np
import threading
import multiprocessing
from pygame.locals import *

# Constants
WIDTH = 1280
HEIGHT = 800
MAX_ITERATIONS = 50
USE_MUL_THREADS = True

def get_color(iterations):
    t = iterations / MAX_ITERATIONS
    r = int(9 * (1-t) * t * t * t * 255)
    g = int(15 * (1-t) * (1-t) * t * t * 255)
    b = int(8.5 * (1-t) * (1-t) * (1-t) * t * 255)
    return (r, g, b)

def convert_to_complex(x, y, zoom, move):
    real = (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + move.real
    imag = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + move.imag
    return complex(real, imag)

def mandelbrot(c):
    z = 0 + 0j
    iter = 0
    while abs(z) < 2 and iter < MAX_ITERATIONS:
        z = z * z + c
        iter += 1
    return iter

def compute_mandelbrot_section(surface, zoom, move, start_y, end_y):
    for x in range(WIDTH):
        for y in range(start_y, end_y):
            c = convert_to_complex(x, y, zoom, move)
            value = mandelbrot(c)
            color = get_color(value)
            surface.set_at((x, y), color)

def main():
    pygame.init()
    window = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Mandelbrot Set")
    surface = pygame.Surface((WIDTH, HEIGHT))
    zoom = 1.0
    move = complex(0, 0)
    redraw = True

    while True:
        for event in pygame.event.get():
            if event.type == QUIT:
                pygame.quit()
                return

            if event.type == MOUSEBUTTONDOWN:
                if event.button == 4: zoom *= 1.1
                elif event.button == 5: zoom /= 1.1
                redraw = True

        keys = pygame.key.get_pressed()
        if keys[K_LEFT]:
            move -= complex(0.1 / zoom, 0)
            redraw = True
        if keys[K_RIGHT]:
            move += complex(0.1 / zoom, 0)
            redraw = True
        if keys[K_UP]:
            move -= complex(0, 0.1 / zoom)
            redraw = True
        if keys[K_DOWN]:
            move += complex(0, 0.1 / zoom)
            redraw = True
        if keys[K_ESCAPE]:
            pygame.quit()
            return

        if redraw:
            # Determine the number of threads to use
            thread_count = multiprocessing.cpu_count() if USE_MUL_THREADS else 1
            threads = []

            for i in range(thread_count):
                start_y = i * HEIGHT // thread_count
                end_y = (i + 1) * HEIGHT // thread_count
                thread = threading.Thread(target=compute_mandelbrot_section, args=(surface, zoom, move, start_y, end_y))
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

            window.blit(surface, (0, 0))
            pygame.display.flip()
            redraw = False

if __name__ == "__main__":
    main()
