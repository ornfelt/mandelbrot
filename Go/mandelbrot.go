package main

import (
    "image"
    "image/color"
    "image/png"
    "math/cmplx"
    "os"
)

const (
    Width        = 1920
    Height       = 1080
    MaxIterations = 5000
)

func mandelbrot(c complex128) int {
    var z complex128
    for n := 0; n < MaxIterations; n++ {
        z = z*z + c
        if cmplx.Abs(z) > 2 {
            return n
        }
    }
    return MaxIterations
}

func getColor(iterations int) color.RGBA {
    t := float64(iterations) / MaxIterations

    r := uint8(9 * (1-t) * t * t * t * 255)
    g := uint8(15 * (1-t) * (1-t) * t * t * 255)
    b := uint8(8.5 * (1-t) * (1-t) * (1-t) * t * 255)

    return color.RGBA{R: r, G: g, B: b, A: 255}
}

func main() {
    img := image.NewRGBA(image.Rect(0, 0, Width, Height))

    for y := 0; y < Height; y++ {
        for x := 0; x < Width; x++ {
            real := float64(x-Width/2)*4/Width
            imag := float64(y-Height/2)*4/Height
            c := complex(real, imag)
            color := getColor(mandelbrot(c))
            img.Set(x, y, color)
        }
    }

    // Save to PNG
    pngFile, _ := os.Create("mandelbrot.png")
    defer pngFile.Close()
    png.Encode(pngFile, img)
}
