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
  unsigned char val[2];
  val[0] = 0b11111111;
  output("halt.txt", val, 1);

  val[0] = 0b11111110;
  output("wait.txt", val, 1);

  val[0] = 0b11111101;
  output("flip.txt", val, 1);

  //definition of smove
  //[00«lld.a»20100]8 [000«lld.i»5]8
  val[0] = 0b00010100;
  val[1] = 0b00000010;
  output("smove_1_2.txt", val, 2);

  //definition of LMove
  // [«sld2.a»2«sld1.a»21100]8 [«sld2.i»4«sld1.i»4]8
  val[0] = 0b10011100;
  val[1] = 0b00001000;
  output("lmove_2_1_0_8.txt", val, 2);

  // definition of FusionP
  // [«nd»5111]8
  val[0] = 0b00111111;
  output("fusionp_7.txt", val, 1);

  // definition of FusionS
  // [«nd»5 110]8
  val[0] = 0b10011110;
  output("fusionp_19.txt", val, 1);

  // definition of Fission
  // [«nd»5101]8 [«m»8]8  
  val[0] = 0b01110101;
  val[1] = 0b00000101;
  output("fission_14_5.txt", val, 2);

  // definition of fill
  //[«nd»5011]8
  val[0] = 0b01010011;
  output("fill_10.txt", val, 1);

  return 0;
}
