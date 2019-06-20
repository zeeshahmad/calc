#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

namespace util
{
    vector<string> splitString(string str, string del) {
        vector<string> results = vector<string>();
        int pos;
        while ( pos != str.npos ) {
            pos = str.find(del);
            if (pos == str.npos) results.push_back(str); //if coudn't find del in the first round
            else results.push_back(str.substr(0, pos));
            str = str.substr(pos+1);
        }
        return results;
    }

    float random() {
        return (float)rand() / (float) RAND_MAX;
    }


    void findAndReplaceAll(string & data, string toSearch, string replaceStr) {
      size_t pos = data.find(toSearch);
      while( pos != string::npos)	{
        data.replace(pos, toSearch.size(), replaceStr);
        pos =data.find(toSearch, pos + replaceStr.size());
      }
    }

    void mkdir(string dirpath) {
      if (-1 == system(("mkdir -p "+dirpath).c_str()) ) {
        cout << "some error making dir: " << dirpath << endl;
      }
    }

    /*int orderOfMagnitude(double input_num) {
      double abs_number = abs(input_num);
      int order = 0;
      while (abs_number/pow(10.0,order) > 10.) order++;
      while (abs_number/pow(10.0,order) < 1. ) order--;
      return order;
    }*/

};

#endif // UTIL_H
