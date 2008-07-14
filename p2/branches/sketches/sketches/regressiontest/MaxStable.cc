// Sketches Library
//
// Copyright (C) 2005 Marios Hadjieleftheriou
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

#include <Sketches.h>

int main(int argc, char** argv)
{
	if (argc != 5)
	{
		std::cerr
			<< std::endl
			<< "Usage: MaxStable " 
			<< "domain_size skew copies norm loops.\n"
			<< "Parameters:\n"
			<< "  domain_size: Total number of distinct elements.\n"
			<< "  copies:      The number of sketch copies.\n"
			<< "  norm:        The L_p norm to compute.\n"
			<< "  loops:       The number of individual experiments to run.\n"
			<< std::endl;
		return -1;
	}

	size_t domainSize = atol(argv[1]);
	size_t copies = atol(argv[2]);
	double dNorm = atof(argv[3]);
	size_t loops = atol(argv[4]);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalError = 0.0;
	size_t total = 0;
	uint32_t seed = time(0);

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		for (size_t cLoop = 0; cLoop < loops; cLoop++)
		{
			Tools::Random r(seed + cLoop);
			Sketches::MaxStable ms(copies, dNorm, r.nextUniformUnsignedShort());
			double exact = 0.0;

			for (size_t i = 1; i <= domainSize; i++)
			{
				double val = r.nextUniformDouble();

				ms.insert(i, val);
				exact += std::pow(val, dNorm);
			}

			exact = std::pow(exact, 1.0 / dNorm);
			double d = ms.getNorm();
			double error = std::abs(d - exact) / exact;
			totalError += error;
			total++;
			std::cerr << "Loop: " << cLoop << ", Error: " << error << ", " << d << " / " << exact << std::endl;
		}

		std::cerr
			<< "Average relative error: "
			<< totalError / total << std::endl;

		Tools::Random r(seed);
		Sketches::MaxStable ms(copies, 2.0);

		for (size_t i = 1; i <= domainSize; i++)
		{
			double val = r.nextUniformDouble();
			ms.insert(i, val);
		}

		std::cerr << "Testing getData operations." << std::endl;

		size_t len, len2;
		byte *data, *data2;

		ms.getData(&data, len);
		Sketches::MaxStable msC(data);
		msC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator<<." << std::endl;

		std::cerr << ms << std::endl;
		std::cerr << msC << std::endl;

		std::cerr << "Testing the clear operation." << std::endl;
		msC.clear();
		std::cerr << msC << std::endl;

		std::cerr << "  O.k." << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

