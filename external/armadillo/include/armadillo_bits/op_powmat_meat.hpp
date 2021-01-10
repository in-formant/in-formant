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



//! \addtogroup op_powmat
//! @{


template<typename T1>
inline
void
op_powmat::apply(Mat<typename T1::elem_type>& out, const Op<T1, op_powmat>& expr)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const uword y     =  expr.aux_uword_a;
  const bool  y_neg = (expr.aux_uword_b == uword(1));
  
  if(y_neg)
    {
    if(y == uword(1))
      {
      const bool inv_status = inv(out, expr.m);
      
      if(inv_status == false)
        {
        out.soft_reset();
        arma_stop_runtime_error("powmat(): matrix inverse failed");
        return;
        }
      }
    else
      {
      Mat<eT> X_inv;
      
      const bool inv_status = inv(X_inv, expr.m);
      
      if(inv_status == false)
        {
        out.soft_reset();
        arma_stop_runtime_error("powmat(): matrix inverse failed");
        return;
        }
      
      op_powmat::apply(out, X_inv, y);
      }
    }
  else
    {
    const quasi_unwrap<T1> U(expr.m);
    
    arma_debug_check( (U.M.is_square() == false), "powmat(): given matrix must be square sized" );
    
    op_powmat::apply(out, U.M, y);
    }
  }



template<typename eT>
inline
void
op_powmat::apply(Mat<eT>& out, const Mat<eT>& X, const uword y)
  {
  arma_extra_debug_sigprint();
  
  const uword N = X.n_rows;
  
  if(y == uword(0))  { out.eye(N,N); return; }
  if(y == uword(1))  { out = X;      return; }
  
  if(X.is_diagmat())
    {
    podarray<eT> tmp(N);  // use temporary array in case we have aliasing
    
    for(uword i=0; i<N; ++i)  { tmp[i] = eop_aux::pow(X.at(i,i), int(y)); }
    
    out.zeros(N,N);
    
    for(uword i=0; i<N; ++i)  { out.at(i,i) = tmp[i]; }
    }
  else
    {
         if(y == uword(2))  {                          out = X*X;       }
    else if(y == uword(3))  { const Mat<eT> tmp = X*X; out = X*tmp;     }
    else if(y == uword(4))  { const Mat<eT> tmp = X*X; out =   tmp*tmp; }
    else if(y == uword(5))  { const Mat<eT> tmp = X*X; out = X*tmp*tmp; }
    else
      {
      Mat<eT> tmp = X;
      
      out = X;
      
      uword z = y-1;
      
      while(z > 0)
        {
        if(z & 1)  { out = tmp * out; }
        
        z /= uword(2);
        
        if(z > 0)  { tmp = tmp * tmp; }
        }
      }
    }
  }



template<typename T1>
inline
void
op_powmat_cx::apply(Mat< std::complex<typename T1::pod_type> >& out, const mtOp<std::complex<typename T1::pod_type>,T1,op_powmat_cx>& expr)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type in_eT;
  typedef typename T1::pod_type  in_T;
  typedef std::complex<in_T>     out_eT;
  
  const in_T y = std::real(expr.aux_out_eT);
  
  if( y == in_T(int(y)) )
    {
    arma_extra_debug_print("op_powmat_cx::apply(): integer exponent detected; redirecting to op_powmat");
    
    out = conv_to< Mat<out_eT> >::from( powmat(expr.m,int(y)) );
    
    return;
    }
  
  const quasi_unwrap<T1> U(expr.m);
  const Mat<in_eT>& A  = U.M;
  
  arma_debug_check( (A.is_square() == false), "powmat(): given matrix must be square sized" );
  
  const uword N = A.n_rows;
  
  if(A.is_diagmat())
    {
    podarray<out_eT> tmp(N);  // use temporary array in case we have aliasing
    
    for(uword i=0; i<N; ++i)  { tmp[i] = eop_aux::pow( std::complex<in_T>(A.at(i,i)), y) ; }
    
    out.zeros(N,N);
    
    for(uword i=0; i<N; ++i)  { out.at(i,i) = tmp[i]; }
    
    return;
    }
  
  #if defined(ARMA_OPTIMISE_SYMPD)
    const bool try_sympd = sympd_helper::guess_sympd_anysize(A);
  #else
    const bool try_sympd = false;
  #endif
  
  if(try_sympd)
    {
    Col<in_T>  eigval;
    Mat<in_eT> eigvec;
    
    const bool eig_status = eig_sym(eigval, eigvec, A);
    
    if(eig_status)
      {
      eigval = pow(eigval, y);
      
      const Mat<in_eT> tmp = diagmat(eigval) * eigvec.t();
      
      out = conv_to< Mat<out_eT> >::from(eigvec * tmp);
      
      return;
      }
    
    // fallthrough
    }
  
  bool powmat_status = false;
  
  Col<out_eT> eigval;
  Mat<out_eT> eigvec;
  
  const bool eig_status = eig_gen(eigval, eigvec, A);
  
  if(eig_status)
    {
    eigval = pow(eigval, y);
    
    Mat<out_eT> eigvec_t = trans(eigvec);
    Mat<out_eT> tmp      = diagmat(conj(eigval)) * eigvec_t;
    
    const bool solve_status = auxlib::solve_square_fast(out, eigvec_t, tmp);
    
    if(solve_status)  { out = trans(out); powmat_status = true; }
    }
  
  if(powmat_status == false)
    {
    out.soft_reset();
    
    arma_stop_runtime_error("powmat(): transformation failed");
    }
  }



//! @}
