# SVJSON

[![Build status](https://ci.appveyor.com/api/projects/status/toagb25p1ym5pxiw?svg=true)](https://ci.appveyor.com/project/AndreyGRSV/svjson)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b736701833204648957b48b4c7673a16)](https://app.codacy.com/gh/AndreyGRSV/SVJSON/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![GitHub](https://img.shields.io/github/license/AndreyGRSV/SVJSON?color=blue)](https://github.com/AndreyGRSV/SVJSON/blob/master/LICENSE)
[![Minimum C++ Standard](https://img.shields.io/badge/standard-C%2B%2B98-blue)](https://img.shields.io/badge/standard-C%2B%2B98-blue)

## Usage
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
## Brief description
This parser uses only standard containers for building data tree. For arrays are used std::vector and for objects std::map with std::string as key values. This more safe then use own containers.

```C++
// Read all items from object
for (auto item : obj.getValues())
{
  ...
}
```

Parsing is only first step of the whole process. Performance of parsing can take more time but finding value in the data tree can compensate this disadvantage. Especially if finding is very frequent process.

If you got the JSON string with errors or with absent data the program is not throwing exception and not setting any error. In error or absence data case the program returns empty result. For string it will be empty string (“”), for number value it will be zero (0.0), for boolean – false and for null – null (“null”). If data are mandatory and they are absent you can use it for application level error detection.

Parser is not serialize data tree to JSON string. For this purpose can be used any standard mechanism, such as std::string or std::stringstream. In any case data from parsed JSON can be used for formatting new JSON string.
The data in data tree are not modifiable.

Program supported compilation starting from C++ 98 standard. It can be used for some old compilers for embedded systems like armcc 4.x for example.

## Conclution
-   Supports wide specter of compilers.
-   Easy adding to project.
-   Simple intuitive usage with overloaded operator [].
-   Used well-known standard containers.
-   Did not overloaded nonstandard own api for modification and serialization.

## Special cases

### How to read unknown object data

```C++
// Helper template for unknown JSON
template <typename Tc> class Elem
{
	const Tc* pelem;		// Pointer to element in JSON parsed structure 
	JSONEmpty<Tc> empty;	// Empty object for safety return
	// Type cast to reference overloading
	operator const Tc&() const
	{
		if (pelem)
			return (*pelem);	// Return real elemnt
		return empty.m_Empty;	// Return empty object apropriate type of elemnt
	}
public:
	// Initialize pointer to element 
	Elem (const JSONElement *el) : pelem (dynamic_cast<const Tc*>(el))
	{
	}
	// Casting is succefull or not
	bool is_empty() const { return (pelem == NULL); }
	// Getting the element through casting operator
	const Tc& get_elem() const { return *this; }
};
```
Read all data from object

```C++
for (auto item : obj.getValues())
{
	// item.first pointed to name
	// item.second pointed to value
	
	// Try as JSONString element
	Elem<JSONString> str(item.second); 
	// Empty object returns empty string
	if (str.get_elem().getValue() != "") // Is not empy print value
		cout << str.get_elem().getValue(); 
		
	// Try as JSONStaticString element
	Elem<JSONStaticString> sstr(item.second);
	// Empty object returns pointer to NULL
	if (sstr.get_elem().getValue() != NULL) // Is not empy print value
		cout << sstr.get_elem().getValue(); 
		
	// Try as JSONNumber element
	Elem<JSONNumber> num(item.second);
	// Empty object returns sero
	if (num.get_elem().getValue() != 0) // Is not zero print value
		cout << num.get_elem().getValue(); 
		
	cout << endl;
}
```
