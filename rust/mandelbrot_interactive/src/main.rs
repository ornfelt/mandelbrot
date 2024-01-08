use minifb::{Key, Window, WindowOptions};
use num_complex::Complex;
use std::sync::{Arc, Mutex};
use std::thread;

const WIDTH: usize = 800;
const HEIGHT: usize = 700;
const MAX_ITERATIONS: i32 = 200;

fn get_color(iterations: i32) -> u32 {
    let t = iterations as f64 / MAX_ITERATIONS as f64;
    let r = (9.0 * (1.0 - t) * t * t * t * 255.0) as u32;
    let g = (15.0 * (1.0 - t) * (1.0 - t) * t * t * 255.0) as u32;
    let b = (8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255.0) as u32;
    (r << 16) | (g << 8) | b
}

fn convert_to_complex(x: usize, y: usize, zoom: f32, move_x: f32, move_y: f32) -> Complex<f32> {
    let real = (x as f32 - WIDTH as f32 / 2.0) / (0.5 * zoom * WIDTH as f32) + move_x;
    let imag = (y as f32 - HEIGHT as f32 / 2.0) / (0.5 * zoom * HEIGHT as f32) + move_y;
    Complex::new(real, imag)
}

fn mandelbrot(c: Complex<f32>) -> i32 {
    let mut z = Complex::new(0.0, 0.0);
    let mut iter = 0;

    while z.norm() <= 2.0 && iter < MAX_ITERATIONS {
        z = z * z + c;
        iter += 1;
    }

    iter
}

fn main() {
    let mut window = Window::new(
        "Mandelbrot Set - ESC to exit",
        WIDTH,
        HEIGHT,
        WindowOptions::default(),
    )
    .unwrap_or_else(|e| {
        panic!("{}", e);
    });

    let mut buffer = vec![0; WIDTH * HEIGHT];
    let mut zoom = 1.0f32;
    let mut move_x = 0.0f32;
    let mut move_y = 0.0f32;

    while window.is_open() && !window.is_key_down(Key::Escape) {
        let buffer_clone = Arc::new(Mutex::new(buffer.clone()));
        //let threads = 4;
        let threads = num_cpus::get() as usize;
        let mut handles = vec![];

        for i in 0..threads {
            let buffer_clone = Arc::clone(&buffer_clone);
            let handle = thread::spawn(move || {
                let start_y = i * HEIGHT / threads;
                let end_y = (i + 1) * HEIGHT / threads;

                for x in 0..WIDTH {
                    for y in start_y..end_y {
                        let c = convert_to_complex(x, y, zoom, move_x, move_y);
                        let value = mandelbrot(c);
                        let color = get_color(value);

                        let mut buffer = buffer_clone.lock().unwrap();
                        buffer[x + y * WIDTH] = color;
                    }
                }
            });
            handles.push(handle);
        }

        for handle in handles {
            handle.join().unwrap();
        }

        buffer = Arc::try_unwrap(buffer_clone).unwrap().into_inner().unwrap();
        window.update_with_buffer(&buffer, WIDTH, HEIGHT).unwrap();

        if window.is_key_down(Key::W) {
            zoom *= 1.1;
        }
        if window.is_key_down(Key::S) {
            zoom /= 1.1;
        }
        if window.is_key_down(Key::A) {
            move_x -= 0.1 / zoom;
        }
        if window.is_key_down(Key::D) {
            move_x += 0.1 / zoom;
        }
        if window.is_key_down(Key::Up) {
            move_y -= 0.1 / zoom;
        }
        if window.is_key_down(Key::Down) {
            move_y += 0.1 / zoom;
        }
        if window.is_key_down(Key::Escape) {
            break;
        }
    }
}
