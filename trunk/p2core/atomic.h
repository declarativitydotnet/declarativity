// -*- c-basic-offset: 2 -*-
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

class atomic_uint32_t { public:
  
    // No constructors because, unfortunately, GCC generates worse code. Use
    // operator= instead.
  
    operator uint32_t() const		{ return _val; }
    uint32_t value() const		{ return _val; }

    atomic_uint32_t &operator=(uint32_t u) { _val = u; return *this; }
  
    atomic_uint32_t &operator+=(int x)	{ _val += x; return *this; }
    atomic_uint32_t &operator-=(int x)	{ _val -= x; return *this; }
    atomic_uint32_t &operator&=(uint32_t u) { _val &= u; return *this; }
    atomic_uint32_t &operator|=(uint32_t u) { _val |= u; return *this; }
  
    void operator++(int)		{ _val++; }
    void operator--(int)		{ _val--; }

    // returns true if value is 0 after decrement
    bool dec_and_test()			{ _val--; return _val == 0; }

    inline uint32_t read_and_add(uint32_t delta);
    inline uint32_t compare_and_swap(uint32_t test_value, uint32_t new_value);

  private:

    volatile uint32_t _val;
  
};

inline uint32_t
atomic_uint32_t::read_and_add(uint32_t delta)
{
    uint32_t old_value = _val;
    _val += delta;
    return old_value;
}

inline uint32_t
atomic_uint32_t::compare_and_swap(uint32_t test_value, uint32_t new_value)
{
    uint32_t old_value = _val;
    if (_val == test_value)
	_val = new_value;
    return old_value;
}

inline uint32_t
operator+(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() + b.value();
}

inline uint32_t
operator-(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() - b.value();
}

inline bool
operator==(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() == b.value();
}

inline bool
operator!=(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() != b.value();
}

inline bool
operator>(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() > b.value();
}

inline bool
operator<(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() < b.value();
}

inline bool
operator>=(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() >= b.value();
}

inline bool
operator<=(const atomic_uint32_t &a, const atomic_uint32_t &b)
{
    return a.value() <= b.value();
}

typedef atomic_uint32_t uatomic32_t;

#endif
