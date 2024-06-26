// Program to compute the area of a Mandelbrot set.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

# define NPOINTS 1000
# define MAXITER 1000



struct d_complex
{
   double r;
   double i;
};

int num_outside = 0;



// Does the iteration z=z*z+c, until |z| > 2 when point is known to be outside set
// If loop count reaches MAXITER, point is considered to be inside the set
void test_point(struct d_complex c)
{
	struct d_complex z = c;
	double temp;
	for ( int iter = 0; iter < MAXITER; ++iter )
	{
		temp = z.r*z.r - z.i*z.i + c.r;
		z.i = 2*z.r*z.i + c.i;
		z.r = temp;
		if ( z.r*z.r + z.i*z.i > 4.0 )
		{
			#pragma omp atomic
				++num_outside;
			break;
		}
	}
}



int main()
{
	const double eps = 1.0e-5;

//	Loop over grid of points in the complex plane which contains the Mandelbrot
//	set, testing each point to see whether it is inside or outside the set.
	#pragma omp parallel default(shared)
	{
		struct d_complex c;
		#pragma omp for
		for ( int i = 0; i < NPOINTS; ++i )
			for ( int j = 0; j < NPOINTS; ++j )
			{
				c.r = -2.0 + 2.5 * i / (double)NPOINTS + eps;
				c.i = 1.125 * j / (double)(NPOINTS) + eps;
				test_point(c);
			}
	}

	double area = 2.0*2.5*1.125*(NPOINTS*NPOINTS - num_outside) / (NPOINTS*NPOINTS);
	double error = area / NPOINTS;

	printf("Area of Mandlebrot set = %12.8f +/- %12.8f\n", area, error);
	printf("Correct answer should be around 1.510659\n");
}

// gcc -std=c17 -Wall -Wextra -O0 -fopenmp -o mandel mandel.c
