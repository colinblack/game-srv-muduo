/*
 * ConfigPB.cpp
 *
 *  Created on: 2016-8-22
 *      Author: Ralf
 */

#include "ConfigPB.h"
#include "ConfigManager.h"

bool ConfigPBBase::_j2p(Message* msg, Json::Value &json)
{
	const Descriptor* des = msg->GetDescriptor();
	const Reflection* ref = msg->GetReflection();
	for (int i = 0; i < des->field_count(); ++i)
	{
		const FieldDescriptor* field = des->field(i);
		const string& key = field->name();

		if (field->is_required())
		{
			if(!json.isMember(key))
			{
				error_log("[ConfigPBBase] key is not exists. key=%s", key.c_str());
				return false;
			}
		}

		if(field->is_repeated() && json.isMember(key))
		{
			if(!json[key].isArray())
			{
				error_log("[ConfigPBBase] key is not array. key=%s", key.c_str());
				return false;
			}
			for(unsigned j = 0; j < json[key].size(); ++j)
			{
				switch (field->cpp_type())
				{
				case FieldDescriptor::CPPTYPE_DOUBLE:
					ref->AddDouble(msg, field, json[key][j].asDouble());
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					ref->AddFloat(msg, field, (float)json[key][j].asDouble());
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					ref->AddInt64(msg, field, (int64_t)json[key][j].asDouble());
					break;
				case FieldDescriptor::CPPTYPE_UINT64:
					ref->AddUInt64(msg, field, (uint64_t)json[key][j].asDouble());
					break;
				case FieldDescriptor::CPPTYPE_INT32:
					ref->AddInt32(msg, field, json[key][j].asInt());
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					ref->AddUInt32(msg, field, json[key][j].asUInt());
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					ref->AddBool(msg, field, json[key][j].asBool());
					break;
				case FieldDescriptor::CPPTYPE_STRING:
					ref->AddString(msg, field, json[key][j].asString());
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					if(!_j2p(ref->AddMessage(msg, field), json[key][j]))
						return false;
					break;
				default:
					break;
				}
			}
		}
		else if (json.isMember(key))
		{
			switch (field->cpp_type())
			{
			case FieldDescriptor::CPPTYPE_DOUBLE:
				ref->SetDouble(msg, field, json[key].asDouble());
				break;
			case FieldDescriptor::CPPTYPE_FLOAT:
				ref->SetFloat(msg, field, (float)json[key].asDouble());
				break;
			case FieldDescriptor::CPPTYPE_INT64:
				ref->SetInt64(msg, field, (int64_t)json[key].asDouble());
				break;
			case FieldDescriptor::CPPTYPE_UINT64:
				ref->SetUInt64(msg, field, (uint64_t)json[key].asDouble());
				break;
			case FieldDescriptor::CPPTYPE_INT32:
				ref->SetInt32(msg, field, json[key].asInt());
				break;
			case FieldDescriptor::CPPTYPE_UINT32:
				ref->SetUInt32(msg, field, json[key].asUInt());
				break;
			case FieldDescriptor::CPPTYPE_BOOL:
				ref->SetBool(msg, field, json[key].asBool());
				break;
			case FieldDescriptor::CPPTYPE_STRING:
				ref->SetString(msg, field, json[key].asString());
				break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
				if(!_j2p(ref->MutableMessage(msg, field), json[key]))
					return false;
				break;
			default:
				break;
			}
		}
	}

	return true;
}

void ConfigPBBase::Fail()
{
	ConfigManager::Fail();
}
