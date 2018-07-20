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
  
  std::pair<CommandType,std::vector<int>>  decode(FILE *fp){
    CommandType command =  CommandType::Empty;
    std::vector<int> option;
    std::pair<CommandType, std::vector<int> > ret;
    
    unsigned char a, b;
    fread(&a,1,1 ,fp);

    std::vector<int> bits = getBits(a);

    //std::cout << "a = " << (int)a << std::endl;
    
    if (a == 0b11111111){ 
      ret.first = CommandType::Halt;
    } else if (a == 0b11111110){
      ret.first = CommandType::Wait;
    } else if (a == 0b11111101){
      ret.first = CommandType::Flip;
    } else if (isSame(bits, std::string("00**0100"))){
      //[00«lld.a»20100]8 [000«lld.i»5]8
      ret.first = CommandType::SMove;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int lld_a = getVal(bits, 2, 3);  // to be modified
      int lld_i = getVal(bits2, 3, 7); // to be modified
      option.emplace_back(lld_a); 
      option.emplace_back(lld_i); 
    }  else if (isSame(bits, std::string("****1100"))){
      //«sld2.a»2«sld1.a»21100]8 [«sld2.i»4«sld1.i»4]8
      ret.first = CommandType::LMove;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int sld2_a = getVal(bits,  0, 1); // to be modified
      int sld1_a = getVal(bits,  2, 3);// to be modified
      int sld2_i = getVal(bits2, 0, 3);// to be modified
      int sld1_i = getVal(bits2, 4, 7);// to be modified
      option.emplace_back(sld2_a);
      option.emplace_back(sld1_a);
      option.emplace_back(sld2_i);
      option.emplace_back(sld1_i);
    } else if (isSame(bits, std::string("*****111"))){
      ret.first = CommandType::FusionP;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(nd);// to be modified
    } else if (isSame(bits, std::string("*****110"))){
      ret.first = CommandType::FusionS;
      int nd = getVal(bits, 0, 4);
      option.emplace_back(nd);
    } else if (isSame(bits, std::string("*****101"))){
      ret.first = CommandType::Fission;
      fread(&b, 1, 1, fp);
      std::vector<int> bits2 = getBits(b);
      int nd = getVal(bits,  0, 4); // to be modified
      int m  = getVal(bits2, 0, 7);// to be modified
      option.emplace_back(nd);
      option.emplace_back(m);
    } else if (isSame(bits, std::string("*****011"))){
      ret.first = CommandType::Fill;
      int nd = getVal(bits, 0, 4); // to be modified
      option.emplace_back(nd);
    } 

    ret.second = option;
    return ret;
  }
}
 

void print(std::string str, std::vector<int> & a){
  std::cout <<"print===" << std::endl;
  std::cout << "Command : " << str << std::endl;
  std::cout << "Option : " ;
  for(int i = 0;i < a.size();i++){
    std::cout << a[i] <<" ";
  }
  std::cout << std::endl;
}

int main(int argc, char **argv){
  //fout.open("file.txt", ios::out|ios::binary|ios::trunc);
  std::string outfile = argc == 1 ? std::string("halt.txt") : std::string(argv[1]); //読み込むファイルの指定
  FILE *fp = fopen(outfile.c_str(), "rb");

  std::pair<decoder::CommandType, std::vector<int>> a = decoder::decode(fp); 
  std::cout << "filename : " << outfile << std::endl;
  if (a.first == decoder::CommandType::Halt) {
    print("Halt", a.second);
  } else if (a.first == decoder::CommandType::Wait) {
    print("Wait", a.second);
  } else if (a.first == decoder::CommandType::Flip) {
    print("Flip", a.second);
  } else if (a.first == decoder::CommandType::SMove){
    print("SMove", a.second);
  } else if (a.first == decoder::CommandType::LMove){
    print("LMove", a.second);
  } else if (a.first == decoder::CommandType::Fission){
    print("Fission", a.second);
  } else if (a.first == decoder::CommandType::Fill){
    print("Fill", a.second);
  } else if (a.first == decoder::CommandType::FusionP){
    print("FusionP", a.second);
  } else if (a.first == decoder::CommandType::FusionS){
    print("FusionS", a.second);
  } else {
    print("EMPTY", a.second);
  }
  

  return 0;
}
