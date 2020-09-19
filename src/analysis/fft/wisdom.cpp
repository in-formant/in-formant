#include "fft.h"

const char *wisdom_string = R"END(

(fftw-3.3.8 fftw_wisdom #x3c273403 #x192df114 #x4d08727c #xe98e9b9d
  (fftw_codelet_hc2cfdftv_4_avx 0 #x11bdd #x11bdd #x0 #x7366d115 #xbc457627 #x02c92fe0 #xa85a47d3)
  (fftw_codelet_n2bv_16_avx 0 #x10bdd #x10bdd #x0 #x44e49825 #xf16e70d9 #x113d8862 #xcea0f6fd)
  (fftw_codelet_n1bv_32_avx 0 #x11bdd #x11bdd #x0 #x6e957ac6 #x49219e0c #xa4199dc1 #xbf0892ad)
  (fftw_codelet_q1fv_2_avx 0 #x11bdd #x11bdd #x0 #x52fb06d4 #x28060efe #xf14aa678 #xf47c6a54)
  (fftw_codelet_t2fv_64_avx 0 #x11bdd #x11bdd #x0 #x3fa86e7f #xe1559c42 #xca5400a3 #xeeaf34cf)
  (fftw_dft_r2hc_register 0 #x11bdd #x11bdd #x0 #xb35f7e91 #xe6d55a7e #xce0ab3c9 #xa6eb116a)
  (fftw_codelet_n1fv_128_avx 0 #x11bdd #x11bdd #x0 #xc2173b26 #x5d4dbed8 #xaaa9c253 #x8ccb9dd3)
  (fftw_codelet_r2cf_4 2 #x11bdd #x11bdd #x0 #xca5d0424 #x5cd4fe8e #xd22f5d77 #x7030e605)
  (fftw_codelet_r2cf_8 2 #x11bdd #x11bdd #x0 #x4bb68bdb #x9482b832 #x00861cae #x3e436ae0)
  (fftw_dft_vrank_geq1_register 0 #x11bdd #x11bdd #x0 #xb24698a3 #x45b18473 #x3236da23 #x1110a41a)
  (fftw_codelet_r2cfII_4 2 #x11bdd #x11bdd #x0 #x5cf94158 #x23dcef83 #xc21bb320 #xd71ba064)
  (fftw_codelet_r2cf_16 2 #x11bdd #x11bdd #x0 #xfce3251b #xd6f16bfc #x5d18f92a #xafc27a64)
  (fftw_dft_buffered_register 0 #x11bdd #x11bdd #x0 #xc069a6e5 #xf5e455ef #xba333b07 #x4282ba2e)
  (fftw_codelet_n1bv_64_avx 0 #x11bdd #x11bdd #x0 #x2a0f7ad2 #x77cbb15d #xce6e24a2 #x30895ac2)
  (fftw_codelet_n2fv_32_avx 0 #x10bdd #x10bdd #x0 #x21bf8747 #x375e1866 #x1b9ee1b5 #xa0a9b5c3)
  (fftw_codelet_t2fv_2_avx 0 #x11bdd #x11bdd #x0 #x24f7bdb1 #xc62456a9 #x5e74710c #x0662a34f)
  (fftw_codelet_n1fv_128_avx 0 #x11bdd #x11bdd #x0 #xb864da7c #x2f20dea0 #x3fdca99c #x3f15445f)
  (fftw_codelet_r2cfII_4 2 #x11bdd #x11bdd #x0 #x8ccbbfa1 #xfa873c28 #x99cf0c05 #x662ee324)
  (fftw_codelet_hc2cfdftv_32_avx 0 #x11bdd #x11bdd #x0 #xdc38cc14 #x0d1c6e3c #xc0ae48bf #xa0e0eb5a)
  (fftw_codelet_r2cf_32 2 #x11bdd #x11bdd #x0 #x429d1761 #xc79f9ecc #xdb982251 #x10f2a2eb)
  (fftw_codelet_hc2cfdftv_4_avx 0 #x11bdd #x11bdd #x0 #xfee68690 #xad28e91e #x220cee4d #x14ded02f)
  (fftw_codelet_q1bv_2_avx 0 #x11bdd #x11bdd #x0 #x39c99d15 #x98b0849a #x160c198f #x78fadd88)
  (fftw_codelet_t2bv_2_sse2 0 #x11bdd #x11bdd #x0 #xffc3f17f #x1d19868c #x44074f53 #x55b1eddd)
  (fftw_dft_vrank_geq1_register 0 #x11bdd #x11bdd #x0 #x8b298557 #x26e17aa9 #xe8e54460 #xa41243d6)
  (fftw_codelet_r2cf_4 2 #x11bdd #x11bdd #x0 #xb4863a42 #xca4935f9 #x095db2cd #x90c47430)
  (fftw_codelet_r2cfII_8 2 #x11bdd #x11bdd #x0 #x5eed9572 #xb16425c6 #x6d005e6f #xbfe69d2f)
  (fftw_codelet_t2bv_2_sse2 0 #x11bdd #x11bdd #x0 #x98fa926d #x515b8651 #xa8cce4d8 #x0e995230)
  (fftw_codelet_r2cf_4 2 #x11bdd #x11bdd #x0 #xe43e1ed6 #x21ac391f #xad059268 #x5598cf5c)
  (fftw_dft_vrank_geq1_register 0 #x11bdd #x11bdd #x0 #x40e80f72 #x465bedbc #xe1e0c31a #x669cc599)
  (fftw_codelet_n1fv_128_avx 0 #x11bdd #x11bdd #x0 #x6bc23899 #xf6136c5c #xc944bf1b #x3c04be1c)
  (fftw_codelet_hc2cfdftv_8_avx 0 #x11bdd #x11bdd #x0 #x2b2224fd #xc6d31fc1 #xf08ddb32 #xd3726977)
  (fftw_codelet_t2bv_32_avx 0 #x10bdd #x10bdd #x0 #xd530cc8e #x44e0c4d3 #xab78c6ba #x1fc2a17f)
  (fftw_codelet_t2fv_16_avx 0 #x10bdd #x10bdd #x0 #x5ccd11b9 #x43ed8830 #x7233cc84 #xe3e53a93)
  (fftw_dft_vrank_geq1_register 0 #x11bdd #x11bdd #x0 #x82d9dfb2 #xd601fd65 #xe5b3dca6 #xe942ec31)
  (fftw_codelet_n2fv_64_avx 0 #x11bdd #x11bdd #x0 #xf552c5e8 #xdf70330a #xdbafb425 #x6d016d71)
  (fftw_codelet_n1fv_64_avx 0 #x11bdd #x11bdd #x0 #x6d039190 #x67896213 #x99c5422d #xf83e6e65)
  (fftw_codelet_r2cfII_16 2 #x11bdd #x11bdd #x0 #x47b3e4c5 #xec91b751 #x6a6ec0c8 #xd8006cc8)
  (fftw_dft_buffered_register 0 #x11bdd #x11bdd #x0 #x6e800d78 #x430422a3 #x85efad29 #xc12c36a9)
  (fftw_codelet_n1fv_128_avx 0 #x11bdd #x11bdd #x0 #x37647671 #xa231fde8 #xcd1493d8 #x9f0a3dac)
  (fftw_codelet_r2cfII_32 2 #x11bdd #x11bdd #x0 #x0d723485 #x067d5676 #x69a91896 #x7274711f)
  (fftw_codelet_hc2cfdftv_4_avx 0 #x11bdd #x11bdd #x0 #x4d4b1bfd #x8518ad3f #x36f383f1 #x5a841548)
  (fftw_dft_r2hc_register 0 #x11bdd #x11bdd #x0 #x41e009b5 #x3534db89 #xc5970c3e #xb18d7461)
  (fftw_codelet_hc2cfdftv_16_avx 0 #x11bdd #x11bdd #x0 #x4fc99822 #xf85b48fc #x7d0a0313 #x81834ad0)
  (fftw_codelet_n1fv_128_avx 0 #x11bdd #x11bdd #x0 #xb5339608 #x94096f01 #x8d67a5dc #xaf678599)
  (fftw_codelet_r2cfII_4 2 #x11bdd #x11bdd #x0 #xb6485697 #xd81997cd #xc4a7f1d0 #x2030e0f1)
  (fftw_codelet_q1bv_2_avx 0 #x11bdd #x11bdd #x0 #x90411bb4 #x4d6f1748 #x0a895e58 #xc6df222e)
  (fftw_dft_nop_register 0 #x11bdd #x11bdd #x0 #x9649d2f6 #xe3345794 #x7a462373 #x7b6e7c60)
  (fftw_dft_nop_register 0 #x11bdd #x11bdd #x0 #xd578073b #x58bd0179 #xe8949384 #xb81a5646)
  (fftw_rdft_rank0_register 0 #x11bdd #x11bdd #x0 #x2f173664 #x34cc3f38 #xd46a714a #xac4293be)
  (fftw_codelet_n1fv_32_avx 0 #x11bdd #x11bdd #x0 #x89d6d6e6 #x0bbd0802 #x19e83c22 #x0105f7af)
)

)END";

static bool imported = false;

void Analysis::importFFTWisdom()
{
    if (!imported) {
        fftw_import_wisdom_from_string(wisdom_string);
    }
}
