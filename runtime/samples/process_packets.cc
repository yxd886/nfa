#include <iostream>

#include "../nfaflags.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);
  cout<<"wtf?"<<endl;
}
