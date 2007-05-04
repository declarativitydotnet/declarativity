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

#ifndef __sketches_h
#define __sketches_h

#include <tools/Tools.h>

namespace Sketches
{
	enum HashType
	{
		HT_UNIVERSAL = 0x0,
		HT_SHA1
	};

	/**************************************************/
	/*           FM and Summation FM Sketch           */
	/**************************************************/
	// This implements the PCSA FM sketch as described in:
	// P. Flajolet and G. N. Martin.
	// Probabilistic Counting Algorithms for data base applications.
	// Journal of Computer and System Sciences, 31, 1985
	//
	// It also implements the sumation FM sketch which
	// supports efficient execution of "bulk insertions" to
	// the FM sketch, where each insertion has weight larger
	// than one. The sumation FM code has been kindly contributed
	// by the authors of:
	// J. Considine, F. Li, G. Kollios, J. Byers
	// Approximate aggregation techniques for sensor databases, ICDE 2004
	//
	// For HT_UNIVERSAL the item ids are converted
	// from std::string to UniversalHash::value_type using atoll.
	// For HT_SHA1 the domain size is infinite,
	// but the HT_SHA1 version does not support bulk insertions.
	// It inserts the values one by one iteratively.
	// Below is a table of supported domain sizes and sketch
	// sizes, depending on the hash function used:
	//
	// Hash type      Domain size     Bits per Bitmap    Bitmaps
	// HT_UNIVERSAL   value_type      infinite           infinite
	// HT_SHA1        infinite        152                256
	class FM
	{
	public:
		FM(
			size_t bits = 32,
			size_t bitmaps = 64,
			HashType type = HT_SHA1
		);
		FM(
			size_t bits,
			size_t bitmaps,
			uint16_t seed = 0x330E
		);
		FM(const byte* data);
		virtual ~FM();

    // Marshal/unmarshal methods (added by alexras@acm.org)
    virtual void marshal(XDR *x) const;
    static Sketches::FM *unmarshal(XDR *x);

		virtual void insert(const std::string& id, uint64_t val = 1);
		virtual void erase(const std::string& id, uint64_t val = 1);
		virtual void insert(
			const Tools::UniversalHash::value_type& id,
			uint64_t val = 1
		);
		virtual void erase(
			const Tools::UniversalHash::value_type& id,
			uint64_t val = 1
		);
		virtual void clear();

		virtual uint64_t getCount() const;

		virtual void merge(const FM& f);
			// merge this instance with the input FM and store in this instance.
		virtual void merge(const FM& f1, const FM& f2);
			// merge the input FMs and store the result in this instance.
		virtual FM getMerged(const FM& f) const;
			// merge this instance with the input FM
			// and store the result only on the returned value.
		virtual void reset(const byte* data);
		virtual bool isSubsumedBy(const FM& f) const;

		virtual size_t getVectorLength() const;
		virtual size_t getNumberOfVectors() const;
		virtual size_t getUncompressedSize() const;
		virtual uint16_t getSeed() const;

		virtual size_t getSize() const;
		virtual void getData(byte** data, size_t& length) const;

	private:
		uint32_t pickOffset(Tools::Random& r) const;
		uint32_t fillLength(uint32_t val) const;
		int32_t pickBinomial(int32_t nIn, double pIn, Tools::Random& rand) const;

		static const double PHI = 0.77351;

		uint16_t m_seed;

		friend std::ostream& Sketches::operator<<(
			std::ostream& os, const Sketches::FM& s
		);
	protected:
	  std::vector<std::vector<bool> > m_bitmap;
    HashType m_type;
	};

	std::ostream& operator<<(std::ostream& os, const Sketches::FM& s);

	/**************************************************/
	/*                CountMin FM Sketch              */
	/**************************************************/
	// TODO: implement getData
	// FIXME: I shouldn't be storing the hashes here.
	// I should be generating them on the fly using a
	// PRG seed.
	class CountMinFM
	{
	public:
		CountMinFM(
			const std::string& id,
			const std::map<std::string, uint64_t>& m,
			size_t counters,
			size_t hashes,
			size_t fmSize,
			size_t fmBitmaps,
			Tools::Random& r
		);
		CountMinFM(
			const std::string& id,
			const std::map<std::string, uint64_t>& m,
			size_t counters,
			size_t hashes,
			size_t fmSize,
			size_t fmBitmaps,
			HashType t
		);
		CountMinFM(
			uint64_t id,
			const std::map<std::string, uint64_t>& m,
			size_t counters,
			size_t hashes,
			size_t fmSize,
			size_t fmBitmaps,
			Tools::Random& r
		);
		CountMinFM(
			uint64_t id,
			const std::map<std::string, uint64_t>& m,
			size_t counters,
			size_t hashes,
			size_t fmSize,
			size_t fmBitmaps,
			HashType t
		);
		virtual ~CountMinFM();

    	virtual void marshal(XDR *x) const;
    	static Sketches::CountMinFM *unmarshal(XDR *x);

		virtual uint64_t getFrequency(const std::string& id) const;

		virtual void merge(const CountMinFM& in);

		virtual size_t getVectorLength() const;
		virtual size_t getNumberOfHashes() const;

		virtual size_t getSize() const;

	protected:
		std::vector<FM> m_filter;
		std::vector<Tools::UniversalHash> m_hash;
	private:
		void initialize(
			const std::string& id,
			const std::map<std::string, uint64_t>& m,
			size_t counters,
			size_t hashes,
			size_t fmSize,
			size_t fmBitmaps,
			HashType t,
			Tools::Random& r
		);

		HashType m_type;
		std::string m_id;
			// Every CountMinFM should have a unique id,
			// for proper merging.
		size_t m_counters;
		size_t m_hashes;
	};

	/**************************************************/
	/*                Useful functions                */
	/**************************************************/

	template<class T> T getMedian(std::multiset<T>& v)
	{
		typename std::multiset<T>::iterator it = v.begin();
		size_t r = static_cast<size_t>(std::ceil(static_cast<double>(v.size()) / 2.0));

		for (size_t i = 1; i < r; i++) it++;

		if (v.size() % 2 != 0)
		{
			return *it;
		}
		else
		{
			T a = *it; it++;
			T b = *it;
			return static_cast<T>((a + b) / 2.0);
		}
	}

	bool isPrime(uint64_t P);
	uint64_t getPrime(uint64_t low, uint64_t high, Tools::Random& r);
	uint64_t getPrime(uint64_t low, uint64_t high);
};

#endif /* __sketches_h */

