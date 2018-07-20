#include "decoder.hpp" 

void print(std::string str, std::vector<std::vector<int>> & a){
  std::cout <<"print===" << std::endl;
  std::cout << "Command : " << str << std::endl;
  std::cout << "Option : size = " << a.size() << std::endl;
  for(int i = 0;i < a.size();i++){
    for(int j = 0;j < a[i].size();j++){
      std::cout << a[i][j] <<" ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int main(int argc, char **argv){
  FILE *fp = fopen("trace.nbt", "rb");

  while(1){
    std::pair<decoder::CommandType, std::vector<std::vector<int>>> a = decoder::decode(fp); 
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
      break;
    }
  }

  return 0;
}


/*
int main(int argc, char **argv){
  //fout.open("file.txt", ios::out|ios::binary|ios::trunc);
  std::string outfile = argc == 1 ? std::string("halt.txt") : std::string(argv[1]); //読み込むファイルの指定
  FILE *fp = fopen(outfile.c_str(), "rb");

  std::pair<decoder::CommandType, std::vector<std::vector<int>>> a = decoder::decode(fp); 
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
*/
