// npm install express canvas
const { createCanvas } = require('canvas');
const express = require('express');
const app = express();

const WIDTH = 1280;
const HEIGHT = 800;
const MAX_ITERATIONS = 500;

function getColor(iterations) {
    const t = iterations / MAX_ITERATIONS;
    const r = Math.floor(9 * (1 - t) * t * t * t * 255);
    const g = Math.floor(15 * (1 - t) * (1 - t) * t * t * 255);
    const b = Math.floor(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return `rgb(${r},${g},${b})`;
}

function convertToComplex(x, y, zoom, move) {
    const real = (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + move.real;
    const imag = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + move.imag;
    return { real, imag };
}

function mandelbrot(c) {
    let z = { real: 0, imag: 0 };
    let iter = 0;
    while (Math.sqrt(z.real * z.real + z.imag * z.imag) < 2 && iter < MAX_ITERATIONS) {
        const temp = {
            real: z.real * z.real - z.imag * z.imag + c.real,
            imag: 2 * z.real * z.imag + c.imag
        };
        z = temp;
        iter++;
    }
    return iter;
}

app.get('/', (req, res) => {
    const canvas = createCanvas(WIDTH, HEIGHT);
    const ctx = canvas.getContext('2d');

    const zoom = 1.0; // Modify as needed
    const move = { real: 0, imag: 0 }; // Modify as needed

    for (let x = 0; x < WIDTH; x++) {
        for (let y = 0; y < HEIGHT; y++) {
            const c = convertToComplex(x, y, zoom, move);
            const value = mandelbrot(c);
            ctx.fillStyle = getColor(value);
            ctx.fillRect(x, y, 1, 1);
        }
    }

    res.setHeader('Content-Type', 'image/png');
    canvas.pngStream().pipe(res);
});

app.listen(3000, () => {
    console.log('Server running at http://localhost:3000');
});
