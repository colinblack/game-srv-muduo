/**
 * 国际化
 *
 * 1.cgi hardcode tips part:
 * 所有语言一个文件
 *
 * 读取配置
 * NeedKey("a");
 * NeedKey("b");
 * InitKeyValues("/path/to/file");
 *
 * 使用讲到的文本
 * GetValue("a",1);
 * GetValue("b",1);
 *
 * 一次设定，多次读取
 * SetLang(1);
 * GetValue("a")
 * GetValue("b")
 *
 * 2.config file part:
 * 每个配置文件对应n(n为支持的语言类型的个数)配置文件
 *  
 * GetConfFilePath("/data/release/farmland/conf/server/items.xml", sConfFilePath)
 *
 * @author paulhuang
 * @author nickyan
 * history:
 * 1.修改map为hashmap以加快查找速度。paulhuang,2010/03/02
 * 2.增加对配置文件国际化需要的功能，语言id到配置文件的转换。
 *    paulhuang,2010/03/15
 * 3.增加模块间调用ID的分语言映射。paulhuang,2010/03/24
 */
#ifndef __CSTR4I_H__
#define __CSTR4I_H__

#include "markupstl.h"
#include "base_comm_define.h"
#include <string>
#include <set>
#include <vector>
#ifdef __GNUC__
#if __GNUC__ < 3
#include <hash_map.h>
namespace Sgi { using ::hash_map; };
#else
#include <ext/hash_map>
#if __GNUC_MINOR__ == 0
namespace Sgi = std;
#else
namespace Sgi = ::__gnu_cxx;
#endif
#endif
#else
namespace Sgi = std;
#endif

using namespace Sgi;
using namespace std;

struct str_hash_str4i
{
	size_t operator()(const std::string& str) const
	{
		return __stl_hash_string(str.c_str());
	}
};

class CStr4I
{
	public:
		enum {
			//中文简体
			LANG_CN_SIMPLE = 0,
		};
	public:
		CStr4I();
		virtual ~CStr4I();
		/**
		 * @brief 设置需要读取的key
		 *
		 * @param pszKey 需要读取的key
		 */
		void NeedKey(const char * pszKey);
		/**
		 * @brief 从配置文件里读取需要的key
		 *
		 * @param 配置文件路径
		 */
		int InitKeyValues(const char * pszConfFile);
		/**
		 * @brief 用指定的语言读取
		 *
		 * @param pszKey 文本的内容
		 * @param nType 类型
		 */
		const char * GetValue(const char * pszKey, int nType);	
		/**
		 * 设置默认的语言
		 *
		 * @param int
		 */
		static void SetLang(int type)
		{
			m_defaultType = type >=0 ? type : LANG_CN_SIMPLE;
		}
		/**
		 * 读取默认的语言
		 *
		 */
		static int GetLang()
		{
			return m_defaultType ;
		}
		/**
		 * @brief 用默认的语言读取
		 *
		 * @param key 文本的内容
		 */
		const char * GetValue(const char * key)
		{
			return GetValue(key, m_defaultType);
		}	
		/**
		 * @brief 发生错误时间查询错误信息
		 */
		const char * GetErrMsg();
		
		//配置文件映射部分
		int GetConfLang()
		{
			return m_defaultType <= m_nConfTypeMax ? m_defaultType : LANG_CN_SIMPLE;
		}
		int GetConfFilePath(const string & sBaseConfFilePath, int nLang, string & sConfFilePath);
		int GetConfTypeMax()
		{
			if (m_bConfInitSucc)
			{
				return m_nConfTypeMax;
			}

			return -1;
		}

		//module id映射部分
		int InitModuleId(const char * pszModuleIdFile);
		int GetModuleId(const int nBaseId, int & nModuleId);

	private:
		int ConfInit(const char * pszConfFile);

	private:
		hash_map<string, vector<string>, str_hash_str4i> m_mapKeyValues;
		set<string> m_setKeys;
		string m_strErrMsg;
		int m_nTypeMax;
		static int m_defaultType;

		static vector<string> m_vsId2Mark;
		static int m_nConfTypeMax;
		static bool m_bConfInitSucc;	

		static hash_map < int, vector < int > > m_mapModuleId;
		static int m_nModuleTypeMax;
		static bool m_bModuleInitSucc;
};

#endif//!__CSTR4I_H__
