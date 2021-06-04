/*
 * DBCWrapTemplate.h
 *
 *  Created on: 2017-5-5
 *      Author: dawx62fac
 */

#ifndef DBCWRAPTEMPLATE_H_
#define DBCWRAPTEMPLATE_H_

template<class _DBC, class _D_MGR>
class DBCWrapTemplate
{
public:
	DBCWrapTemplate(int idx)
		: container_(_D_MGR::Instance())
		, index_(idx)
		, data_(container_->m_data->data[index_])
	{

	}

	void Save()
	{
		container_->m_data->MarkChange(index_);
	}

	_DBC& Obj()
	{
		return data_;
	}

	const _DBC& Obj() const
	{
		return data_;
	}

	virtual ~DBCWrapTemplate() {}
protected:
	_D_MGR* container_;
	int		index_;
	_DBC&	data_;
};




#endif /* DBCWRAPTEMPLATE_H_ */
