/*
* Project name: Mandelbrot Fractals in openMP.
* Written by: Abdulaziz Alaa Karim Ahmed, Omar Essam.
* Course: High Performance Computing.
* Speedup: Approximately 2.6 faster.
* Output format: ppm image.
*/


#include <fstream>
#include <iostream>
#include <omp.h>
using namespace std;

// Declare a new datatype to hold the RGB values for every pixel.
typedef unsigned int pixelData[3];

// Check whether the number is in mandelbrot or not
int getMandelbrot(double a, double b, int maxIterations)
{
	int i = 0;
	double zReal = 0.0, zImaginary = 0.0;

	while (i < maxIterations && (zReal * zReal) + (zImaginary * zImaginary) < 4.0)
	{
		double temp = (zReal * zReal) - (zImaginary * zImaginary) + a;
		zImaginary = (2.0 * zReal * zImaginary) + b;
		zReal = temp;
		i++;
	}
	return i;
}

// Map the value to an appropriate value on the real axis
double mapToReal(int x, int imageWidth, double minReal, double maxReal)
{
	double range = maxReal - minReal;
	return x * (range / imageWidth) + minReal;
}

// Map the value to an appropriate value on the imaginary axis
double mapToImaginary(int y, int imageHeight, double minImaginary, double maxImaginary)
{
	double range = maxImaginary - minImaginary;
	return y * (range / imageHeight) + minImaginary;
}

int main()
{

	// Get input from file
	ifstream fin("mandelbrot parameters.txt");

	// Variables to hold image data
	int imageWidth, imageHeight, maxIterations;

	// Variables to hold the values of axes
	double minReal, maxReal, minImaginary, maxImaginary;

	// If failed to open file, output to console and return error code 1
	if (!fin)
	{
		cout << "Could not open file!" << endl;
		return 1;
	}

	// Read in the image data
	fin >> imageWidth >> imageHeight >> maxIterations;

	// Read in the values of axes
	fin >> minReal >> maxReal >> minImaginary >> maxImaginary;

	// Close input file stream
	fin.close();

	// Allocate a dynamic array in memory equal to the size of 
	// the image.
	pixelData *pixels = (pixelData*)malloc(sizeof(pixelData)*imageHeight*imageWidth);

	// Open the output file
	ofstream fout("mandelbrot fractal.ppm");

	// Output magic number that indicates "PPM ASCII" format
	fout << "P3" << endl; 

	// Output the dimensions of the image
	fout << imageWidth << " " << imageHeight << endl; 

	// Output maximum value of a pixel R,G,B value
	fout << "255" << endl; 

	// Execute code for every pixel in the image
	// Disable the dynamic setting of threads
	omp_set_dynamic(0);

	// Set the number of threads to number of processors
	omp_set_num_threads(omp_get_num_procs());

	// Set the array of pixels as a shared resource and parallelize code
	#pragma omp parallel shared(pixels)
	{

		// Parallelize for loop and schedule loop into equal sized chunks
		#pragma omp for schedule(static)

		// For every row in the image
		for (int y = 0; y < imageHeight; y++) 
		{
			// For every column in the image
			for (int x = 0; x < imageWidth; x++)
			{
				// Find real and imaginary parts corresponding to C
				double cReal = mapToReal(x, imageWidth, minReal, maxReal);
				double cImaginary = mapToImaginary(y, imageHeight, minImaginary, maxImaginary);

				// Call mandelbrot function
				int n = getMandelbrot(cReal, cImaginary, maxIterations);

				// Map the result into RGB values
				int r = ((int)(n) % 256);
				int g = ((int)(n*2) % 256);
				int b = ((int)(n*4) % 256);

				// Store the value of the RGB into an array of pixels
				pixels[y*imageWidth + x][0] = r;
				pixels[y*imageWidth + x][1] = g;
				pixels[y*imageWidth + x][2] = b;

			}
		}
	}

	// For every pixel entry in pixels array, output the RGB values to the image file
	for (int i = 0; i <= imageWidth*imageHeight; i++) {
		fout << pixels[i][0] << " " << pixels[i][1] << " " << pixels[i][2] << " ";
	}

	// Free the pixels array from memory
	free(pixels);

	// Close output file stream
	fout.close();

	// Return 0 on successful completion of main
	return 0;
}