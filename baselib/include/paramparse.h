
#ifndef __PARAM_PARSE__
#define	__PARAM_PARSE__

#include <string>
#include <vector>

class CParamParse
{
public:

	struct TData
	{
		std::string sName;
		std::string sValue;
		TData()
		{
		}
		TData(const std::string& xName, const std::string& xValue) : sName(xName), sValue(xValue)
		{
		}
		bool operator == (const TData& stData)
		{
			return (sName == stData.sName);
		}
		bool operator == (const std::string& sName)
		{
			return (this->sName==sName);
		}
	};

public:

	CParamParse();

	CParamParse(const std::string& sParam, const std::string& sDelimElem="&", const std::string& sDelimPair="=", bool bUnescape=true);

	void Parse(const std::string& sParam, const std::string& sDelimElem="&", const std::string& sDelimPair="=", bool bUnescape=true);

	const std::string& GetData(const std::string& sName);

	CParamParse& SetData(const std::string& sName, const std::string& sValue);

	std::vector<std::string>& GetData(const std::string& sName, std::vector<std::string>& vData);
	
	std::vector<TData>::size_type GetSize() const;

	static std::string Construct(std::vector<TData>& vData, const std::string& sDelimElem="&", const std::string& sDelimPair="=", bool bEscape=true);

	operator std::string () const;

	std::string ToString();

protected:

	std::string m_sDelimElem;
	std::string m_sDelimPair;
	bool m_bEscape;
	std::vector<TData> m_vData;
};

#endif

