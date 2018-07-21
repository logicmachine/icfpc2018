#include "decoder.hpp" 

using namespace std;

void print(FILE *fp, std::string str, std::vector<std::vector<int>> & a){
  fprintf(fp, "%s ", str.c_str());
  // to make visible for human
  for(int i = 0;i < a.size();i++){
    if (i != 0)    fprintf(fp, ",");
    for(int j = 0;j < a[i].size();j++){
      fprintf(fp, "%d ", a[i][j]);
    }
  }
  fprintf(fp, "\n");
}

int main(int argc, char**argv){
  string in = argc < 2 ? "trace.nbt" : argv[1];
  string out = argc < 3 ? "tmp.txt" : argv[2];
  FILE *fp = fopen(in.c_str(), "rb");
  FILE *fout = fopen(out.c_str(), "w");
  if (fp == NULL){
    cout <<"FILE Cannot be read" << endl;
    exit(1);
  }
  int cnt = 0;
  while(1){
    std::pair<decoder::CommandType, std::vector<std::vector<int>>> a = decoder::decode(fp); 
    if (a.first == decoder::CommandType::Halt) {
      print(fout, "Halt", a.second);
    } else if (a.first == decoder::CommandType::Wait) {
      print(fout, "Wait", a.second);
    } else if (a.first == decoder::CommandType::Flip) {
      print(fout, "Flip", a.second);
    } else if (a.first == decoder::CommandType::SMove){
      print(fout, "SMove", a.second);
    } else if (a.first == decoder::CommandType::LMove){
      print(fout, "LMove", a.second);
    } else if (a.first == decoder::CommandType::Fission){
      print(fout, "Fission", a.second);
    } else if (a.first == decoder::CommandType::Fill){
      print(fout, "Fill", a.second);
    } else if (a.first == decoder::CommandType::FusionP){
      print(fout, "FusionP", a.second);
    } else if (a.first == decoder::CommandType::FusionS){
      print(fout, "FusionS", a.second);
    } else if (a.first == decoder::CommandType::Fill){
    } else {
      //print("EMPTY", a.second);
      break;
    }
    cnt++;
    //if (cnt%100 == 0)cout << cnt << endl;
  }
  //cout << cnt << endl;
  fclose(fp);
  fclose(fout);

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
