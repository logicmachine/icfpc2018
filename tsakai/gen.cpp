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

  return 0;
}
