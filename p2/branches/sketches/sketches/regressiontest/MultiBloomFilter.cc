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
			<< "Usage: MultiBloomFilter " 
			<< "stream_length domain_size skew bits hashes.\n"
			<< "Parameters:\n"
			<< "  stream_length: Total number of insertions.\n"
			<< "  domain_size:   Total number of distinct elements.\n"
			<< "  skew:          The skew parameter of the Zipf distribution.\n"
			<< "  bits:          The number of bits per vector in the filter.\n"
			<< "  hashes:        The number of hashes used per insertion.\n"
			<< std::endl;
		return -1;
	}

	size_t N = atol(argv[1]);
	size_t domainSize = atol(argv[2]);
	double skew  = atof(argv[3]);
	size_t bits = atol(argv[4]);
	size_t hashes = atol(argv[5]);

	Sketches::MultiBloomFilter bfU(bits, hashes, Sketches::HT_UNIVERSAL);
	Sketches::MultiBloomFilter bfS(bits, hashes, Sketches::HT_SHA1);
	std::set<std::string> exact;

	Tools::Random r;
	Tools::PRGZipf zipf(0L, domainSize, skew, &r);

	std::cerr << std::setprecision(2) << std::fixed;
	std::cout << std::setprecision(2) << std::fixed;

	int64_t falsePositiveDif = 0;

	try
	{
		std::cerr << "Testing insertions." << std::endl;

		for (size_t i = 1; i <= N; i++)
		{
			uint32_t l = zipf.nextLong();
			std::ostringstream ss;
			ss << l << std::flush;

			bfU.insert(l);
			bfS.insert(ss.str());
			exact.insert(ss.str());

			if (i % 100 == 0)
			{
				size_t falsePositivesU = 0;
				size_t falsePositivesS = 0;

				for (size_t j = 0; j < domainSize; j++)
				{
					if (bfU.contains(j)) falsePositivesU++;
					if (bfS.contains(j)) falsePositivesS++;
				}

				falsePositivesU -= exact.size();
				falsePositivesS -= exact.size();

				falsePositiveDif =
					static_cast<int64_t>(falsePositivesU) -
					static_cast<int64_t>(falsePositivesS);

				size_t bitsSetU = bfU.getNumberOfBitsSet();
				size_t bitsSetS = bfS.getNumberOfBitsSet();

				std::cerr
					<< "Insertions: " << i << ", "
					<< "False positives (%): " << 100.0 * falsePositivesU / domainSize<< " | " << 100.0 * falsePositivesS / domainSize << ", "
					<< "Bits set: " << bitsSetU << " | " << bitsSetS
					<< std::endl;
			}
		}

		std::cerr
			<< "False positive difference (U - S): " << falsePositiveDif
			<< std::endl;

		std::cerr << "Testing getData operations." << std::endl;

		size_t len, len2;
		byte *data, *data2;

		bfS.getData(&data, len);
		Sketches::MultiBloomFilter bfC(data);
		bfC.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		bfU.getData(&data, len);
		Sketches::MultiBloomFilter bfC2(data);
		bfC2.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The getData operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing copy constructor." << std::endl;

		Sketches::MultiBloomFilter bfC3(bfU);
		bfU.getData(&data, len);
		bfC3.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The copy constructor does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing operator<<." << std::endl;

		std::cerr << bfU << std::endl;
		std::cerr << bfC3 << std::endl;

		std::cerr << "Testing the clear operation." << std::endl;
		bfC3.clear();
		std::cerr << bfC3 << std::endl;

		std::cerr << "Testing the merge operation." << std::endl;
		bfC3.merge(bfU);

		bfU.getData(&data, len);
		bfC3.getData(&data2, len2);

		if (len != len2 || memcmp(data, data2, len) != 0)
			throw Tools::IllegalStateException(
				"Error: The merge operation does not work correctly!"
			);

		delete[] data;
		delete[] data2;

		std::cerr << "  O.k." << std::endl;
		std::cerr << "Testing the intersect operation." << std::endl;

		bfC3.clear();
		bfC3.intersect(bfU);

		bfU.getData(&data, len);
		bfC3.getData(&data2, len2);

		if (bfC3.getNumberOfBitsSet() > 0)
			throw Tools::IllegalStateException(
				"Error: The intersect operation does not work correctly!"
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

