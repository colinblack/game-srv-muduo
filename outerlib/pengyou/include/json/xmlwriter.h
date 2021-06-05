#ifndef _JSON_XML_WRITER_H
#define _JSON_XML_WRITER_H

#include "value.h"
#include <string>

namespace Json 
{
	using std::string;

	class Value;

	/**
	 * ��Json::Value���ݽṹ��XML��ʽ���.
	 *
	 * ע�⣺��������Ԫ�ص������
	 *
	 * 	JSON��ʽΪ��{ "array" : [1,2,3] }
	 *
	 *	�����XMLΪ��
	 *   <array>
	 *     <i>1</i>
	 *     <i>2</i>
	 *     <i>3</i>
	 *   </array>
	 *
	 *  ��Ƕ��<i>��ȱʡtag��Ҳ������"array:item"��ָ����
	 *    {"array:item" : [1,2,3]}
	 *  �����
	 *    <array>
	 *    	<item>1</item>
	 *    	<item>2</item>
	 *    	<item>3</item>
	 *    </array>
	 */
	class JSON_API XMLWriter
	{
	public:
		string write(const Value& root, const string& tag = "root");
	private:
		void writeValue(const Value& value, const string& tag);

		string m_doc;
	};
}

#endif
