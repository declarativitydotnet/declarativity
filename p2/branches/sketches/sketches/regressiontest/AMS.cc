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
			<< std::endl
			<< "Usage: AMS " 
			<< "stream_length domain_size skew counters vectors.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distribution.\n"
			<< "  counters:      The number of counters per vector.\n"
			<< "  vectors:       The number of vectors per sketch.\n"
			<< std::endl;
		return -1;
	}

	size_t N = atol(argv[1]);
	size_t domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	size_t counters = atol(argv[4]);
	size_t vectors = atol(argv[5]);

	Sketches::AMS ams(counters, vectors);
	std::map<uint32_t, size_t> exact;
	std::map<uint32_t, size_t>::iterator itEx;

	std::cerr
		<< "Counters: " << ams.getVectorLength() << ", "
		<< "Vectors: " << ams.getNumberOfVectors() << std::endl;

	Tools::Random r;
	Tools::PRGZipf zipf(0L, domainSize, skew, &r);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	double totalError = 0.0, totalErrorNorm = 0.0;
	size_t totalAnswers = 0, total = 0;

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		for (size_t i = 1; i <= N; i++)
		{
			uint32_t l = zipf.nextLong();

			ams.insert(l);

			itEx = exact.find(l);
			if (itEx != exact.end())
				(*itEx).second++;
			else
				exact[l] = 1;

			if (i % 100 == 0)
			{
				// find top 20 most frequent.
				std::multimap<uint32_t, size_t> frequent;
				for (itEx = exact.begin(); itEx != exact.end(); itEx++)
					frequent.insert(
						std::pair<uint32_t, size_t>(
							(*itEx).second,
							(*itEx).first)
					);

				std::multimap<uint32_t, size_t>::iterator it = frequent.end(); it--;
				size_t top = 20;
				while (top > 0)
				{
					int64_t f = ams.getFrequency((*it).second);

					double error =
						std::abs(static_cast<double>(f) - static_cast<double>((*it).first)) / static_cast<double>((*it).first);

					totalError += error;
					totalAnswers++;

					top--;
					it--;
				}

				double F2 = 0.0;

				for (itEx = exact.begin(); itEx != exact.end(); itEx++)
					F2 += std::pow(static_cast<double>((*itEx).second), 2.0);

				totalErrorNorm += std::abs(ams.getF2Norm() - F2) / F2;
				total++;

				std::cerr
					<< "Insertions: " << i << ", "
					<< "Relative error (L_2 Norm): " << totalErrorNorm / total
					<< std::endl;
			}
		}

		std::cerr
			<< "Average relative error (top-20 frequency): "
			<< totalError / totalAnswers << std::endl
			<< "Average relative error (L_2 Norm): "
			<< totalErrorNorm / total
			<< std::endl;

		std::cerr << "Testing deletions." << std::endl;

		for (size_t i = 0; i < N / 10; i++)
		{
			uint32_t l = zipf.nextLong();

			ams.erase(l);

			itEx = exact.find(l);
			if (itEx != exact.end())
				(*itEx).second--;
		}

		std::cerr << "Testing getData operations." << std::endl;

		size_t len, len2;
		byte *data, *data2;

		ams.getData(&data, len);
		Sketches::AMS amsC(data);
		amsC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing copy constructor." << std::endl;

		Sketches::AMS amsC2(ams);
		ams.getData(&data, len);
		amsC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The copy constructor does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator<<." << std::endl;

		std::cerr << ams << std::endl;
		std::cerr << amsC2 << std::endl;

		std::cerr << "Testing the clear operation." << std::endl;
		amsC2.clear();
		std::cerr << amsC2 << std::endl;

		std::cerr << "Testing operator=" << std::endl;

		amsC2 = ams;

		ams.getData(&data, len);
		amsC2.getData(&data2, len2);

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

