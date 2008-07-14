// Tools Library
//
// Copyright (C) 2004  Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#include <Tools.h>

int main(int argc, char** argv)
{
#if HAVE_GETTIMEOFDAY
	Tools::ResourceUsage ru;
	Tools::Random r;

	ru.start();
	for (uint32_t cIndex = 0; cIndex < 1000; cIndex++)
	{
		std::cout << r.nextSkewedDouble(10.0, 50.0, Tools::LVL_LOW) << std::endl;
	}
	ru.stop();

	std::cout << std::endl;

	ru.start();
	for (uint32_t cIndex = 0; cIndex < 1000; cIndex++)
	{
		std::cout << r.nextUniformDouble() << std::endl;
	}
	ru.stop();

	std::cerr << "Total time: " << ru.getTotalTime() << std::endl;
	std::cerr << "User time: " << ru.getUserTime() << std::endl;
	std::cerr << "System time: " << ru.getSystemTime() << std::endl;
	std::cerr << "Page faults: " << ru.getPageFaults() << std::endl;
	std::cerr << "Read IO: " << ru.getReadIO() << std::endl;
	std::cerr << "Write IO: " << ru.getWriteIO() << std::endl;
	std::cerr << "Peak memory usage: " << ru.getTotalMemoryUsage() << std::endl;

	std::cout << std::endl;

	ru.reset();
	ru.start();
	for (uint32_t cIndex = 0; cIndex < 1000; cIndex++)
	{
		std::cout << r.nextUniformDouble(10.0, 50.0) << std::endl;
	}
	ru.stop();

	std::cerr << "Total time: " << ru.getTotalTime() << std::endl;
	std::cerr << "User time: " << ru.getUserTime() << std::endl;
	std::cerr << "System time: " << ru.getSystemTime() << std::endl;
	std::cerr << "Page faults: " << ru.getPageFaults() << std::endl;
	std::cerr << "Read IO: " << ru.getReadIO() << std::endl;
	std::cerr << "Write IO: " << ru.getWriteIO() << std::endl;
	std::cerr << "Peak memory usage: " << ru.getTotalMemoryUsage() << std::endl;
#endif

	return 0;
}
