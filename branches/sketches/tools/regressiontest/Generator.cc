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

#include <assert.h>
#include <iostream>

int32_t nextRandomLong(int32_t min, int32_t max)
{
	return static_cast<int32_t>(static_cast<double>(min) + ((static_cast<double>(max) - static_cast<double>(min)) * drand48()));
}

double nextRandomDouble(double min, double max)
{
	return (min + ((max - min) * drand48()));
}

double nextRandomDouble()
{
	return drand48();
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " number_of_data." << std::endl;
		return -1;
	}

	srand48(time(0));

	uint32_t numberOfObjects = atol(argv[1]);

	std::cout << std::fixed;

	for (uint32_t i = 0; i < numberOfObjects; i++)
	{
		double x = nextRandomDouble();
		double y = nextRandomDouble();
		double dx = nextRandomDouble(0.0001, 0.1);
		double dy = nextRandomDouble(0.0001, 0.1);

		std::cout << x << " " << y << " " << x + dx << " " << y + dy << std::endl;
	}

	return 0;
}

