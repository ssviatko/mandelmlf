#include <iostream>
#include <complex>
#include <cstdlib>
#include <cstdint>

#include "ss-misc.h"

extern "C" uint64_t *iterate4(double *a_cr, double *a_ci, uint64_t a_max_iterations);

void cppscalar();
void mlvector();

int main(int argc, char **argv)
{
	cppscalar();
	mlvector();
	return 0;
}

void cppscalar()
{
	std::cout << "Mandel (C++, scalar) START: " << ss::iso8601_us() << std::endl;
	ss::targa24 mandel(2048, 1536);

	std::size_t const ixsize = 2048;
	std::size_t const iysize = 1536;
	double cxmax = 1.5;
	double cxmin = -2.5;
	double cymax = 1.7;
	double cymin = -1.7;
	unsigned int max_iterations = 1024;

	for (std::size_t ix = 0; ix < ixsize; ++ix)
		for (std::size_t iy = 0; iy < iysize; ++iy)
		{
			std::complex<double> c(cxmin + ix/(ixsize-1.0)*(cxmax-cxmin), cymin + iy/(iysize-1.0)*(cymax-cymin));
			std::complex<double> z = 0;
			unsigned int iterations;

			for (iterations = 0; iterations < max_iterations && std::abs(z) < 2.0; ++iterations) 
				z = z*z + c;

			mandel.plot(ix, iy, iterations % 256, iterations % 256, iterations % 256);
	        }
	mandel.write_image("mandel-scalar.tga");
	std::cout << "Mandel (C++, scalar) END  : " << ss::iso8601_us() << std::endl;

	return;
}

void mlvector()
{
	std::cout << "Mandel (ML, vector) START: " << ss::iso8601_us() << std::endl;
	ss::targa24 mandel(2048, 1536);

	std::size_t const ixsize = 2048;
	std::size_t const iysize = 1536;
	double cxmax = 1.5;
	double cxmin = -2.5;
	double cymax = 1.7;
	double cymin = -1.7;
	unsigned int max_iterations = 1024;

	for (std::size_t ix = 0; ix < ixsize; ix += 4)
		for (std::size_t iy = 0; iy < iysize; ++iy)
		{
			double cr[4], ci[4];
			for (int offset = 0; offset < 4; ++offset) {
				cr[offset] = cxmin + (ix + offset)/(ixsize - 1.0) * (cxmax - cxmin);
				ci[offset] = cymin + iy/(iysize - 1.0) * (cymax - cymin);
			}

			uint64_t *iterations = iterate4(cr, ci, max_iterations);

			for (int offset = 0; offset < 4; ++offset) {
				mandel.plot(ix + offset, iy, iterations[offset] % 256, iterations[offset] % 256, iterations[offset] % 256);
			}
	        }
	mandel.write_image("mandel-vector.tga");
	std::cout << "Mandel (ML, vector) END  : " << ss::iso8601_us() << std::endl;

	return;
}

