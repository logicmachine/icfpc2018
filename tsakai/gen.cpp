#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <functional>
#include <iomanip>
#include <cstdio>

using namespace std;

void output(char *filename, unsigned char val[], int size){
  FILE *fp;
  fp = fopen(filename, "wb");
  fwrite(val, 1, size, fp);
  fclose(fp);
}

int main(){
  unsigned char val[4];
  val[0] = 0b11111111;
  output("halt.txt", val, 1);

  val[0] = 0b11111110;
  output("wait.txt", val, 1);

  val[0] = 0b11111101;
  output("flip.txt", val, 1);

  //definition of smove
  //[00«lld.a»20100]8 [000«lld.i»5]8
  val[0] = 0b00010100;
  val[1] = 0b00011011;
  output("smove_12_0_0.txt", val, 2);

  //definition of smove
  //[00«lld.a»20100]8 [000«lld.i»5]8
  val[0] = 0b00110100;
  val[1] = 0b00001011;
  output("smove_0_0_-4.txt", val, 2);

  //definition of LMove
  // [«sld2.a»2«sld1.a»21100]8 [«sld2.i»4«sld1.i»4]8
  val[0] = 0b10011100;
  val[1] = 0b00001000;
  output("lmove_3_0_0__0_-5_0.txt", val, 2);

  //definition of LMove
  // [«sld2.a»2«sld1.a»21100]8 [«sld2.i»4«sld1.i»4]8
  val[0] = 0b11101100;
  val[1] = 0b01110011;
  output("lmove_0_-2_0__0_0_2.txt", val, 2);


  // definition of FusionP
  // [«nd»5111]8
  //FusionP <-1,1,0> is encoded as [00111111].
  val[0] = 0b00111111;
  output("fusionp_-1_1_0.txt", val, 1);


  // definition of FusionS
  // [«nd»5 110]8
  //FusionS <1,-1,0> is encoded as [10011110].
  val[0] = 0b10011110;
  output("fusionS_1_-1_0.txt", val, 1);


  // definition of Fission
  // [«nd»5101]8 [«m»8]8  
  val[0] = 0b01110101;
  val[1] = 0b00000101;
  output("fission_0_0_1__5.txt", val, 2);

  // definition of fill
  //[«nd»5011]8
  val[0] = 0b01010011;
  output("fill_0_-1_0.txt", val, 1);

  // definition of void
  val[0] = 0b10111010;
  output("void_1_0_1.txt", val, 1);

  //definition of GFill
  //[«nd»5001]8 [«fd.dx»8]8 [«fd.dy»8]8 [«fd.dz»8]8
  //For example, GFill <0,-1,0> <10,-15,20> is encoded as 
  val[0] = 0b01010001;
  val[1] = 0b00101000;
  val[2] = 0b00001111;
  val[3] = 0b00110010;
  output("GFill_0_-1_0__10_-15_20.txt", val, 4);

  // definition of GVoid
  //[«nd»5000]8 [«fd.dx»8]8 [«fd.dy»8]8 [«fd.dz»8]8
  //For example, GVoid <1,0,0> <5,5,-5> is encoded as [] [] [] [].
  val[0] = 0b10110000;
  val[1] = 0b00100011;
  val[2] = 0b00100011;
  val[3] = 0b00011001;
  output("GVoid_1_0_0__5_5_-5.txt", val, 4);



  return 0;
}
