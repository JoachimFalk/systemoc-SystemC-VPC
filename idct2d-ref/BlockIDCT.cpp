#include <math.h> 
#include <stdarg.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <cstdlib>
#include <iostream>
#include <fstream>

// IDCT constants
#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */
#define B_WIDTH				(    8 )
#define B_SIZE				(   64 ) // B_WIDTH * B_HEIGHT
#define MAXVAL_PIXEL		(  255 ) // 2^BITS_PIXEL - 1

// Function prototypes
static void BlockIDCT(short *in, short negM, short *out);
static void idctrow(short *in, short *out);
static void idctcol(short *blk, short negM);


int main(int argc, char *argv[])
{
  int   i;
  char  dc_scaler = 0;
  short dequant_block[B_SIZE];
  short texture_block[B_SIZE];
  short negMaxval;
  #define INAMEblk "test_in.dat"
  #define ONAMEblk "test_out.dat"
  
  std::ifstream fi (INAMEblk);
  
 

  std::cout<<"Hier ist Eingaben:\n";
  for(i=0; i<B_SIZE; i++)
  {
    fi >> dequant_block[i];  
    std::cout<< dequant_block[i]<<"\n";
  }
  
  fi.close();
  
  if (dc_scaler)
          negMaxval = 0;
  else
          negMaxval = -MAXVAL_PIXEL - 1;
	

  BlockIDCT(dequant_block, negMaxval, texture_block);
  
  std::ofstream fo (ONAMEblk);
  
  std::cout<<"Hier ist Ausgaben:\n";
  for(i=0; i<B_SIZE; i++) 
  {
    fo << texture_block[i] <<"\n";
    std::cout<< texture_block[i]<<"\n";
  }
  
  fo.close();
  
  return 0;
}



//
// Block IDCT function
//
static void BlockIDCT(short *in, short negM, short *out)
{
  // 1-D IDCT on rows
  for (int i = 0; i <= 7; ++i)
    idctrow(in+(i<<3), out+(i<<3));
  
  for(int j = 0; j<  64; ++j)
    std::cout<< "IDCTrow out" <<"("<< j <<"): " << out[j] << std::endl;

  // 1-D IDCT on columns
  for (int i = 0; i <= 7; ++i) {
    for (int j = 0; j <= 7; ++j)
      std::cout<< "IDCTcol in" <<"("<< j <<"): " << out[i+8*j] << std::endl;
    // 1-D IDCT on column i
    idctcol(out+i, negM);
    for (int j = 0; j <= 7; ++j)
      std::cout<< "IDCTcol out" <<"("<< j <<"): " << out[i+8*j] << std::endl;
  }
  
  return;
} // BlockIDCT



/* row (horizontal) IDCT
 *
 *           7                       pi         1
 * dst[k] = sum c[l] * src[l] * cos( -- * ( k + - ) * l )
 *          l=0                      8          2
 *
 * where: c[0]    = 128
 *        c[1..7] = 128*sqrt(2)
 */
static void idctrow(short *in, short *out)
{
  int tmpval;
  int x[B_WIDTH];

  /* first stage */
  /* for proper rounding in the fourth stage */
  x[0] = (in[0]<<11) + 128;  // iscale1 (2^11,128)
  x[4] = in[4]<<11;          // iscale2 (2^11,  0)
  
  tmpval = W7*(in[1]+in[7]);      //fly2.t  = (565*(I1+I2))+OS:0
  x[1] = tmpval + (W1-W7)*in[1];  //fly2.O1 = (t+(I1*2276))    /*((coeff1:2841-Coeff7:565)=2276)*/
  x[7] = tmpval - (W1+W7)*in[7];  //fly2.O2 = (t+(I2*(-3406))) /*(-(Coeff1:2841+Coeff7:565)=-3406)*/ 
  tmpval = W3*(in[5]+in[3]);      //fly.t   = (2408*(I1+I2))+OS:0 
  x[5] = tmpval - (W3-W5)*in[5];  //fly.O1  = (t+(I1*(-799)))  /*(-(Coeff3:2408-Coeff5:1609)=-799)*/
  x[3] = tmpval - (W3+W5)*in[3];  //fly.O2  = (t+(I2*(-4017))) /*(-(Coeff3:2408+Coeff5:1609)=-4017*/
  x[2] = in[2];
  x[6] = in[6];
                  
  /* second stage */
  tmpval = x[0] + x[4]; // addsub01.o1 (1,0,0)
  x[0] -= x[4];         // addsub01.o2
  x[4] = W6*(x[2]+x[6]);          //fly3.t  = (1108*(I1+I2))+OS:0
  x[6] = x[4] - (W2+W6)*x[6];     //fly3.O1 = (t+(I1*(-3784))) /*(-(Coeff2:2676+Coeff6:1108)=-3784*/
  x[2] = x[4] + (W2-W6)*x[2];     //fly3.O2 = (t+(I2*1568))    /*((Coeff2:2676-Coeff6:1108)=1568)*/
  x[4] = x[1] + x[5]; // addsub02.o1 (1,0,0)
  x[1] -= x[5];       // addsub02.o2
  x[5] = x[7] + x[3]; // addsub03.o1 (1,0,0)
  x[7] -= x[3];       // addsub03.o2
                  
  /* third stage */
  x[3] = tmpval + x[2]; // addsub04.o1 (1,0,0)
  tmpval -= x[2];       // addsub04.o2
  x[2] = x[0] + x[6];   // addsub05.o1 (1,0,0)
  x[0] -= x[6];         // addsub05.o2
  x[6] = (181*(x[1]+x[7])+128)>>8; // addsub06.o1 (181,128,8)
  x[1] = (181*(x[1]-x[7])+128)>>8; // addsub06.o2
                  
  /* fourth stage */
  out[0] = (x[3]+x[4])>>8;   // addsub09.O1 (1,0,8)
  out[1] = (x[2]+x[6])>>8;   // addsub10.o1 (1,0,8)
  out[2] = (x[0]+x[1])>>8;   // addsub08.o1 (1,0,8)
  out[3] = (tmpval+x[5])>>8; // addsub07.o1 (1,0,8)
  out[4] = (tmpval-x[5])>>8; // addsub07.o2
  out[5] = (x[0]-x[1])>>8;   // addsub08.o2
  out[6] = (x[2]-x[6])>>8;   // addsub10.o2
  out[7] = (x[3]-x[4])>>8;   // addsub09.o2
  
  return;
} // idctrow







/* column (vertical) IDCT
 *
 *             7                         pi         1
 * dst[8*k] = sum c[l] * src[8*l] * cos( -- * ( k + - ) * l )
 *            l=0                        8          2
 *
 * where: c[0]    = 1/1024
 *        c[1..7] = (1/1024)*sqrt(2)
 */
static void idctcol(short *blk, short negMaxval)
{
  int v;
  int x[9];

  /* shortcut */
  if (!((x[1] = (blk[8*4]<<8)) | (x[2] = blk[8*6]) | (x[3] = blk[8*2]) |
		(x[4] = blk[8*1]) | (x[5] = blk[8*7]) | (x[6] = blk[8*5]) | (x[7] = blk[8*3])))
  {
		v = (blk[8*0]+32)>>6;
		blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]=blk[8*6]=blk[8*7] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;

   return;
  }

  
  x[0] = (blk[8*0]<<8) + 8192;// iscale1 (2^8,8129)
	
  /* first stage */
  x[8] = W7*(x[4]+x[5]) + 4;
  x[4] = (x[8]+(W1-W7)*x[4])>>3;
  x[5] = (x[8]-(W1+W7)*x[5])>>3;
  x[8] = W3*(x[6]+x[7]) + 4;
  x[6] = (x[8]-(W3-W5)*x[6])>>3;
  x[7] = (x[8]-(W3+W5)*x[7])>>3;
  
  /* second stage */
  x[8] = x[0] + x[1];
  x[0] -= x[1];
  x[1] = W6*(x[3]+x[2]) + 4;
  x[2] = (x[1]-(W2+W6)*x[2])>>3;
  x[3] = (x[1]+(W2-W6)*x[3])>>3;
  x[1] = x[4] + x[6];
  x[4] -= x[6];
  x[6] = x[5] + x[7];
  x[5] -= x[7];
  
  /* third stage */
  x[7] = x[8] + x[3];
  x[8] -= x[3];
  x[3] = x[0] + x[2];
  x[0] -= x[2];
  x[2] = (181*(x[4]+x[5])+128)>>8;
  x[4] = (181*(x[4]-x[5])+128)>>8;
  
  /* fourth stage */
	v = (x[7]+x[1])>>14;
  blk[8*0] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[3]+x[2])>>14;
  blk[8*1] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v =  (x[0]+x[4])>>14;
  blk[8*2] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[8]+x[6])>>14;
  blk[8*3] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[8]-x[6])>>14;
  blk[8*4] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[0]-x[4])>>14;
  blk[8*5] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[3]-x[2])>>14;
  blk[8*6] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[7]-x[1])>>14;
  blk[8*7] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;

	return;
} // idctcol