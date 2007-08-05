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
	if (argc != 6)
	{
		std::cerr
			<< "Usage: QuantileDigest stream_length domain_size skew k loops.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distribution.\n"
			<< "  k:             Compression parameter.\n"
			<< "  loops:         The number of individual experiments to run.\n"
			<< std::endl;
		return -1;

		return -1;
	}

	size_t N = atol(argv[1]);
	size_t domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	size_t k = atol(argv[4]);
	size_t loops = atol(argv[5]);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalerror = 0.0;
	double totalerror2 = 0.0;
	uint32_t seed = time(0);
	std::map<int32_t, uint32_t> exact;

	try
	{
		for (size_t cLoop = 0; cLoop < loops; cLoop++)
		{
			std::cerr << "Loop " << cLoop << ": ";

			Tools::Random r(seed + cLoop);
			Tools::PRGZipf zipf(0L, domainSize, skew, &r);

			exact.clear();
			for (size_t i = 0; i < N; i++)
			{
				uint32_t l = zipf.nextLong();

				if (exact.find(l) == exact.end())
					exact.insert(std::pair<int32_t, uint32_t>(l, 1));
				else
					exact[l]++;
			}

			Sketches::QuantileDigest qd(k, 0, domainSize, exact);

			int32_t value;
			uint32_t rank;
			qd.getQuantile(0.5, value, rank);

			double e = static_cast<double>(std::abs(static_cast<int32_t>(rank) - static_cast<int32_t>(N * 0.5))) / static_cast<double>(N * 0.5);
			totalerror += e;
			totalerror2 += std::pow(e, 2.0);
			std::cerr << e << std::endl;
		}

		std::cerr
			<< "Average relative error: "
			<< totalerror / loops << std::endl
			<< "Standard deviation: "
			<<	std::sqrt((loops * totalerror2 - std::pow(totalerror, 2.0)) /
				(loops * (loops - 1.0)))
			<< std::endl;

		Sketches::QuantileDigest qd(k, 0, domainSize, exact);

		std::cerr << "Testing getData operations." << std::endl;

		size_t len, len2;
		byte *data, *data2;

		qd.getData(&data, len);
		Sketches::QuantileDigest qdC(data);
		qdC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing copy constructor." << std::endl;

		Sketches::QuantileDigest qdC2(qd);
		qd.getData(&data, len);
		qdC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The copy constructor does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator =." << std::endl;

		qdC2 = qd;
		qd.getData(&data, len);
		qdC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The = operator does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
	}
	catch (Tools::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}

