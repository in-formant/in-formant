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


//! \addtogroup op_inv
//! @{



template<typename T1>
inline
void
op_inv::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_inv>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const strip_diagmat<T1> strip(X.m);
  
  bool status = false;
  
  if(strip.do_diagmat)
    {
    status = op_inv::apply_diagmat(out, strip.M);
    }
  else
    {
    const quasi_unwrap<T1> U(X.m);
    
    if(U.is_alias(out))
      {
      Mat<eT> tmp;
      
      status = op_inv::apply_noalias(tmp, U.M);
      
      out.steal_mem(tmp);
      }
    else
      {
      status = op_inv::apply_noalias(out, U.M);
      }
    }
  
  if(status == false)
    {
    out.soft_reset();
    arma_stop_runtime_error("inv(): matrix seems singular");
    }
  }



template<typename eT>
inline
bool
op_inv::apply_noalias(Mat<eT>& out, const Mat<eT>& A)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (A.n_rows != A.n_cols), "inv(): given matrix must be square sized" );
  
  bool status = false;
  
  if(A.n_rows <= 4)
    {
    status = auxlib::inv_tiny(out, A);
    }
  else
  if(A.is_diagmat())
    {
    return op_inv::apply_diagmat(out, A);
    }
  else
    {
    const bool is_triu =                     trimat_helper::is_triu(A);
    const bool is_tril = (is_triu) ? false : trimat_helper::is_tril(A);
    
    if(is_triu || is_tril)
      {
      const uword layout = (is_triu) ? uword(0) : uword(1);
      
      return auxlib::inv_tr(out, A, layout);
      }
    else
      {
      #if defined(ARMA_OPTIMISE_SYMPD)
        const bool try_sympd = sympd_helper::guess_sympd_anysize(A);
      #else
        const bool try_sympd = false;
      #endif
      
      if(try_sympd)
        {
        status = auxlib::inv_sympd(out, A);
        
        if(status == false)  { arma_extra_debug_print("warning: sympd optimisation failed"); }
        }
      
      // auxlib::inv_sympd() may have failed because A isn't really sympd
      }
    }
  
  if(status == false)
    {
    status = auxlib::inv(out, A);
    }
  
  return status;
  }



template<typename T1>
inline
bool
op_inv::apply_diagmat(Mat<typename T1::elem_type>& out, const T1& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const diagmat_proxy<T1> A(X);
  
  arma_debug_check( (A.n_rows != A.n_cols), "inv(): given matrix must be square sized" );
  
  const uword N = (std::min)(A.n_rows, A.n_cols);
  
  bool status = true;
  
  if(A.is_alias(out) == false)
    {
    out.zeros(N,N);
    
    for(uword i=0; i<N; ++i)
      {
      const eT val = A[i];
      
      out.at(i,i) = eT(1) / val;
      
      status = (val == eT(0)) ? false : status;
      }
    }
  else
    {
    Mat<eT> tmp(N, N, fill::zeros);
    
    for(uword i=0; i<N; ++i)
      {
      const eT val = A[i];
      
      tmp.at(i,i) = eT(1) / val;
      
      status = (val == eT(0)) ? false : status;
      }
    
    out.steal_mem(tmp);
    }
  
  return status;
  }



template<typename T1>
inline
void
op_inv_tr::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_inv_tr>& X)
  {
  arma_extra_debug_sigprint();
  
  const bool status = auxlib::inv_tr(out, X.m, X.aux_uword_a);
  
  if(status == false)
    {
    out.soft_reset();
    arma_stop_runtime_error("inv(): matrix seems singular");
    }
  }



template<typename T1>
inline
void
op_inv_sympd::apply(Mat<typename T1::elem_type>& out, const Op<T1,op_inv_sympd>& X)
  {
  arma_extra_debug_sigprint();
  
  const bool status = auxlib::inv_sympd(out, X.m);
  
  if(status == false)
    {
    out.soft_reset();
    arma_stop_runtime_error("inv_sympd(): matrix is singular or not positive definite");
    }
  }



//! @}
