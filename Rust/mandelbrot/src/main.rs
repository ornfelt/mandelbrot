use std::fs::File;
use std::io::{self, Write};
use std::path::Path;
use image::{ImageBuffer, RgbImage, Rgb};

const WIDTH: usize = 800;
const HEIGHT: usize = 600;
const MAX_ITERATIONS: usize = 1000;

fn mandelbrot(real: f64, imag: f64) -> usize {
    let mut z_real = 0.0;
    let mut z_imag = 0.0;
    let mut iterations = 0;

    while z_real * z_real + z_imag * z_imag < 4.0 && iterations < MAX_ITERATIONS {
        let temp_real = z_real * z_real - z_imag * z_imag + real;
        let temp_imag = 2.0 * z_real * z_imag + imag;
        z_real = temp_real;
        z_imag = temp_imag;

        iterations += 1;
    }

    iterations
}

fn get_color(iterations: usize) -> (u8, u8, u8) {
    let t = iterations as f64 / MAX_ITERATIONS as f64;

    let r = (9.0 * (1.0 - t) * t * t * t * 255.0) as u8;
    let g = (15.0 * (1.0 - t) * (1.0 - t) * t * t * 255.0) as u8;
    let b = (8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255.0) as u8;

    (r, g, b)
}

fn save_bitmap<P: AsRef<Path>>(filename: P, colors: &[(u8, u8, u8)]) -> io::Result<()> {
    let mut file = File::create(filename)?;

    let file_header = [
        b'B', b'M', 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        54, 0, 0, 0
    ];

    let info_header = [
        40, 0, 0, 0, 
        WIDTH as u8, (WIDTH >> 8) as u8, (WIDTH >> 16) as u8, (WIDTH >> 24) as u8,
        HEIGHT as u8, (HEIGHT >> 8) as u8, (HEIGHT >> 16) as u8, (HEIGHT >> 24) as u8,
        1, 0, 
        24, 0,
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0
    ];

    let file_size = 54 + WIDTH * HEIGHT * 3;
    file.write_all(&file_header)?;
    file.write_all(&info_header)?;

    for y in 0..HEIGHT {
        for x in 0..WIDTH {
            let index = y * WIDTH + x;
            let (r, g, b) = colors[index];
            file.write_all(&[b, g, r])?; // BMP uses BGR format
        }
    }

    Ok(())
}

fn main() -> io::Result<()> {
    let mut img: RgbImage = ImageBuffer::new(WIDTH as u32, HEIGHT as u32);
    let mut bmp_colors = vec![(0, 0, 0); WIDTH * HEIGHT];

    for y in 0..HEIGHT {
        for x in 0..WIDTH {
            let real = (x as f64 - WIDTH as f64 / 2.0) * 4.0 / WIDTH as f64;
            let imag = (y as f64 - HEIGHT as f64 / 2.0) * 4.0 / HEIGHT as f64;
            let iterations = mandelbrot(real, imag);
            let color = get_color(iterations);

            img.put_pixel(x as u32, y as u32, Rgb([color.0, color.1, color.2]));
            bmp_colors[y * WIDTH + x] = (color.0, color.1, color.2);
        }
    }

    img.save("mandelbrot_color.png").map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
    println!("Mandelbrot set image saved as mandelbrot_color.png");

    save_bitmap("mandelbrot_color.bmp", &bmp_colors)?;
    println!("Mandelbrot set image saved as mandelbrot_color.bmp");

    Ok(())
}
