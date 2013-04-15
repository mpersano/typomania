#ifndef FFT_H_
#define FFT_H_

/* stolen from http://paulbourke.net/miscellaneous/dft/ */

/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/
void fft(int dir,long m,float *x,float *y);

#endif // FFT_H_
