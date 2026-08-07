/* How often does each spectral bin win the local-ratio maximum, on scenes that
 * contain NOTHING?  Under an unbiased background estimator this is flat.
 *
 * This is the measurement behind item 111.  rs_local_ratio() scores a bin
 * against the median of its neighbourhood, and the median of fewer draws is more
 * variable; the statistic is then MAXIMISED over the band, so any bin with a
 * shorter neighbourhood wins the maximum more often for no reason in the scene.
 * Before the fix the 39% of the band with a clipped neighbourhood took 72% of
 * the maxima, a per-bin rate of 4.0x; after it, 0.89x.
 *
 * Build it against the static library, from the repository root:
 *
 *     cc -O2 -I include runs/kilauea/2026-08-07-refbins/measure_edge_bias.c \
 *        build/libmm_core.a build/libpocketfft.a -o /tmp/edge_bias -lm
 */
#include "resonarsat/microm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static double nb(size_t w,size_t k,unsigned s){unsigned x=(unsigned)(w*73856093u)^(unsigned)(k*19349663u)^(s*83492791u);x^=x>>16;x*=2246822519u;x^=x>>13;x*=3266489917u;x^=x>>16;return -log(((double)x+0.5)/4294967296.0);}
int main(void){
  const size_t NF=65,NW=225,K_LO=3; const size_t NB=NF-K_LO;
  double *hist=calloc(NF,sizeof*hist); size_t trials=200;
  for(unsigned t=0;t<trials;t++){
    rs_spectrum_t s; memset(&s,0,sizeof s);
    s.n_win=NW;s.n_win_az=15;s.n_win_rg=15;s.n_freq=NF;s.df=0.1663;
    s.psd=calloc(NW*NF,sizeof*s.psd); s.freq=calloc(NF,sizeof*s.freq);
    for(size_t k=0;k<NF;k++)s.freq[k]=k*0.1663;
    for(size_t w=0;w<NW;w++)for(size_t k=0;k<NF;k++)s.psd[w*NF+k]=nb(w,k,1000u+t);
    /* rs_spectrum_local_window takes the ARGMAX over all windows and bins --
     * the same nomination rule, one pick instead of six. */
    rs_local_peak_t lp;
    if(rs_spectrum_local_window(&s,&lp)==RS_OK) hist[lp.bin]+=1.0;
    rs_spectrum_free(&s);
  }
  printf("argmax bin over %zu motionless scenes (flat would be %.1f%% per bin):\n",trials,100.0/NB);
  double edge=0,mid=0; size_t nedge=0,nmid=0;
  for(size_t k=K_LO;k<NF;k++){
    int is_edge = (k < K_LO+12) || (k+12 >= NF);
    if(hist[k]>0) printf("  bin %2zu %5.1f%%%s\n",k,100.0*hist[k]/trials,is_edge?"   <- starved neighbourhood":"");
    if(is_edge){edge+=hist[k];nedge++;} else {mid+=hist[k];nmid++;}
  }
  printf("\n  starved bins: %zu of %zu (%.0f%% of the band) took %.0f%% of the maxima\n",
         nedge,NB,100.0*nedge/NB,100.0*edge/trials);
  printf("  per-bin rate: starved %.2f%%  vs  full %.2f%%   ratio %.2fx\n",
         100.0*edge/trials/nedge,100.0*mid/trials/nmid,(edge/nedge)/(mid/nmid));
  return 0;}
