#ifndef _JSON_XML_WRITER_H
#define _JSON_XML_WRITER_H

#include "value.h"
#include <string>

namespace Json 
{
	using namespace std;

	class Value;

	/**
	 * 将Json::Value数据结构以XML格式输出.
	 *
	 * 注意：对于数组元素的输出：
	 *
	 * 	JSON格式为：{ "array" : [1,2,3] }
	 *
	 *	输出的XML为：
	 *   <array>
	 *     <i>1</i>
	 *     <i>2</i>
	 *     <i>3</i>
	 *   </array>
	 *
	 *  内嵌的<i>是缺省tag，也可以用"array:item"来指定：
	 *    {"array:item" : [1,2,3]}
	 *  则输出
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
