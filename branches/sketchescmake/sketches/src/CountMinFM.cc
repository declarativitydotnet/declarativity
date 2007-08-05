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

Sketches::CountMinFM::CountMinFM(
  const std::string& id,
  const std::map<std::string, uint64_t>& m,
  size_t counters,
  size_t hashes,
  size_t fmSize,
  size_t fmBitmaps,
  Tools::Random& r
)
{
  initialize(id, m, counters, hashes, fmSize, fmBitmaps, HT_UNIVERSAL, r);
}

Sketches::CountMinFM::CountMinFM(
  const std::string& id,
  const std::map<std::string, uint64_t>& m,
  size_t counters,
  size_t hashes,
  size_t fmSize,
  size_t fmBitmaps,
  HashType t
)
{
  Tools::Random r;
  initialize(id, m, counters, hashes, fmSize, fmBitmaps, t, r);
}

Sketches::CountMinFM::CountMinFM(
  uint64_t id,
  const std::map<std::string, uint64_t>& m,
  size_t counters,
  size_t hashes,
  size_t fmSize,
  size_t fmBitmaps,
  Tools::Random& r
)
{
  std::stringstream ss;
  ss.write(reinterpret_cast<const char*>(&id), sizeof(uint64_t));
  initialize(ss.str(), m, counters, hashes, fmSize, fmBitmaps, HT_UNIVERSAL, r);
}

Sketches::CountMinFM::CountMinFM(
  uint64_t id,
  const std::map<std::string, uint64_t>& m,
  size_t counters,
  size_t hashes,
  size_t fmSize,
  size_t fmBitmaps,
  HashType t
)
{
  Tools::Random r;
  std::stringstream ss;
  ss.write(reinterpret_cast<const char*>(&id), sizeof(uint64_t));
  initialize(ss.str(), m, counters, hashes, fmSize, fmBitmaps, t, r);
}

Sketches::CountMinFM::~CountMinFM()
{}

uint64_t Sketches::CountMinFM::getFrequency(const std::string& id) const
{
  if (m_type == HT_UNIVERSAL)
  {
    uint64_t min = std::numeric_limits<uint64_t>::max();
    uint64_t l = atoll(id.c_str());

    for (size_t i = 0; i < m_hashes; i++)
    {
      size_t h = m_hash[i].hash(l) % m_counters;
      uint64_t v = m_filter[i * m_counters + h].getCount();
      if (v < min) min = v;
    }

    return min;
  }
  else if (m_type == HT_SHA1)
  {
    uint64_t min = std::numeric_limits<uint64_t>::max();
    Tools::SHA1Hash sha;
    size_t len;
    byte* data;
    sha.hash(id, &data, len);

    for (size_t i = 0; i < m_hashes; i++)
    {
      size_t h =
        *(reinterpret_cast<uint16_t*>(data + 2 * i)) % m_counters;
      uint64_t v = m_filter[i * m_counters + h].getCount();
      if (v < min) min = v;
    }

    return min;
  }
  else
  {
    throw Tools::NotSupportedException("CountMinFM: Unknown hash type.");
  }
}

void Sketches::CountMinFM::merge(const Sketches::CountMinFM& in)
{
  if (m_counters != in.m_counters || m_hashes != in.m_hashes)
    throw Tools::IllegalArgumentException(
      "CountMinFM: Sketches should have the same "
      "size and number of hashes."
    );

  if (m_type != in.m_type)
    throw Tools::IllegalArgumentException(
      "CountMinFM: Sketches should have the same type."
    );

  for (size_t i = 0; i < m_counters * m_hashes; i++)
    m_filter[i].merge(in.m_filter[i]);
}

size_t Sketches::CountMinFM::getVectorLength() const
{
  return m_counters;
}

size_t Sketches::CountMinFM::getNumberOfHashes() const
{
  return m_hashes;
}

size_t Sketches::CountMinFM::getSize() const
{
  size_t ret =
    sizeof(HashType) +
    3 * sizeof(size_t) +
    m_id.size();

  for (size_t i = 0; i < m_hash.size(); i++)
    ret += m_hash[i].getSize();

  for (size_t i = 0; i < m_filter.size(); i++)
    ret += m_filter[i].getSize();

  return ret;
}

void Sketches::CountMinFM::marshal(boost::archive::text_oarchive *x) const
{
  int32_t sl = m_id.length();
  const char *st = m_id.c_str(); 
  
  *x & sl;
  x->save_binary(const_cast<char *>(st), sl + 1);

  *x & m_counters;
  *x & m_hashes;

  int32_t numFilters = m_filter.size();
  
  *x & numFilters;

  for ( std::vector<FM>::const_iterator vectIter = m_filter.begin(); 
	vectIter != m_filter.end();
	vectIter++)
  {
    vectIter->marshal(x);
  }

  int32_t numHashes;

  *x & numHashes;

  for (std::vector<Tools::UniversalHash>::const_iterator vectIter = m_hash.begin();
       vectIter != m_hash.end(); vectIter++)
  {
      vectIter->marshal(x);
  }
}

Sketches::CountMinFM *Sketches::CountMinFM::unmarshal(
    boost::archive::text_iarchive *x)
{
  int32_t sl;
  *x & sl;
  // Now fetch the string itself
  static const int STATIC_STRING_BUFFER = 10000;

  std::string *sketchId;

  if (sl + 1 <= STATIC_STRING_BUFFER) {
    // We can use the static buffer
    static char stringBuffer[STATIC_STRING_BUFFER];
    static char* sb = &(stringBuffer[0]);
    x->load_binary(sb, sl + 1);
    sb[sl] = 0;       // make sure it's null terminated
    sketchId = new std::string(sb, sl);
  }  else {
    // We can't use the static buffer. We must allocate a one-shot
    // buffer
    char * localBuffer = new char[sl + 1];
    x->load_binary(localBuffer, sl);
    localBuffer[sl] = 0;
    sketchId = new std::string(localBuffer, sl);
    delete localBuffer;
  }

  size_t counters;
  size_t hashes;

  *x & counters;
  *x & hashes;

  std::map<std::string, uint64_t> emptyMap;

  Sketches::CountMinFM *sketch = new Sketches::CountMinFM(
      *sketchId, emptyMap, counters, hashes, 32, 64, HT_SHA1);
  
  int32_t numFilters;
  
  *x & numFilters;

  for(int i = 0; i < numFilters; i++) {
    Sketches::FM *fmPointer = Sketches::FM::unmarshal(x);

    sketch->m_filter[i] = *fmPointer;
  }

  int32_t numHashes;

  *x & numHashes;

  for(int i = 0; i < numHashes; i++) {
    Tools::UniversalHash *hashPtr = Tools::UniversalHash::unmarshal(x);

    sketch->m_hash[i] = *hashPtr;
  }
  
  return sketch;
}

void Sketches::CountMinFM::initialize(
  const std::string& id,
  const std::map<std::string, uint64_t>& m,
  size_t counters,
  size_t hashes,
  size_t fmSize,
  size_t fmBitmaps,
  HashType type,
  Tools::Random& r
)
{
  m_id = id;
  m_counters = counters;
  m_hashes = hashes;
  m_type = type;

  if (m_hashes == 0)
    throw Tools::IllegalArgumentException(
      "CountMinFM: number of hashes must be larger than zero."
    );

  if (m_counters == 0)
    throw Tools::IllegalArgumentException(
      "CountMinFM: vector size must be larger than zero."
    );

  for (size_t i = 0; i < m_counters * m_hashes; i++)
    m_filter.push_back(Sketches::FM(fmSize, fmBitmaps, m_type));

  if (m_type == HT_UNIVERSAL)
  {
    for (size_t i = 0; i < m_hashes; i++)
      m_hash.push_back(Tools::UniversalHash(r));

    std::map<std::string, uint64_t>::const_iterator it;

    for (it = m.begin(); it != m.end(); it++)
    {
      uint64_t l = atoll((*it).first.c_str());

      for (size_t i = 0; i < m_hashes; i++)
      {
        size_t h = m_hash[i].hash(l) % m_counters;
        m_filter[i * m_counters + h].insert(m_id, (*it).second);
      }
    }
  }
  else if (m_type == HT_SHA1)
  {
    if (m_counters >= std::pow(2.0, 16.0))
      throw Tools::IllegalArgumentException(
        "CountMinFM: to use the SHA1 hash, vector "
        "size must be in [1, 2^16 - 1]."
      );

    if (m_hashes > 10)
      throw Tools::IllegalArgumentException(
        "CountMinFM: to use the SHA1 hash, "
        "number of hashes must be in [1, 10]."
      );

    std::map<std::string, uint64_t>::const_iterator it;

    for (it = m.begin(); it != m.end(); it++)
    {
      Tools::SHA1Hash sha;
      size_t len;
      byte* data;
      sha.hash(id, &data, len);

      for (size_t i = 0; i < m_hashes; i++)
      {
        size_t h =
          *(reinterpret_cast<uint16_t*>(data + 2 * i)) % m_counters;
        m_filter[i * m_counters + h].insert(m_id, (*it).second);
      }
      delete[] data;
    }
  }
}
