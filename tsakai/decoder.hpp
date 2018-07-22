#ifndef DECODER_H
#define DECODER_H
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <functional>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <string>
#include <cassert>

namespace decoder{
  enum AXIS{
    X = 0, Y = 1, Z = 2
      };
  
  enum class CommandType {
    Empty,
      Halt,
      Wait,
      Flip,
      SMove,
      LMove,
      Fission,
      Fill,
      FusionP,
      FusionS,
      Void,
      GFill,
      GVoid
      };
  
  // command to index =  [01234567]
  std::vector<int> getBits(unsigned char in){
    std::vector<int> ret;
    for(int i = 0;i < 8;i++){
      ret.emplace_back(in&1);
      in = in / 2;
    }
    reverse(begin(ret), end(ret));
    return ret;
  }

  bool isSame(const std::vector<int> & bits, std::string str){
    for(int i = 0;i < 8;i++){
      if (str[i] == '*')continue;
      else if ((int)(str[i]-'0') != bits[i]){
	return false;
      }
    }
    return true;
  }

  int getVal(const std::vector<int> & bits, int l, int r){
    int ret = 0;
    for(int i = l;i <= r;i++){
      ret = ret * 2 + bits[i];
    }
    return ret;
  }

  void printVec(const std::vector<int> & a){
    std::cout <<"printVec: " ;
    for(int i = 0;i < a.size();i++){
      std::cout << a[i] <<" ";
    }
    std::cout << std::endl;
  }
  /*
    if dx ≠ 0, then a = [01]2 and i = dx + 5
    if dy ≠ 0, then a = [10]2 and i = dy + 5
    if dz ≠ 0, then a = [11]2 and i = dz + 5
  */

  std::vector<int> getsld(int a, int i){
    std::vector<int> ret(3, 0);
    if (a == 0b01){ // dx
      ret[AXIS::X] = i - 5;
    } else if (a == 0b10){
      ret[AXIS::Y] = i - 5;
    } else if (a == 0b11){
      ret[AXIS::Z] = i - 5;
    }
    return ret;
  }
    
  std::vector<int> getlld(int a, int i){
    std::vector<int> ret(3, 0);
    if (a == 0b01){ // dx
      ret[AXIS::X] = i - 15;
    } else if (a == 0b10){
      ret[AXIS::Y] = i - 15;
    } else if (a == 0b11){
      ret[AXIS::Z] = i - 15;
    }
    return ret;
  }

  std::vector<int> getnd(int nd){
    //(dx + 1) * 9 + (dy + 1) * 3 + (dz + 1)
    std::vector<int> ret(3, 0);
    ret[AXIS::Z] = (nd % 3) - 1;
    nd /= 3;
    ret[AXIS::Y] = (nd % 3) - 1;
    nd /= 3;
    ret[AXIS::X] = (nd % 3) - 1;
    nd /= 3;
    return ret;
  }
  std::vector<int> getfd(int fd_x, int fd_y, int fd_z){
    std::vector<int> ret(3);
    ret[AXIS::X] = fd_x - 30;
    ret[AXIS::Y] = fd_y - 30;
    ret[AXIS::Z] = fd_z - 30;
    return ret;
  }
  
  
  std::pair<CommandType,std::vector<std::vector<int>> >  decode(FILE *fp){
    CommandType command =  CommandType::Empty;
    std::vector<std::vector<int> > option;
    std::pair<CommandType, std::vector<std::vector<int>>> ret;
    
    unsigned char a, b, x ,y, z;
    int cnt = fread(&a,1,1 ,fp);
    if (cnt == 0){
      ret.first = CommandType::Empty;
      return ret;
    }

    std::vector<int> bits = getBits(a);
    
    if (a == 0b11111111){ 
      //std::cout <<"Halt" << std::endl;
      ret.first = CommandType::Halt;
    } else if (a == 0b11111110){
      //std::cout <<"Wait" << std::endl;
      ret.first = CommandType::Wait;
    } else if (a == 0b11111101){
      //std::cout <<"Flip" << std::endl;
      ret.first = CommandType::Flip;
    } else if (isSame(bits, std::string("00**0100"))){
      //std::cout <<"SMove" << std::endl;
      ret.first = CommandType::SMove;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int lld_a = getVal(bits, 2, 3);  
      int lld_i = getVal(bits2, 3, 7); 
      option.emplace_back(getlld(lld_a, lld_i)); 
    }  else if (isSame(bits, std::string("****1100"))){
      //std::cout <<"LMove" << std::endl;
      //«sld2.a»2«sld1.a»21100]8 [«sld2.i»4«sld1.i»4]8
      ret.first = CommandType::LMove;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int sld2_a = getVal(bits,  0, 1);
      int sld1_a = getVal(bits,  2, 3);
      int sld2_i = getVal(bits2, 0, 3);
      int sld1_i = getVal(bits2, 4, 7);
      option.emplace_back(getsld(sld1_a, sld1_i));
      option.emplace_back(getsld(sld2_a, sld2_i));
    } else if (isSame(bits, std::string("*****111"))){
      //std::cout <<"FusionP" << std::endl;
      ret.first = CommandType::FusionP;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(getnd(nd));// to be modified
    } else if (isSame(bits, std::string("*****110"))){
      //std::cout <<"FusionS" << std::endl;
      ret.first = CommandType::FusionS;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(getnd(nd));
    } else if (isSame(bits, std::string("*****101"))){
      //std::cout <<"Fission" << std::endl;
      ret.first = CommandType::Fission;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int nd = getVal(bits,  0, 4); // to be modified
      int m  = getVal(bits2, 0, 7);// to be modified
      option.emplace_back(getnd(nd));
      option.emplace_back(std::vector<int>(1,m) );
    } else if (isSame(bits, std::string("*****011"))){
      //std::cout <<"Fill" << std::endl;
      //[«nd»5011]8
      ret.first = CommandType::Fill;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(getnd(nd));
    } else if ( isSame(bits, std::string("*****010"))){
      //Void nd:
      //std::cout << "Void" << std::endl;
      //[«nd»5010]8
      //For example, Void <1,0,1> pis encoded as [10111010].
      ret.first = CommandType::Void;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(getnd(nd));
    } else if ( isSame(bits, std::string("*****001"))){
      // GFill fd
      //std::cout << "GFill" << std::endl;
      //[«nd»5001]8 [«fd.dx»8]8 [«fd.dy»8]8 [«fd.dz»8]8
      //For example, GFill <0,-1,0> <10,-15,20> is encoded as [01010001] [00101000] [00001111] [00110010].
      ret.first = CommandType::GFill;
      int nd = getVal(bits, 0, 4);
      cnt = fread(&x, 1, 1, fp);
      std::vector<int> bits2 = getBits(x);
      int fd_x = getVal(bits2, 0, 7);
      fread(&y, 1, 1, fp);
      std::vector<int> bits3 = getBits(y);
      int fd_y = getVal(bits3, 0, 7);
      fread(&z, 1, 1, fp);
      std::vector<int> bits4 = getBits(z);
      int fd_z = getVal(bits4, 0, 7);
      option.emplace_back(getnd(nd));
      option.emplace_back(getfd(fd_x, fd_y, fd_z));
    } else if (isSame(bits, std::string("*****000"))){
      //GVoid nd fd:
      //[«nd»5000]8 [«fd.dx»8]8 [«fd.dy»8]8 [«fd.dz»8]8
      //For example, GVoid <1,0,0> <5,5,-5> is encoded as [10110000] [00100011] [00100011] [00011001].
      ret.first = CommandType::GVoid;
      int nd = getVal(bits, 0, 4);
      cnt = fread(&x, 1, 1, fp);
      std::vector<int> bits2 = getBits(x);
      int fd_x = getVal(bits2, 0, 7);
      fread(&y, 1, 1, fp);
      std::vector<int> bits3 = getBits(y);
      int fd_y = getVal(bits3, 0, 7);
      fread(&z, 1, 1, fp);
      std::vector<int> bits4 = getBits(z);
      int fd_z = getVal(bits4, 0, 7);
      option.emplace_back(getnd(nd));
      option.emplace_back(getfd(fd_x, fd_y, fd_z));
    }
    
    ret.second = option;
    return ret;
  }
}
#endif //DECODER_H
