/************************************************************************
 *                                                                      *
 *                   Copyright (C) 2002  3Dlabs Inc. Ltd.               *
 *                                                                      *
 ***********************************************************************/

#pragma once

namespace hogbox {

extern void SetNoiseFrequency(int frequency);

extern double noise1(double arg);
extern double noise2(double vec[2]);
extern double noise3(double vec[3]);
extern void normalize2(double vec[2]);
extern void normalize3(double vec[3]);

/*
   In what follows "alpha" is the weight when the sum is formed.
   Typically it is 2, As this approaches 1 the function is noisier.
   "beta" is the harmonic scaling/spacing, typically 2.
*/

extern double PerlinNoise1D(double x,double alpha, double beta, int n);
extern double PerlinNoise2D(double x,double y,double alpha, double beta, int n);
extern double PerlinNoise3D(double x,double y,double z,double alpha, double beta, int n);


};//end hogbox namespace
