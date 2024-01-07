extern crate sfml;

use sfml::graphics::{Color, Image, RenderWindow, Sprite, Texture, RenderTarget};
use sfml::window::{Event, Style, Key};
use std::sync::{Arc, Mutex};
use std::thread;
use std::env;
use std::fs::File;
use std::io::Write;

const WIDTH: u32 = 1280;
const HEIGHT: u32 = 800;
const MAX_ITERATIONS: i32 = 1000;

// Define a struct to hold the data for each pixel
struct PixelData {
    x: u32,
    y: u32,
    color: Color,
}

fn get_color(iterations: i32) -> Color {
    let t = iterations as f32 / MAX_ITERATIONS as f32;
    let r = (9.0 * (1.0-t) * t * t * t * 255.0) as u8;
    let g = (15.0 * (1.0-t) * (1.0-t) * t * t * 255.0) as u8;
    let b = (8.5 * (1.0-t) * (1.0-t) * (1.0-t) * t * 255.0) as u8;

    Color::rgb(r, g, b)
}

fn convert_to_complex(x: i32, y: i32, zoom: f32, move_x: f32, move_y: f32) -> (f32, f32) {
    let real = (x as f32 - WIDTH as f32 / 2.0) / (0.5 * zoom * WIDTH as f32) + move_x;
    let imag = (y as f32 - HEIGHT as f32 / 2.0) / (0.5 * zoom * HEIGHT as f32) + move_y;
    (real, imag)
}

fn mandelbrot(c_real: f32, c_imag: f32) -> i32 {
    let mut z_real = 0.0;
    let mut z_imag = 0.0;
    let mut iter = 0;

    while z_real * z_real + z_imag * z_imag < 4.0 && iter < MAX_ITERATIONS {
        let temp = z_real * z_real - z_imag * z_imag + c_real;
        z_imag = 2.0 * z_real * z_imag + c_imag;
        z_real = temp;
        iter += 1;
    }

    iter
}

fn compute_mandelbrot_section(zoom: f32, move_x: f32, move_y: f32, start_y: u32, end_y: u32) -> Vec<PixelData> {
    let mut data = Vec::new();
    for x in 0..WIDTH {
        for y in start_y..end_y {
            let (c_real, c_imag) = convert_to_complex(x as i32, y as i32, zoom, move_x, move_y);
            let value = mandelbrot(c_real, c_imag);
            let color = get_color(value);
            data.push(PixelData { x, y, color });
        }
    }
    data
}

fn save_coordinates(zoom: f32, move_x: f32, move_y: f32, filename: &str) {
    let mut file = File::create(filename).expect("Unable to create file");
    writeln!(file, "{} {} {}", zoom, move_x, move_y).expect("Unable to write to file");
}

fn main() {
    let mut window = RenderWindow::new((WIDTH, HEIGHT), "Mandelbrot Set", Style::DEFAULT, &Default::default());
    window.set_vertical_sync_enabled(true);

    let mut image = Image::new(WIDTH, HEIGHT);
    for x in 0..WIDTH {
        for y in 0..HEIGHT {
            image.set_pixel(x, y, Color::BLACK);
        }
    }

    let mut texture = Texture::from_image(&image).unwrap();

    let mut zoom = 1.0;
    let mut move_x = 0.0;
    let mut move_y = 0.0;

    // Use zoom / pan from command-line argument
    let args: Vec<String> = env::args().collect();
    if args.len() == 4 {
        zoom = args[1].parse().unwrap();
        move_x = args[2].parse().unwrap();
        move_y = args[3].parse().unwrap();
    }

    while window.is_open() {
        while let Some(event) = window.poll_event() {
            // Event handling remains unchanged
        }

        let thread_count = num_cpus::get() as u32;
        let mut threads = vec![];

        for i in 0..thread_count {
            let start_y = i * HEIGHT / thread_count;
            let end_y = (i + 1) * HEIGHT / thread_count;
            let zoom_clone = zoom;
            let move_x_clone = move_x;
            let move_y_clone = move_y;
            threads.push(thread::spawn(move || {
                compute_mandelbrot_section(zoom_clone, move_x_clone, move_y_clone, start_y, end_y)
            }));
        }

        let mut pixel_data = Vec::new();
        for thread in threads {
            pixel_data.extend(thread.join().unwrap());
        }

        for pixel in pixel_data {
            image.set_pixel(pixel.x, pixel.y, pixel.color);
        }

        texture.update_from_image(&image, 0, 0);

        // Create and use the sprite after updating the texture
        let sprite = Sprite::with_texture(&texture);
        window.clear(Color::BLACK);
        window.draw(&sprite);
        window.display();
    }

    save_coordinates(zoom, move_x, move_y, "last_coordinates.txt");
}
