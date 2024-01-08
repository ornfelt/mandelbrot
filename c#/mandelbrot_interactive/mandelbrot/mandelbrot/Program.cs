using System;
using System.Drawing;
using System.Numerics;
using System.Windows.Forms;
using System.Threading;

namespace mandelbrot
{
    public class MandelbrotForm : Form
    {
        private const int WIDTH = 1280;
        private const int HEIGHT = 800;
        private const int MAX_ITERATIONS = 1000;
        //private const int MAX_ITERATIONS = 10_000;

        private double zoom = 1.0f;
        private Complex move = new Complex(0, 0);
        // Using preset zoom / move (see last_coordinates.txt)
        //private double zoom = 368.4231567;
        //private Complex move = new Complex(-0.7577724344446324, -0.06907189212506637);
        // Very zoomed in (try high MaxIterations like 5000 or even 10000)
        //private double zoom = 754679.3818201923;
        //private Complex move = new Complex(-0.7572662262770462, -0.06744332997301555);

        private Bitmap image;
        private Thread redrawThread;
        private readonly object lockObject = new object();
        private bool updateRequested = false;
        private DateTime lastUpdate;
        private readonly TimeSpan redrawDelay = TimeSpan.FromMilliseconds(500);
        private bool redraw = true;

        public MandelbrotForm()
        {
            this.ClientSize = new Size(WIDTH, HEIGHT);
            this.Text = "Mandelbrot Set";
            image = new Bitmap(WIDTH, HEIGHT);

            this.MouseWheel += new MouseEventHandler(OnMouseWheel);
            this.KeyDown += new KeyEventHandler(OnKeyDown);

            redrawThread = new Thread(RedrawThreadFunction);
            redrawThread.Start();

            // Stop white flickering
            this.SetStyle(
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.UserPaint |
                ControlStyles.DoubleBuffer,
                true);
        }

        private void RedrawThreadFunction()
        {
            while (true)
            {
                lock (lockObject)
                {
                    while (!updateRequested)
                    {
                        Monitor.Wait(lockObject);
                    }

                    lastUpdate = DateTime.Now;
                    updateRequested = false;
                }

                // Wait for 500ms or until a new update request
                while ((DateTime.Now - lastUpdate) < redrawDelay)
                {
                    Thread.Sleep(50); // Sleep for short intervals to check again
                }

                this.Invoke(new Action(() =>
                {
                    DrawMandelbrot();
                    Invalidate(); // Redraw the form
                }));
            }
        }

        private void HandleEvent(Action updateAction)
        {
            lock (lockObject)
            {
                updateAction();
                updateRequested = true;
                Monitor.Pulse(lockObject);
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (redraw)
            {
                DrawMandelbrot();
                redraw = false;
            }

            e.Graphics.DrawImage(image, 0, 0);
            base.OnPaint(e);
        }

        private void OnMouseWheel(object sender, MouseEventArgs e)
        {
            HandleEvent(() =>
            {
                if (e.Delta > 0) zoom *= 1.1f;
                else zoom /= 1.1f;
                redraw = true;
            });
            //this.Invalidate(); // Causes the form to be redrawn
        }

        private void OnKeyDown(object sender, KeyEventArgs e)
        {
            HandleEvent(() =>
            {
                switch (e.KeyCode)
                {
                    case Keys.Left:
                        move = new Complex(move.Real - 0.1f / zoom, move.Imaginary);
                        break;
                    case Keys.Right:
                        move = new Complex(move.Real + 0.1f / zoom, move.Imaginary);
                        break;
                    case Keys.Up:
                        move = new Complex(move.Real, move.Imaginary - 0.1f / zoom);
                        break;
                    case Keys.Down:
                        move = new Complex(move.Real, move.Imaginary + 0.1f / zoom);
                        break;
                    case Keys.Escape:
                        SaveCoordinates(zoom, move, "last_coordinates.txt");
                        this.Close();
                        // Brute force exit (the above doesn't fully exit)
                        System.Environment.Exit(1);
                        break;
                }

                redraw = true;
                //this.Invalidate();
            });
        }

        private void SaveCoordinates(double zoom, Complex move, string filename)
        {
            try
            {
                using (var writer = new StreamWriter(filename))
                {
                    writer.WriteLine($"{zoom} {move.Real} {move.Imaginary}");
                }
            }
            catch (Exception ex)
            {
                // Handle any exceptions here, such as logging or showing an error message
                Console.WriteLine("Error saving coordinates: " + ex.Message);
            }
        }

        private void DrawMandelbrot()
        {
            // Parallelize drawing for improved performance
            // To specify threads to use:
            //var parallelOptions = new ParallelOptions { MaxDegreeOfParallelism = 4 };
            //Parallel.For(0, Height, parallelOptions, y =>
            Parallel.For(0, HEIGHT, y =>
            {
                for (int x = 0; x < WIDTH; x++)
                {
                    Complex c = ConvertToComplex(x, y);
                    int value = Mandelbrot(c);
                    Color color = GetColor(value);
                    lock (image)
                    {
                        image.SetPixel(x, y, color);
                    }
                }
            });
        }

        private Color GetColor(int iterations)
        {
            double t = (double)iterations / MAX_ITERATIONS;
            int r = (int)(9 * (1 - t) * t * t * t * 255);
            int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
            int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            return Color.FromArgb(r, g, b);
        }

        private Complex ConvertToComplex(int x, int y)
        {
            double real = (x - WIDTH / 2.0) / (0.5 * zoom * WIDTH) + move.Real;
            double imag = (y - HEIGHT / 2.0) / (0.5 * zoom * HEIGHT) + move.Imaginary;
            return new Complex(real, imag);
        }

        private int Mandelbrot(Complex c)
        {
            Complex z = new Complex(0, 0);
            int iter = 0;

            while (Complex.Abs(z) < 2 && iter < MAX_ITERATIONS)
            {
                z = z * z + c;
                iter++;
            }

            return iter;
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (redrawThread != null && redrawThread.IsAlive)
            {
                //redrawThread.Abort(); // Forcefully terminate the thread
            }
            base.OnFormClosing(e);
        }

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MandelbrotForm());
        }

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        //[STAThread]
        //static void Main()
        //{
        //    // To customize application configuration such as set high DPI settings or default font,
        //    // see https://aka.ms/applicationconfiguration.
        //    ApplicationConfiguration.Initialize();
        //    Application.Run(new Form1());
        //}
    }
}