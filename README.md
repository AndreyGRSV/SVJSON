
# SVJSON

[![Build Status](https://travis-ci.org/AndreyGRSV/SVJSON.svg?branch=master)](https://travis-ci.org/AndreyGRSV/SVJSON)



Simple for use JSON parser in one file

```C++
#include "JSON.hpp"

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

  return 0;
}

```
