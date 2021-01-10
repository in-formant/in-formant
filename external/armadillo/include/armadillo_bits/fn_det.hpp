// Copyright 2008-2016 Conrad Sanderson (http://conradsanderson.id.au)
// Copyright 2008-2016 National ICT Australia (NICTA)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ------------------------------------------------------------------------


//! \addtogroup fn_det
//! @{



template<typename T1>
arma_warn_unused
inline
typename enable_if2< is_supported_blas_type<typename T1::elem_type>::value, typename T1::elem_type >::result
det(const Base<typename T1::elem_type,T1>& X)
  {
  arma_extra_debug_sigprint();
  
  const quasi_unwrap<T1> U(X.get_ref());
  
  arma_debug_check( (U.M.is_square() == false), "det(): given matrix must be square sized" );
  
  if( (U.M.n_rows > 4) && (U.M.is_diagmat()) )
    {
    return det(diagmat(U.M));  // call the specialised version of det()
    }
  
  return auxlib::det(U.M);
  }



template<typename T1>
arma_warn_unused
inline
typename T1::elem_type
det(const Op<T1, op_diagmat>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const diagmat_proxy<T1> A(X.m);
  
  arma_debug_check( (A.n_rows != A.n_cols), "det(): given matrix must be square sized" );
  
  const uword N = (std::min)(A.n_rows, A.n_cols);
  
  eT val1 = eT(1);
  eT val2 = eT(1);
  
  uword i,j;
  for(i=0, j=1; j<N; i+=2, j+=2)
    {
    val1 *= A[i];
    val2 *= A[j];
    }
  
  
  if(i < N)
    {
    val1 *= A[i];
    }
  
  return val1 * val2;
  }



template<typename T1>
arma_warn_unused
inline
typename T1::elem_type
det(const Op<T1, op_trimat>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const Proxy<T1> P(X.m);
  
  const uword N = P.get_n_rows();
  
  arma_debug_check( (N != P.get_n_cols()), "det(): given matrix must be square sized" );
  
  eT val1 = eT(1);
  eT val2 = eT(1);
  
  uword i,j;
  for(i=0, j=1; j<N; i+=2, j+=2)
    {
    val1 *= P.at(i,i);
    val2 *= P.at(j,j);
    }
  
  if(i < N)
    {
    val1 *= P.at(i,i);
    }
  
  return val1 * val2;
  }



template<typename T>
arma_warn_unused
arma_inline
typename arma_scalar_only<T>::result
det(const T& x)
  {
  return x;
  }



//! @}
