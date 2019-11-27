
# SVJSON

[![Build Status](https://travis-ci.org/AndreyGRSV/SVJSON.svg?branch=master)](https://travis-ci.org/AndreyGRSV/SVJSON)
[![GitHub](https://img.shields.io/github/license/AndreyGRSV/SVJSON?color=blue)](https://github.com/AndreyGRSV/SVJSON/blob/master/LICENSE)
![Minimum C++ Standard] (https://img.shields.io/badge/standard-C%2B%2B98-blue)


Simple for use JSON parser in one file

```C++
#include "JSON.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    std::string str = " { \"name\" : \"value\" ,\n  \"name2\" : \"value2\" ,  \"name3\" : 0 ,  \"name4\" : -0.111 ,  \"name5\" : true } ";
    
	  JSONObject psO(str, 0);
	  psO.parse();
    
    std::string name = psO["name"];
    std::string name2 = psO["name2"];
    double name3 = psO["name3"];
    double name4 = psO["name4"];
    const char *name5 = psO["name5"];

    std::cout << "name : " << name << std::endl
              << "name2 : " << name2 << std::endl
              << "name3 : " << name3 << std::endl
              << "name4 : " << name4 << std::endl
              << "name5 : " << name5 << std::endl;
	
  return 0;
}
```
