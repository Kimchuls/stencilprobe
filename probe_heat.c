/*
	StencilProbe Heat Equation
	Implements 7pt stencil from Chombo's heattut example.
*/
#include <stdio.h>
#include "common.h"
#include "SZ_fault_generator.h"

#ifdef STENCILTEST
void StencilProbe_naive(double* A0, double* Anext, int nx, int ny, int nz,
			int tx, int ty, int tz, int timesteps) {
#else
void StencilProbe(double *A0, double *Anext, int nx, int ny, int nz,
                  int tx, int ty, int tz, int timesteps) {
#endif
  // Fool compiler so it doesn't insert a constant here
  double fac = A0[0];
  double *temp_ptr;
  int i, j, k, t;

  Fault_Injector_Configs fi_config = read_fault_injector_config();
  overwrite_sz_config(fi_config);

  for (t = 0; t < timesteps; t++) 
  {
    if(fi_config.iters == t)
    {
	    printf("#######[Notification]#######\n"); 
      double *temp_p = (double*)malloc(sizeof(double));
      temp_p = SZ_fault_generator_1D(1, &A0[Index3D (nx, ny, fi_config.x, fi_config.y, fi_config.z)]); // SZ fault generated!
      A0[Index3D (nx, ny, fi_config.x, fi_config.y, fi_config.z)] = *temp_p;

      // double *temp_p = (double*)malloc(nz*nx*ny*sizeof(double));
      // printf("\n%lld\n",A0);
      // temp_p = SZ_fault_generator_1D(nz*nx*ny, A0); // SZ fault generated!
      // for (i=0;i<nz*nx*ny;i++){
      //   A0[i]=temp_p[i];
      // }
      free(temp_p);
	    printf("    error added in position [%d, %d, %d].\n", fi_config.x, fi_config.y, fi_config.z);
      printf("    SZ fault injected!\n");
      printf("    Injected Iters: %d\n", fi_config.iters);
      printf("    Error Mode:     %s\n", fi_config.error_type); 
      printf("    Error Bound:    %s\n", fi_config.error_bound_ch);
      printf("############################\n");
    }
    for (k = 1; k < nz - 1; k++) {
      for (j = 1; j < ny - 1; j++) {
	      for (i = 1; i < nx - 1; i++) {
	        Anext[Index3D (nx, ny, i, j, k)] = 
	          A0[Index3D (nx, ny, i, j, k + 1)] +
	          A0[Index3D (nx, ny, i, j, k - 1)] +
	          A0[Index3D (nx, ny, i, j + 1, k)] +
	          A0[Index3D (nx, ny, i, j - 1, k)] +
	          A0[Index3D (nx, ny, i + 1, j, k)] +
	          A0[Index3D (nx, ny, i - 1, j, k)]
	          - 6.0 * A0[Index3D (nx, ny, i, j, k)] / (fac*fac);
	      }
      }
    }
    temp_ptr = A0;
    A0 = Anext;
    Anext = temp_ptr;
  }
}
