
/*!
 * \file cgivalue.h
 * \author FelitHuang
 * \date 2006-02-08
*/

#ifndef    CGIVALUE_H_
#define    CGIVALUE_H_

#include <vector>
#include <string>
#include <list>


/*!
 * \class CCGIValue
 * \remark
 --------------------
 The CCGIValue class parses the CGI parameters, including GET, POST or Multipart-Form-Data.
 And two groups(SINGLE-VALUE/MULTY-VALUE) of methods to get the CGI values specified by the CGI parameter name.
 The SINGLE-VALUE functions will still return the first value if it's a multi-value CGI parameter.
 --------------------
*/
class CCGIValue
{
public:
	enum TYPE{ TYPE_UNKNOW=0, TYPE_GET=1, TYPE_POSTURL, TYPE_POSTFILE };
    
    /*!
     * \brief Get the single CGI value specified by the CGI name.
    */
    static const std::string& GetValue(const std::string& sName);

    /*!
     * \brief Get the single file name specified by the CGI name.
    */
    static const std::string& GetFileName(const std::string& sName);

    /*!
     * \brief Get the single file content specified by the CGI name.
    */
    static const std::string& GetFileData(const std::string& sName);

    /*!
     * \brief Get the multiple CGI value specified by the CGI name.
    */
    static void GetValue(const std::string& sName, std::vector<std::string>& vValue);

    /*!
     * \brief Get the CGI multiple file name specified by the CGI name.
    */
    static void GetFileName(const std::string& sName, std::vector<std::string>& vValue);

    /*!
     * \brief Get the multiple file content specified by the CGI name.
    */
    static void GetFileData(const std::string& sName, std::vector<std::string>& vValue);

    /*!
     * \brief Check if the specified name exists in the CGI value list
     * \return int [ 0--Exists ..--NotExists ]
     * \remark
     --------------------
     Generally speaking this function is not necessarily called.
     If the name does not exists in the CGI value list
     the GET functions can still return the empty value correctly.
     However, if you have the special need, you can call this function for the checking.
     --------------------
    */
    static int CheckExists(const std::string& sName);

    
protected:
    class CData
    {
    public:
        struct CItem
        {
            std::string sName;        /*! <DataName*/
            std::string sData;        /*! <DataData*/
            std::string sValue;       /*! <FileName*/
			int iType;				/*! <ParamType[1-get 2-url_post 3-file_data]*/
        };
    public:
        CData();
        const CItem& GetData(const std::string& sName);
        void GetData(const std::string& sName, std::vector<const CItem*>& vValue);
    protected:
        int  ParseStringValue(std::string& sValue, const std::string& sData, const std::string& sHead, const std::string& sTail, std::string::size_type bpos, std::string::size_type epos);
        void CGIGetParam();
        void CGIPostParam();
        void CGIMultipartParam();
	void CGIDebugParam();
    public:
        std::list<CItem> m_vData;
    };
protected:
    /*!
     * \brief Construct the CGI data by this function
     * \remark
     --------------------
     I don't define the CGI data by using the static data member.
     Because the Compiler and Linker may still Construct it even if it's not used.
     I have defined the static local Object in this function so that it will
     Be Constructed only once when you call it, but will not do so if you don't need
     To use the CGI parameters.
     --------------------
    */
    static CData& GetData();
};

#endif
