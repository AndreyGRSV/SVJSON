/*
 *  Simple JSON parser for embedded systems
 *
 *
 *
 *  Created by Andrey Svyatovets
 *
 */

#ifndef __JSON_HPP
#define __JSON_HPP

#ifndef _STRING_
#include <string>	
#endif

#ifndef _MAP_
#include <map>	
#endif

#ifndef _VECTOR_
#include <vector>	
#endif

#ifndef _TYPEINFO
#include <typeinfo>
#endif

#ifndef _CCTYPE_
#include <cctype>
#endif

#ifndef _ALGORITHM_
#include <algorithm>
#endif

#include <cstring>
#include <memory>

#if __cplusplus < 201103L 
#if !defined _MSC_VER || (defined(_MSC_VER) && _MSC_VER < 1900) 
namespace sag
{
	template<bool B, class T = void> struct enable_if {};
	template<class T> struct enable_if<true, T> { typedef T type; };

	struct true_type { enum { value = true }; };
	struct false_type { enum { value = false }; };

	template <class _Ty> struct is_array : false_type { };
	template <class _Ty, size_t _Nx> struct is_array<_Ty[_Nx]> : true_type { };
	template <class _Ty> struct is_array<_Ty[]> : true_type {};

	template<class T, class U> struct is_same : false_type {};
	template<class T> struct is_same<T, T> : true_type {};
}
using namespace sag;
#else
using namespace std;
#endif
#else
using namespace std;
#endif


template <typename T> struct JSONEmpty
{
	static T m_Empty;
};

class JSONString;
class JSONStaticString;
class JSONNumber;
class JSONObject;
class JSONArray;

class JSONElement
{
	template <class Ti> typename enable_if< is_same<Ti, std::string>::value, std::string>::type getTypeValue() const;
	template <class Ti> typename enable_if< is_same<Ti, const char*>::value, const char*>::type getTypeValue() const;
	template <class Ti> typename enable_if< is_same<Ti, bool>::value, bool>::type getTypeValue() const;
	template <class Ti> typename enable_if< is_same<Ti, double>::value, double>::type getTypeValue() const;

	JSONElement& operator = (const JSONElement&);

protected:
	std::size_t m_Pos;
	const std::string& m_sJSON;
	JSONEmpty<std::string> m_EmptyString;

	JSONEmpty<JSONArray> m_EmptyArray;
	JSONEmpty<JSONObject> m_EmptyObject;

public:
	JSONElement (const std::string& sJSON, std::size_t pos = 0) : m_Pos(pos), m_sJSON(sJSON) { }
	JSONElement() : m_Pos(0), m_sJSON(m_EmptyString.m_Empty) 
	{
	}
	virtual ~JSONElement () {}
	virtual std::size_t parse()
	{
		m_Pos = m_sJSON.find_first_not_of (" \t\n\r", m_Pos);
		return m_Pos;
	}

	template <class Ti> operator Ti () const { return getTypeValue<Ti>(); }

	const JSONElement& operator[](const char* name) const;
	const JSONElement& operator[](const int idx) const;
	operator JSONObject& () const;
	operator JSONArray& () const;
};

class JSONStaticString : public virtual JSONElement
{
	const char* m_pValue;

	struct ToLower {
		char operator ()(char& c) { return (char)std::tolower(c); }
	};

	std::string to_lower (const std::string& val)
	{
		std::string v = val;
		ToLower tl;
		std::transform(v.begin(), v.end(), v.begin(), tl);
		return v;
	}

public:
	JSONStaticString (const std::string& sJSON, std::size_t pos, const char* static_value) : JSONElement (sJSON, pos)
		, m_pValue(static_value)	
	{ 
	}
	JSONStaticString() : JSONElement(), m_pValue(NULL)
	{
	}
	const char* getValue () const { return m_pValue; }
	virtual std::size_t parse()
	{
		if (!m_pValue)
			return std::string::npos; // Incorrect value

		JSONElement::parse();
		std::size_t len = std::strlen(m_pValue);
		std::string parsed_value = to_lower(m_sJSON.substr(m_Pos, len));
		if (std::string(m_pValue) == parsed_value)
		{
			m_Pos += len;
			return m_Pos;
		}
		return std::string::npos; // Not a this value
	}
};

class JSONString : public virtual JSONElement
{
	std::string m_Value;
public:
	JSONString (const std::string& sJSON, std::size_t pos) : JSONElement (sJSON, pos)
	{
	}
	JSONString () : JSONElement()
	{
	}
	const std::string& getValue () const { return m_Value; }
	operator std::string() { return m_Value; }
	virtual std::size_t parse()
	{
		m_Pos = JSONElement::parse();
		if (m_Pos == std::string::npos)
			return m_Pos; // No any symbol

		if (m_sJSON[m_Pos] == '"')
		{
			m_Pos++;
			while (true)
			{
				std::size_t start_pos = m_Pos;
				m_Pos = m_sJSON.find_first_of ("\\\"", m_Pos);
				if (m_Pos == std::string::npos)
					return m_Pos; // Did not have closed quotes
				m_Value += m_sJSON.substr(start_pos, m_Pos - start_pos);
				if (m_sJSON[m_Pos] == '\\')
				{
					m_Pos++; 
					m_Value += m_sJSON[m_Pos];
					m_Pos++; // Skip symbol after slash
				}
				else
					break;
			} 
			m_Pos++;
			return m_Pos;
		}
		return std::string::npos; // Is not string value
	}
};

template <typename T> struct NumberData
{
	static const char* m_pValue;
};
template <typename T> const char* NumberData<T>::m_pValue = "0123456789";

class JSONNumber : public virtual JSONElement
{
	double m_Value;
	NumberData<void> m_Numbers;
public:
	JSONNumber (const std::string& sJSON, std::size_t pos) : JSONElement (sJSON, pos)
		, m_Value(0)
	{
	}
	JSONNumber() : JSONElement(), m_Value(0.0) 
	{
	}
	double getValue () const { return m_Value; }

	operator double() {	return m_Value;	}

	virtual std::size_t parse()
	{
		JSONElement::parse();
		if (m_Pos == std::string::npos)
			return m_Pos; // No Value

		std::size_t start_pos = m_Pos;
		if (m_sJSON[m_Pos] == '-') // m_sJSON[m_Pos] == '0' )
			m_Pos++;
		if (!std::isdigit(m_sJSON[m_Pos]))
			return std::string::npos; // Not a digit value

		m_Pos = m_sJSON.find_first_not_of (m_Numbers.m_pValue, m_Pos);
		if (m_Pos == std::string::npos)
			return m_Pos; // Bad ending of object

		if (m_sJSON[m_Pos] == '.')
		{
			m_Pos++;
			m_Pos = m_sJSON.find_first_not_of (m_Numbers.m_pValue, m_Pos);
			if (m_Pos == std::string::npos)
				return m_Pos; // Bad ending of object
		}

		if ( m_sJSON[m_Pos] == 'E' || m_sJSON[m_Pos] == 'e' )
		{
			m_Pos++;
			if (m_sJSON[m_Pos] == '-' || m_sJSON[m_Pos] == '+' )
				m_Pos++;
			m_Pos = m_sJSON.find_first_not_of (m_Numbers.m_pValue, m_Pos);
			if (m_Pos == std::string::npos)
				return m_Pos; // Bad ending of object
		}
		m_Value = std::atof (m_sJSON.substr(start_pos, m_Pos - start_pos).c_str());

		//else 
		//	m_Value = std::atol (m_sJSON.substr(start_pos, m_Pos - start_pos).c_str());

		return m_Pos;
	}
};

class JSONObject;
class JSONArray;

template <typename T> class JSONGroup : virtual public JSONElement
{
protected:

	template <typename valueT> JSONElement* ParseType (std::size_t start_pos, const char* pStaticValue);
	template <typename valueT> JSONElement* ParseType (std::size_t start_pos);
	template <typename valueT> JSONElement* ParseType (
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER > 1900) 
		std::unique_ptr
#else
		std::auto_ptr
#endif
		<valueT> pValue);

	JSONElement* parseValue()
	{

		std::size_t store_pos = m_Pos;
		JSONElement* pValue = NULL;

		// Parse String Value
		pValue = ParseType<JSONString> (store_pos);
		if (pValue)
			return pValue;

		// Parse Numeric Value
		pValue = ParseType<JSONNumber> (store_pos);
		if (pValue)
			return pValue;

		// Parse True Value
		pValue = ParseType<JSONStaticString> (store_pos, "true");
		if (pValue)
			return pValue;

		// Parse False Value
		pValue = ParseType<JSONStaticString> (store_pos, "false");
		if (pValue)
			return pValue;

		// Parse NULL Value
		pValue = ParseType<JSONStaticString> (store_pos, "null");
		if (pValue)
			return pValue;

		// Parse Object Value
		pValue = ParseType<JSONObject> (store_pos);
		if (pValue)
			return pValue;

		// Parse Object Value
		pValue = ParseType<JSONArray> (store_pos);
		if (pValue)
			return pValue;

		return pValue; // Bad format of object or unknown value type
	}

public:
	JSONGroup () 
	{
	}
	JSONGroup (const std::string& sJSON, std::size_t pos) : JSONElement (sJSON, pos)
	{
	}
};

class JSONArray : public virtual JSONGroup<void>
{
	typedef std::vector<const JSONElement*> ArrayType;

	ArrayType m_Values;

	JSONArray (JSONArray&) {}
public:
	JSONArray () 
	{
	}
	JSONArray (const std::string& sJSON, std::size_t pos) : JSONElement (sJSON, pos), JSONGroup<void> (sJSON, pos)
	{
	}
	virtual ~JSONArray()
	{
		for (ArrayType::size_type i = 0; i < m_Values.size(); i++)
			if (m_Values[i])
				delete m_Values[i];
	}
	const ArrayType& getValues () const { return m_Values; }

	const JSONElement& operator[](const ArrayType::size_type _idx) const
	{
		if (_idx >= m_Values.size() )
			return m_EmptyArray.m_Empty;
		const JSONElement* pElement = const_cast<std::vector<const JSONElement*>&>(m_Values)[_idx];
		if (pElement)
			return *pElement;
		return m_EmptyArray.m_Empty;
	}

	virtual std::size_t parse()
	{
		JSONElement::parse();
		if (m_Pos == std::string::npos)
			return m_Pos;
		if (m_sJSON[m_Pos] == '[')
		{
			m_Pos++;
			while (true)
			{
				// Witespace
				JSONElement::parse();
				if (m_Pos == std::string::npos)
					break; // Bad formed object

				if (m_sJSON[m_Pos] == ']')
				{
					m_Pos++;
					break;
				}

				JSONElement* pValue = parseValue();
				if (!pValue)
					return std::string::npos; // Value parsing error

				m_Values.push_back(pValue);

				// Witespace
				JSONElement::parse();
				if (m_Pos == std::string::npos)
					break; // Bad formed object

				if (m_sJSON[m_Pos] == ',')
					m_Pos++;
				if (m_sJSON[m_Pos] == ']')
				{
					m_Pos++;
					break;
				}
			}
		}
		else
			return std::string::npos; // Not an array
		return m_Pos;
	}
};

class JSONObject : public virtual JSONGroup<void>
{
	typedef std::map <std::string, const JSONElement*> DOMType;
	DOMType m_Values;

   class cDeleteValue {
   public:
	   void operator()(const std::pair<std::string, const JSONElement*>& _val)
	   {
		   if (_val.second)
			   delete _val.second;
	   }
   };

    JSONObject (JSONObject& cObj);
public:
	JSONObject () { }
	JSONObject (const std::string& sJSON, std::size_t pos) : JSONGroup<void> (sJSON, pos), JSONElement (sJSON, pos)
	{
	}
	virtual ~JSONObject ()
	{
		cDeleteValue dv;
		std::for_each (m_Values.begin(), m_Values.end(), dv);
	}
	const DOMType& getValues () const { return m_Values; }

	const JSONObject& GetObject (const char* name) const
	{
		if (name)
		{
			const JSONElement* pElement = const_cast<DOMType&>(m_Values)[name];
			if (pElement)
				return dynamic_cast<const JSONObject&>(*pElement);
		}
		return m_EmptyObject.m_Empty;
	}
	const JSONElement& operator[](const char* name) const
	{
		if (name)
		{
			const JSONElement* pElement = const_cast<DOMType&>(m_Values)[name];
			if (pElement)
				return *pElement;
		}
		return m_EmptyObject.m_Empty;
	}
	virtual std::size_t parse()
	{
		// Witespace
		JSONElement::parse(); 
		if (m_Pos == std::string::npos)
			return m_Pos; // Unexpected end

		if (m_sJSON[m_Pos] == '{')
		{
			m_Pos++;
			while (true)
			{
				// Witespace
				JSONElement::parse(); 
				if (m_Pos == std::string::npos)
					break; // Bad formed object

				if (m_sJSON[m_Pos] == '}')
					return ++m_Pos;

				// Parse Name
				JSONString pName(m_sJSON, m_Pos);
				m_Pos = pName.parse();
				if (m_Pos == std::string::npos)
					break; // Bad formed object name

				// Witespace
				JSONElement::parse(); 
				if (m_Pos == std::string::npos)
					break; // Name parse error

				if (m_sJSON[m_Pos] != ':')
					return std::string::npos; // Bad formed object
				m_Pos++;

				JSONElement* pValue = parseValue();
				if (!pValue || m_Pos == std::string::npos)
					return std::string::npos; // Bad formed object

				m_Values[pName.getValue()] = pValue;

				// Witespace
				JSONElement::parse();
				if (m_Pos == std::string::npos)
					break; 

				if (m_sJSON[m_Pos] == ',')
					m_Pos++;

				if (m_sJSON[m_Pos] == '}')
					return ++m_Pos;
			}
		}
		else
			return std::string::npos; // Not an object
		return m_Pos;
	}
};

template <typename T> T JSONEmpty<T>::m_Empty;


template <class Ti> typename enable_if< is_same<Ti, std::string>::value, std::string>::type JSONElement::getTypeValue() const
{
	if (typeid(*this) != typeid(JSONString))
		return "";
	return dynamic_cast<const JSONString&>(*this).getValue();
}
template <class Ti> typename enable_if< is_same<Ti, const char*>::value, const char*>::type JSONElement::getTypeValue() const
{
	if (typeid(*this) != typeid(JSONStaticString))
		return "";
	return dynamic_cast<const JSONStaticString&>(*this).getValue();
}
template <class Ti> typename enable_if< is_same<Ti, bool>::value, bool>::type JSONElement::getTypeValue() const
{
	if (typeid(*this) != typeid(JSONStaticString))
		return false;
	const char* ps = dynamic_cast<const JSONStaticString&>(*this).getValue();
	if (std::string(ps) == "true")
		return true;
	return false;
}
template <class Ti> typename enable_if< is_same<Ti, double>::value, double>::type JSONElement::getTypeValue() const
{
	if (typeid(*this) != typeid(JSONNumber))
		return 0;
	return dynamic_cast<const JSONNumber&>(*this).getValue();
}

template <typename T>
template <typename valueT> JSONElement*  JSONGroup<T>::ParseType (std::size_t start_pos, const char* pStaticValue)
{
	return ParseType<valueT>(
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER > 1900) 
		std::unique_ptr
#else
		std::auto_ptr
#endif
		<valueT> (new valueT (m_sJSON, start_pos, pStaticValue)));
}
template <typename T>
template <typename valueT> JSONElement* JSONGroup<T>::ParseType (std::size_t start_pos)
{
	return ParseType<valueT>(
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER > 1900) 
		std::unique_ptr
#else
		std::auto_ptr
#endif
		<valueT> (new valueT (m_sJSON, start_pos)));
}
template <typename T>
template <typename valueT> JSONElement* JSONGroup<T>::ParseType (
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER > 1900) 
	std::unique_ptr
#else
	std::auto_ptr
#endif
	<valueT> pValue)
{
	m_Pos = pValue->parse();
	if (m_Pos == std::string::npos)
		return NULL;
	return dynamic_cast<JSONElement*> (pValue.release());
}

inline JSONElement::operator JSONObject& () const 
{	
	if (typeid(*this) != typeid(JSONObject))
		return this->m_EmptyObject.m_Empty;
	return dynamic_cast<JSONObject&>(const_cast<JSONElement&>(*this));
}
inline const JSONElement& JSONElement::operator[](const char* name) const 
{	
	if (typeid(*this) != typeid(JSONObject))
		return this->m_EmptyObject.m_Empty;
	return dynamic_cast<const JSONObject&>(*this)[name];
}
inline const JSONElement& JSONElement::operator[](const int idx) const 
{ 
	if (typeid(*this) != typeid(JSONArray))
		return this->m_EmptyArray.m_Empty;
	return dynamic_cast<const JSONArray&>(*this)[idx];
}
inline JSONElement::operator JSONArray& () const
{ 	
	if (typeid(*this) != typeid(JSONArray))
		return this->m_EmptyArray.m_Empty;
	return dynamic_cast<JSONArray&>(const_cast<JSONElement&>(*this));
}

#endif // __JSON_HPP
