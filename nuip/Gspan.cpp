#include<bits/stdc++.h>
#include "nbtasm.hpp"
#include "read_model.hpp"

using namespace std;

/*
 —LŒü‘Sˆæ–Ø‚ğ\’z
 ground ‚Í [Vec3{0, -1, 0}, Vec3{R-1, -1, R-1}]
 */

// template<class S, class T> ostream& operator<<(ostream &os, const pair<S,T> &t) { return os<<"("<<t.first<<","<<t.second<<")";}


#include "Gtree.hpp"

int main(int argc, char* argv[]){
  ios_base::sync_with_stdio(false);
  cout<<fixed<<setprecision(0);

	VoxelGrid v = read_data(argv[1]);


	Gtree tree(v);
	
	// {
	// 	auto g=Gspan(v);
	// 	for(auto p:g){
	// 		for(auto v:p.second){
	// 			cout<<"\""<<p.first<<"\" \""<<v<<"\""<<endl;
	// 		}
	// 	}
	// 	return 0;
	// }
  return 0;
}
