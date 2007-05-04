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

Sketches::FM::FM(
  size_t bits,
  size_t bitmaps,
  Sketches::HashType t
) : m_type(t), m_seed(0x330E)
{
  if (bitmaps < 1 || bitmaps > 256)
    throw Tools::IllegalArgumentException(
      "FM: Number of bitmaps must be in [1, 256]"
    );

  if (m_type == HT_SHA1)
  {
    if (bits < 1 || bits > (Tools::SHA1Hash::HashLength - 1) * CHAR_BIT)
    {
      std::ostringstream ss;
      ss <<
      "FM: Bitmap length must be in [1, " <<
      (Tools::SHA1Hash::HashLength - 1) * CHAR_BIT <<
      "].";
      throw Tools::IllegalArgumentException(ss.str());
    }
  }
  else if (m_type == HT_UNIVERSAL)
  {
    if (bits < 1 || bits > 256)
      throw Tools::IllegalArgumentException(
        "FM: Bitmap length must be in [1, 256]."
      );
  }
  else
  {
    throw Tools::NotSupportedException(
      "FM: This hash type is not supported yet."
    );
  }

  for (size_t i = 0; i < bitmaps; i++)
    m_bitmap.push_back(std::vector<bool>(bits, false));
}

Sketches::FM::FM(
  size_t bits,
  size_t bitmaps,
  uint16_t seed
) : m_type(HT_UNIVERSAL), m_seed(seed)
{
  if (bitmaps < 1 || bitmaps > 256)
    throw Tools::IllegalArgumentException(
      "FM: Number of bitmaps must be in [1, 256]"
    );

  if (bits < 1 || bits > 256)
    throw Tools::IllegalArgumentException(
      "FM: Bitmap length must be in [1, 256]."
    );

  for (size_t i = 0; i < bitmaps; i++)
    m_bitmap.push_back(std::vector<bool>(bits, false));
}

Sketches::FM::FM(const byte* data)
{
  reset(data);
}

Sketches::FM::~FM()
{}

void Sketches::FM::insert(const std::string& id, uint64_t val)
{
  if (m_type == HT_UNIVERSAL)
  {
    uint64_t l = atoll(id.c_str());
    insert(l);
  }
  else if (m_type == HT_SHA1)
  {
    if (val == 0) val = 1;

    for (uint32_t i = 0; i < val; i++)
    {
      std::ostringstream ss;
      ss << i << "-" << id;

      Tools::SHA1Hash sha;
      size_t len;
      byte* data;
      sha.hash(ss.str(), &data, len);

      size_t b =
        static_cast<size_t>(data[0]) % m_bitmap.size();

      size_t i = 1, offset = 0;

      /*
      while (data[i] == 0) { i++; offset += CHAR_BIT; }

      if (offset >= m_bitmap[b].size())
      {
      	m_bitmap[b][m_bitmap[b].size() - 1] = true;
      }
      else
      {
      	byte mask = 1;
      	while ((data[i] & mask) == 0) { mask <<= 1; offset++; }
      		// this should yield a non-zero bit.
      	m_bitmap[b][offset] = true;
      }
      */

      // optimize for the most common case, where the
      // first byte is not expected to be zero.
      byte mask = 1;

      while ((data[i] & mask) == 0)
      {
        mask <<= 1;
        offset++;

        if (mask == 0)
        {
          if (i == len - 1) break;
          // prevent buffer overrun.
          mask = 1;
          i++;
        }
      }

      if (offset >= m_bitmap[b].size())
        m_bitmap[b][m_bitmap[b].size() - 1] = true;
      else
        m_bitmap[b][offset] = true;
    }
  }
  else
  {
    throw Tools::NotSupportedException(
      "FM: This hash type is not supported yet."
    );
  }
}

void Sketches::FM::insert(
  const Tools::UniversalHash::value_type& id,
  uint64_t val
)
{
  if (m_type == HT_UNIVERSAL)
  {
    if (val == 0) val = 1;

    Tools::Random r(id, m_seed);
    //Tools::Random r(id);
    // The Mersenne generator has huge initialization
    // overhead so, for efficiency, it cannot be used here.
    // On the other hand, drand48 produces low order bits
    // with very small periodicity, so it requires a larger
    // sample of values in order to produce equivalent
    // results.

    for (; val % m_bitmap.size() != 0; --val)
    {
      byte b = r.nextUniformLong(0, m_bitmap.size());
      uint32_t offset = pickOffset(r);
      m_bitmap[b][offset] = true;
    }

    uint32_t bulkAddSize = val / m_bitmap.size();

    if (bulkAddSize > 0)
    {
      uint32_t fl = fillLength(bulkAddSize);

      if (fl > 0)
      {
        for (size_t b = 0; b < m_bitmap.size(); b++)
          for (uint32_t j = 0; j < fl; j++)
            m_bitmap[b][j] = true;

        uint32_t remaining =
          pickBinomial(
            bulkAddSize,
            std::pow(0.5, static_cast<double>(fl)),
            r
          );

        for (uint32_t j = 0; j < remaining; j++)
        {
          for (size_t b = 0; b < m_bitmap.size(); b++)
          {
            uint32_t offset = pickOffset(r);
            m_bitmap[b][offset] = true;
          }
        }
      }
      else
      {
        for (uint32_t j = 0; j < bulkAddSize; j++)
        {
          size_t b = r.nextUniformLong(0L, m_bitmap.size());
          uint32_t offset = pickOffset(r);
          m_bitmap[b][offset] = true;
        }
      }
    }
  }
  else if (m_type == HT_SHA1)
  {
    std::ostringstream ss;
    ss << id << std::flush;
    insert(ss.str());
  }
  else
  {
    throw Tools::NotSupportedException(
      "FM: This hash type is not supported yet."
    );
  }
}

void Sketches::FM::erase(const std::string& id, uint64_t val)
{
  throw Tools::NotSupportedException(
    "FM: FM sketches do not support deletions."
  );
}

void Sketches::FM::erase(
  const Tools::UniversalHash::value_type& id,
  uint64_t val
)
{
  throw Tools::NotSupportedException(
    "FM: FM sketches do not support deletions."
  );
}

void Sketches::FM::clear()
{
  size_t bitmaps = m_bitmap.size();
  size_t bits = m_bitmap[0].size();
  m_bitmap.clear();

  for (size_t i = 0; i < bitmaps; i++)
    m_bitmap.push_back(std::vector<bool>(bits, false));
}

uint64_t Sketches::FM::getCount() const
{
  size_t R, S = 0;

  for (size_t b = 0; b < m_bitmap.size(); b++)
  {
    R = 0;
    while (R < m_bitmap[b].size() && m_bitmap[b][R] == true) R++;
    S += R ;
  }

  // we need to scale with m_bitmap.size() here,
  // since during the insertion we only insert each element
  // in one bitmap only (a la Flajolet pseudocode, page 16),
  // instead of in all bitmaps (this is much faster).
  return static_cast<uint64_t>(std::ceil((m_bitmap.size() / PHI) * std::pow(2.0, static_cast<double>(S) / static_cast<double>(m_bitmap.size()))));
}

void Sketches::FM::merge(const Sketches::FM& in)
{
  if (
    m_type != in.m_type ||
    m_seed != in.m_seed ||
    m_bitmap.size() != in.m_bitmap.size() ||
    m_bitmap[0].size() != in.m_bitmap[0].size()
  )
    throw Tools::IllegalArgumentException(
      "FM: FM sketches do not have compatible sizes or types."
    );

  for (size_t i = 0; i < m_bitmap.size(); i++)
    for (size_t j = 0; j < m_bitmap[i].size(); j++)
      if (in.m_bitmap[i][j]) m_bitmap[i][j] = true;
}

void Sketches::FM::merge(const Sketches::FM& f1, const Sketches::FM& f2)
{
  if (
    f1.m_type != f2.m_type ||
    f1.m_seed != f2.m_seed ||
    f1.m_bitmap.size() != f2.m_bitmap.size() ||
    f1.m_bitmap[0].size() != f2.m_bitmap[0].size()
  )
    throw Tools::IllegalArgumentException(
      "FM: FM sketches do not have compatible sizes or types."
    );

  (*this) = f1;
  merge(f2);
}

Sketches::FM Sketches::FM::getMerged(const Sketches::FM& in) const
{
  FM ret(*this);
  ret.merge(in);
  return ret;
}

void Sketches::FM::reset(const byte* data)
{
  // Seed.
  memcpy(&m_seed, data, sizeof(uint16_t));
  data += sizeof(uint16_t);

  byte l;
  // FM type.
  memcpy(&l, data, sizeof(byte));
  data += sizeof(byte);
  m_type = static_cast<HashType>(l);

  // number of bitmaps.
  memcpy(&l, data, sizeof(byte));
  data += sizeof(byte);
  size_t bitmaps = static_cast<size_t>(l) + 1;

  // number of bits per bitmap.
  memcpy(&l, data, sizeof(byte));
  data += sizeof(byte);
  size_t bits = static_cast<size_t>(l) + 1;

  if (bits < 1 || bitmaps < 1)
    throw Tools::IllegalArgumentException(
      "FM: Bitmap length and number of "
      "bitmaps must be positive integers."
    );

  for (size_t i = 0; i < bitmaps; i++)
    m_bitmap.push_back(std::vector<bool>(bits, false));

  // number of bits in prefix.
  memcpy(&l, data, sizeof(byte));
  data += sizeof(byte);
  size_t prefix = static_cast<size_t>(l);

  for (size_t i = 0; i < bitmaps; i++)
  {
    for (size_t j = 0; j < prefix; j++) m_bitmap[i][j] = true;

    // number of significant remaining bytes.
    memcpy(&l, data, sizeof(byte));
    data += sizeof(byte);

    size_t cl = prefix;

    for (size_t j = 0; j < static_cast<size_t>(l); j++)
    {
      byte mask = 1;

      for (size_t m = 0; m < CHAR_BIT; m++)
      {
        if ((*data) & mask) m_bitmap[i][cl] = true;
        else m_bitmap[i][cl] = false;
        cl++;
        if (cl >= bits) break;
        mask <<= 1;
      }
      data += sizeof(byte);
    }
    for (size_t j = cl; j < bits; j++) m_bitmap[i][j] = false;
  }
  //assert(pC == data + length);
}

bool Sketches::FM::isSubsumedBy(const Sketches::FM& in) const
{
  if (
    m_type != in.m_type ||
    m_seed != in.m_seed ||
    m_bitmap.size() != in.m_bitmap.size() ||
    m_bitmap[0].size() != in.m_bitmap[0].size()
  )
    throw Tools::IllegalArgumentException(
      "FM: FM sketches do not have compatible sizes or types."
    );

  for (size_t i = 0; i < m_bitmap.size(); i++)
    for (size_t j = 0; j < m_bitmap[i].size(); j++)
      if (m_bitmap[i][j] == true && in.m_bitmap[i][j] == false)
        return false;

  return true;
}

size_t Sketches::FM::getUncompressedSize() const
{
  return static_cast<size_t>(std::ceil(static_cast<double>(m_bitmap.size() * m_bitmap[0].size()) / CHAR_BIT));
}

void Sketches::FM::marshal(XDR *x) const
{
  u_int myBits = (u_int) m_bitmap.size();
  u_int myBitmaps = (u_int) (m_bitmap.begin())->size();
  int type = (int) m_type;

  xdr_u_int(x, &myBits);
  xdr_u_int(x, &myBitmaps);
  xdr_int(x, &type);

  for ( size_t i = 0; i < m_bitmap.size(); i ++ )
  {
    for ( size_t j = 0; j < (m_bitmap.begin())->size(); j++ )
    {
      xdr_bool(x, (bool_t *) m_bitmap[i][j]);
    }
  }
}

Sketches::FM *Sketches::FM::unmarshal(XDR *x)
{
  u_int bits;
  u_int bitmaps;
  int type;

  xdr_u_int(x, &bits);
  xdr_u_int(x, &bitmaps);
  xdr_int(x, &type);

  Sketches::FM *sketch = new Sketches::FM((u_int) bits, (u_int) bitmaps,
                                          (HashType) type);

  for (u_int i = 0; i < bitmaps; i++)
  {
    for (u_int j = 0; j < bits; j++)
    {
      int tmpJ;
      xdr_bool(x, &tmpJ);

      sketch->m_bitmap[i][j] = tmpJ;
    }
  }
}

size_t Sketches::FM::getSize() const
{
  size_t l;
  byte* data;
  getData(&data, l);
  delete[] data;
  return l;
}

void Sketches::FM::getData(byte** buffer, size_t& length) const
{
  // I will use a stringstream here to store the
  // bytes, since I do not know the final data
  // length to begin with. Notice I can only
  // insert byte values in the stringstream for
  // this to work correctly.

  size_t prefix = m_bitmap[0].size();
  // this points to the first zero.

  for (size_t i = 0; i < m_bitmap.size(); i++)
  {
    size_t j = 0;
    while (j < m_bitmap[i].size() && m_bitmap[i][j] == true) j++;
    if (prefix > j) prefix = j;
  }

  // I will assume here that bitmaps are not
  // larger than 255 bits and that we do not
  // have more than 255 bitmaps.

  assert(m_bitmap.size() <= 256);
  assert(m_bitmap[0].size() <= 256);

  std::ostringstream ret;
  ret
  << static_cast<byte>(m_type)
  << static_cast<byte>(m_bitmap.size() - 1)
  << static_cast<byte>(m_bitmap[0].size() - 1)
  << static_cast<byte>(prefix);

  for (size_t i = 0; i < m_bitmap.size(); i++)
  {
    std::vector<byte> v;
    byte buf = 0, mask = 1;
    size_t postfix = 0;
    // this points to the first of the last zeroes.

    for (size_t j = prefix; j < m_bitmap[i].size(); j++)
    {
      if (m_bitmap[i][j] == true)
      {
        buf = buf | mask;
        postfix = j + 1 - prefix;
      }
      mask <<= 1;

      if (mask == 0)
      {
        v.push_back(buf);
        buf = 0;
        mask = 1;
      }
    }
    if (mask != 1) v.push_back(buf);

    size_t postfixByte =
      ((postfix % CHAR_BIT) == 0) ?
      postfix / CHAR_BIT : (postfix / CHAR_BIT) + 1;

    ret << static_cast<byte>(postfixByte);
    for (size_t j = 0; j < postfixByte; j++) ret << v[j];
  }

  ret.flush();
  std::string str = ret.str();
  length = str.size() + sizeof(uint16_t);
  *buffer = new byte[length];
  memcpy(*buffer, &m_seed, sizeof(uint16_t));
  memcpy(
    (*buffer) + sizeof(uint16_t),
    str.c_str(),
    length - sizeof(uint16_t)
  );
}

size_t Sketches::FM::getVectorLength() const
{
  return m_bitmap[0].size();
}

size_t Sketches::FM::getNumberOfVectors() const
{
  return m_bitmap.size();
}

uint16_t Sketches::FM::getSeed() const
{
  return m_seed;
}

uint32_t Sketches::FM::pickOffset(Tools::Random& r) const
{
  // this approach needs too many random numbers.
  // Also, it utilizes the low order bits of
  // drand48 which introduces a lot of bias
  // and messes up the accuracy of the filter
  // (see comments below).
  /*
  uint32_t x = 0;
  while(r.flipCoin()) x++;
  if (x >= m_bitmap[0].size()) x = m_bitmap[0].size() - 1;
  return x;
  */

  // this approach is sexy, but it is not
  // optimized for the most common case,
  // where x is expected not to be zero.
  // FIXME: this approach might introduce bias
  // if the low order bits produced by the PRG
  // are biased, as is the case with drand48.
  // See below on how to fix this
  // (you should try this! It really messes up
  // the results significantly!!)
  /*
  uint32_t offset = 0;
  uint32_t x = static_cast<uint32_t>(r.nextUniformLong());

  while (x == 0)
  {
  	offset += sizeof(uint32_t) * CHAR_BIT;
  	x = static_cast<uint32_t>(r.nextUniformLong());

  	// I could terminate the loop here early
  	// if offset becomes larger than needed,
  	// by why bother checking since this is
  	// expected to happen very rarely.
  }

  if (offset >= m_bitmap[0].size())
  {
  	return m_bitmap[0].size() - 1;
  }
  else
  {
  	uint32_t mask = 1;
  	while ((x & mask) == 0) { mask <<= 1; offset++; }
  		// this has to yield a non-zero bit before mask overflows.
  	assert(offset < m_bitmap[0].size());
  	return offset;
  }
  */

  // optimize the most common case, where the first
  // random bits are not expected to be all zero.
  // NOTICE: this approach might introduce bias
  // if the low order bits produced by the PRG
  // are biased, as is the case with drand48.
  // So, we sample the PRG twice per iteration
  // and use only the high order bits.
  //assert(sizeof(uint32_t) % 2 == 0);
  uint32_t mask = 1;
  uint32_t offset = 0;
  uint32_t hm = (~0) << 16;
  uint32_t lm = ~hm;

  uint32_t x =
    ((static_cast<uint32_t>(r.nextUniformLong()) >> 16) & lm) |
    (static_cast<uint32_t>(r.nextUniformLong()) & hm);

  while ((x & mask) == 0)
  {
    mask <<= 1;
    offset++;

    if (mask == 0)
    {
      x = ((static_cast<uint32_t>(r.nextUniformLong()) >> 16) & lm) | (static_cast<uint32_t>(r.nextUniformLong()) & hm);
      mask = 1;
    }
  }

  if (offset >= m_bitmap[0].size())
    return m_bitmap[0].size() - 1;
  else
    return offset;
}

uint32_t Sketches::FM::fillLength(uint32_t val) const
{
  if (val <= 10) return 0;

  double value = static_cast<double>(val);
  double logValue = std::log(value);

  return static_cast<uint32_t>(std::floor(std::log(value / (logValue * logValue)) / std::log(2.0)));
}

int32_t Sketches::FM::pickBinomial(
  int32_t nIn, double pIn, Tools::Random& rand
) const
{
  double q = 1.0 - pIn;
  double s = pIn / q;
  double a = static_cast<double>(nIn + 1L) * s;
  double r = std::pow(q, static_cast<double>(nIn));
  double u = rand.nextUniformDouble();
  int32_t x = 0;

  while (u > r)
  {
    u -= r;
    ++x;
    r *= (a / x) - s;
  }

  return x;
}

std::ostream& Sketches::operator<<(std::ostream& os, const Sketches::FM& s)
{
  if (s.m_type == HT_UNIVERSAL)
    os << s.m_seed << " ";

  os
  << s.m_type << " "
  << s.m_bitmap.size() << " "
  << s.m_bitmap[0].size();

  for (size_t i = 0; i < s.m_bitmap.size(); i++)
  {
    os << " ";

    for (size_t j = 0; j < s.m_bitmap[i].size(); j++)
      os << s.m_bitmap[i][j];
  }

  return os;
}

