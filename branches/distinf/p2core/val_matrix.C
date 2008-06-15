/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "val_null.h"
#include "val_matrix.h"
#include "val_int64.h"
#include "val_list.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

class OperMatrix : public opr::OperCompare<Val_Matrix> {
};

const opr::Oper* Val_Matrix::oper_ = new OperMatrix();

ValuePtr Val_Matrix::transpose() {

  uint32_t sz1 = size1();
  uint32_t sz2 = size2();
  
  MatrixPtr mp(new ublas::matrix<ValuePtr>(sz2, sz1));
  (*mp) = trans((*M));
  return Val_Matrix::mk(mp);
}

MatrixPtr Val_Matrix::cast(ValuePtr v)
{
   
  switch(v->typeCode()) {
  case Value::MATRIX:
    {
      return (static_cast<Val_Matrix *>(v.get()))->M;         
    }
  case Value::LIST:
    {
      ListPtr entries = Val_List::cast(v);
      assert(entries->size()!=0);
      size_t m = entries->size();
      size_t n = Val_List::cast(entries->front())->size();
      
      MatrixPtr mp(new ValPtrMatrix(m, n));
      for(size_t i = 0; i<m; i++) {
	ListPtr row = Val_List::cast(entries->at(i));
	assert(row->size() == n);
	for(size_t j = 0; j<n; j++) (*mp)(i, j) = row->at(j);
      }

      return mp;
    }
  default:
    {
      throw Value::TypeError(v->typeCode(),
			     v->typeName(),
			     Value::MATRIX,
			     "matrix");      
    }
  }
}


doubleMatrix Val_Matrix::cast_double(ValuePtr v)
{
  switch(v->typeCode()) {
  case Value::MATRIX:
    {
      doubleMatrix result(size1(), size2());
      std::transform(M->data().begin(), M->data().end(), result().begin(),
                     Val_Double::cast_t());
      return result;
    }

  case Value::STR:
    {
      typedef boost::tokenizer< boost::char_separator<char> > tokenizer;

      std::string str(v->toString());
      doubleMatrix m;
      bool first_row = true;
      size_t nrows = std::count(str.begin(), str.end(), ";") + 1;
      size_t ncols = 0;
      size_t i = 0;
      tokenizer rows(str, boost::char_separator<char>(";"));

      for(tokenizer::iterator it = rows.begin(), it != rows.end(); ++it) {
        assert(i < nrows);
        tokenizer items(*it, boost::char_separator<char>("_"));
        if (first_row) { // count the number of columns
          ncols = std::distance(items.begin(), items.end());
          m.resize(nrows, ncols);
          first_row = false;
        }
        // parse the entries of i-th row
        for(tokenizer::iterator jt = items.begin(), jt != items.end(); ++jt) {
          assert(j < ncols);
          m(i, j++) = boost::lexical_cast<double>(*jt);
        }
        i++;
      }

      return m;
    }
      
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::MATRIX,
                           "matrix");      
  }
}


// Marshal/Unmarshal essentially copied from Val_Tuple
void Val_Matrix::xdr_marshal_subtype( XDR *x )
{
  ValPtrMatrix::const_iterator1 it1;
  ValPtrMatrix::const_iterator2 it2;
  
  uint32_t sz1 = M->size1();
  u_int32_t i1 = (u_int32_t)sz1;
  uint32_t sz2 = M->size2();
  u_int32_t i2 = (u_int32_t)sz2;
  xdr_uint32_t(x, &i1);
  xdr_uint32_t(x, &i2);
  // Marshal the entries -- we will use default storage (row-major)
  for (it1 = M->begin1();  it1 != M->end1(); ++it1)
    { 
      for (it2 = it1.begin(); it2 != it1.end(); ++it2)
        (*it2)->xdr_marshal(x); 
    }
}

ValuePtr Val_Matrix::xdr_unmarshal( XDR *x )
{
  u_int32_t ui1, ui2;
  xdr_uint32_t(x, &ui1);
  xdr_uint32_t(x, &ui2);
  uint32_t sz1 = ui1;
  uint32_t sz2 = ui2;

  MatrixPtr mp(new ublas::matrix<ValuePtr>(sz1, sz2));
  // Unmarshal the entries -- we use default (row-major) storage
  for (uint32_t i1 = 0; i1 < sz1; i1++) {
    for (uint32_t i2 = 0; i2 < sz2; i2++) {
      (*mp)(i1,i2) = Value::xdr_unmarshal(x);
    }
  }
  return mk(mp);
}

string Val_Matrix::toString() const
{
  ostringstream sb;
   
  sb << "[";
   
  ValPtrMatrix::const_iterator1 iter1 = M->begin1();
  ValPtrMatrix::const_iterator1 end1 = M->end1();
  ValPtrMatrix::const_iterator1 almost_end1 = M->end1();
  almost_end1--;
   
  while(iter1 != end1) {
    sb << "[";
    ValPtrMatrix::const_iterator2 iter2 = iter1.begin();
    ValPtrMatrix::const_iterator2 end2 = iter1.end();
    ValPtrMatrix::const_iterator2 almost_end2 = iter1.end();
    almost_end2--;

    while (iter2 != end2) {
      sb << (*iter2)->toString();
      
      if(iter2 != almost_end2) {
        sb << ", ";
      }
      iter2++;
    }
   
    sb << "]";

    if (iter1 != almost_end1) {
      sb << ", ";
    }
    iter1++;
  }
	
  sb << "]";
  return sb.str();
}

string Val_Matrix::toConfString() const
{
  ostringstream sb;
   
  sb << "{";
   
  ValPtrMatrix::const_iterator1 iter1 = M->begin1();
  ValPtrMatrix::const_iterator1 end1 = M->end1();
  ValPtrMatrix::const_iterator1 almost_end1 = M->end1();
  almost_end1--;
   
  while(iter1 != end1) {
    sb << "[";
    ValPtrMatrix::const_iterator2 iter2 = iter1.begin();
    ValPtrMatrix::const_iterator2 end2 = iter1.end();
    ValPtrMatrix::const_iterator2 almost_end2 = iter1.end();
    almost_end2--;

    while (iter2 != end2) {
      sb << (*iter2)->toConfString();
      
      if(iter2 != almost_end2) {
        sb << ", ";
      }
      iter2++;
    }
   
    sb << "]";

    if (iter1 != almost_end1) {
      sb << "%";
    }
    iter1++;
  }
	
  sb << "}";
  return sb.str();
}

// Matrix comparison. Follow rules from Tuple
int Val_Matrix::compareTo(ValuePtr other) const
{
  if(other->typeCode() < Value::MATRIX) {
    return -1;
  } 
  else if(other->typeCode() > Value::MATRIX) {
    return 1;
  } 
  else {
    Val_Matrix om = cast(other);
    if (size1() == om.M->size1() && size2() == om.M->size2()) {
      // same size
      ValPtrMatrix::const_iterator1 myiter1, oiter1;
      ValPtrMatrix::const_iterator2 myiter2, oiter2;
      for (myiter1 = M->begin1(), oiter1 = om.M->begin1();
           myiter1 != M->end1(); myiter1++, oiter1++) {
        for (myiter2 = myiter1.begin(), oiter2 = oiter1.begin();
             myiter2 != myiter1.end(); myiter2++, oiter2++) {
          int result = (*myiter2)->compareTo(*oiter2);
          if (result != 0) {
            // Found a field position for which we are different.  Return
            // the difference.
            return result;
          }
        }
      }
      // All fields are equal
      return(0);
    }
    else if (size1() == om.M->size1()) { // tie on size1
      if (size2() < om.M->size2()) {
        return -1;
      } 
      else { // size2() > om.M->size2()
        return 1;
      }
    }
    else if (size1() < om.M->size1()) {
      return -1;
    }
    else { // size1() > om.M->size1()
      return 1;
    }
  }
}
