// npm install canvas
const { createCanvas } = require('canvas');
const fs = require('fs');

const WIDTH = 800;
const HEIGHT = 600;
const MAX_ITERATIONS = 1000;

function mandelbrot(c) {
    let z = { real: 0, imag: 0 };
    let n = 0;
    while (abs(z) <= 2 && n < MAX_ITERATIONS) {
        z = add(mul(z, z), c);
        n++;
    }
    return n;
}

function getColor(iterations) {
    const t = iterations / MAX_ITERATIONS;
    const r = Math.round(9 * (1 - t) * Math.pow(t, 3) * 255);
    const g = Math.round(15 * Math.pow((1 - t), 2) * Math.pow(t, 2) * 255);
    const b = Math.round(8.5 * Math.pow((1 - t), 3) * t * 255);
    return `rgb(${r}, ${g}, ${b})`;
}

function complex(real, imag) {
    return { real, imag };
}

function abs(c) {
    return Math.sqrt(c.real * c.real + c.imag * c.imag);
}

function add(c1, c2) {
    return { real: c1.real + c2.real, imag: c1.imag + c2.imag };
}

function mul(c1, c2) {
    return {
        real: c1.real * c2.real - c1.imag * c2.imag,
        imag: c1.real * c2.imag + c1.imag * c2.real
    };
}

function main() {
    const canvas = createCanvas(WIDTH, HEIGHT);
    const ctx = canvas.getContext('2d');

    for (let x = 0; x < WIDTH; x++) {
        for (let y = 0; y < HEIGHT; y++) {
            const real = (x - WIDTH / 2) * 4 / WIDTH;
            const imag = (y - HEIGHT / 2) * 4 / HEIGHT;
            const color = getColor(mandelbrot(complex(real, imag)));
            ctx.fillStyle = color;
            ctx.fillRect(x, y, 1, 1);
        }
    }

    const buffer = canvas.toBuffer('image/png');
    fs.writeFileSync('mandelbrot_color.png', buffer);
    console.log('Mandelbrot set image saved as mandelbrot_color.png');
}

main();
