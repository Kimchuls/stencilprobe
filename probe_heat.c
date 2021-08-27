/*
	StencilProbe Heat Equation
	Implements 7pt stencil from Chombo's heattut example.
*/
#include <stdio.h>
#include "common.h"
#define min(x,y) (x)<(y)?(x):(y)

#ifdef STENCILTEST
void StencilProbe_naive(double* A0, double* Anext, int nx, int ny, int nz,
			int tx, int ty, int tz, int timesteps) {
#else
void StencilProbe(double *A0, double *Anext, int nx, int ny, int nz,
                  int tx, int ty, int tz, int timesteps) {
#endif
  // Fool compiler so it doesn't insert a constant here
  // double fac = A0[0];
  double minxyz = min(min((nx*nx)/ ((nx-1)*(nx-1)), (ny*ny)/ ((ny-1)*(ny-1))), (nz*nz)/ ((nz-1)*(nz-1)));
  double fac = 1/16.0*minxyz;
  double *temp_ptr;
  int i, j, k, t;
  // for (k = 0; k <= nz - 1; k++) {
  //   for (j = 0; j <= ny - 1; j++) {
	//     for (i = 0; i <= nx - 1; i++) {
  //         if (k==0 || j==0||i==0||k==nz-1||j==ny-1||i==nx-1){
  //           A0[Index3D (nx, ny, i, j, k)]=0;
  //           Anext[Index3D (nx, ny, i, j, k)]=0;
  //         }
  //     }
  //   }
  // }
  
  for (t = 0; t < timesteps; t++) {
    printf("\nnow step is t = %d.\n",t);
    for (k = 1; k < nz - 1; k++) {
      for (j = 1; j < ny - 1; j++) {
	      for (i = 1; i < nx - 1; i++) {
          printf("now space is z=%d, y=%d, x=%d:\n",k,j,i);
	        Anext[Index3D (nx, ny, i, j, k)] = A0[Index3D (nx, ny, i, j, k)] -
          +fac/((nx*nx)/ ((nx-1)*(nx-1)))*(A0[Index3D (nx, ny, i + 1, j, k)]+A0[Index3D (nx, ny, i - 1, j, k)]-2*A0[Index3D (nx, ny, i, j, k)])
	        +fac/((ny*ny)/ ((ny-1)*(ny-1)))*(A0[Index3D (nx, ny, i , j+ 1, k)]+A0[Index3D (nx, ny, i , j- 1, k)]-2*A0[Index3D (nx, ny, i, j, k)])
          +fac/((nz*nz)/ ((nz-1)*(nz-1)))*(A0[Index3D (nx, ny, i , j, k+ 1)]+A0[Index3D (nx, ny, i , j, k- 1)]-2*A0[Index3D (nx, ny, i, j, k)]);
          printf("Anext[Index3D (nx, ny, i, j, k)] = %.26lf\n",Anext[Index3D (nx, ny, i, j, k)] );
          
          // Anext[Index3D (nx, ny, i, j, k)] = 
	        //   A0[Index3D (nx, ny, i, j, k + 1)] +
	        //   A0[Index3D (nx, ny, i, j, k - 1)] +
	        //   A0[Index3D (nx, ny, i, j + 1, k)] +
	        //   A0[Index3D (nx, ny, i, j - 1, k)] +
	        //   A0[Index3D (nx, ny, i + 1, j, k)] +
	        //   A0[Index3D (nx, ny, i - 1, j, k)]
	        //   - 6.0 * A0[Index3D (nx, ny, i, j, k)]/fac ;
          // printf("%lf = \n\t%lf+%lf+%lf+%lf+%lf+%lf \n\t-6.0* %lf /%lf\n",Anext[Index3D (nx, ny, i, j, k)],
	        //   A0[Index3D (nx, ny, i, j, k + 1)],A0[Index3D (nx, ny, i, j, k - 1)],A0[Index3D (nx, ny, i, j + 1, k)],
	        //   A0[Index3D (nx, ny, i, j - 1, k)],A0[Index3D (nx, ny, i + 1, j, k)],A0[Index3D (nx, ny, i - 1, j, k)],
	        //   A0[Index3D (nx, ny, i, j, k)], fac );
          // printf("%lf = 6.0* %lf /%lf\n",6.0 * A0[Index3D (nx, ny, i, j, k)]/fac,A0[Index3D (nx, ny, i, j, k)], fac);
	      }
      }
    }
    // for (k = 0; k <= nz - 1; k++) {
    //   for (j = 0; j <= ny - 1; j++) {
	  //     for (i = 0; i <= nx - 1; i++) {
    //       printf("%lf ",Anext[Index3D (nx, ny, i, j, k)] );
    //     }
    //   }
    // }
    temp_ptr = A0;
    A0 = Anext;
    Anext = temp_ptr;
  }
}
