/*
 * ProtobufDispatcher.h
 *
 *  Created on: 2016-8-20
 *      Author: Ralf
 */

#ifndef PROTOBUFDISPATCHER_H_
#define PROTOBUFDISPATCHER_H_

#include "Common.h"

/*
 * 回调基类
 */
class Callback {
public:
	virtual ~Callback() {
	}
	;
	virtual int onMessage(Message* message) const = 0;
};

/*
 * 回调模版
 * T是pb message的具体类型
 */
template<typename T>
class CallbackT: public Callback {
public:
	typedef int (*ProtobufMessageCallback)(T* message);

	CallbackT(const ProtobufMessageCallback& callback) :
			callback_(callback) {
	}

	virtual int onMessage(Message* message) const {
		T* t = dynamic_cast<T*>(message);
		return callback_(t);
	}

private:
	ProtobufMessageCallback callback_;
};

/*
 * 回调分发器
 * 在LogicManager中注册和回调
 */
class ProtobufDispatcher {
public:

	ProtobufDispatcher(int (*def)(Message* message)) :
			defaultCallback_(def) {
	}

	int onMessage(Message* message) const {
		CallbackMap::const_iterator it = callbacks_.find(
				message->GetDescriptor());
		if (it != callbacks_.end()) {
			return it->second->onMessage(message);
		} else {
			return defaultCallback_(message);
		}
	}

	template<typename T>
	void registerMessageCallback(
			const typename CallbackT<T>::ProtobufMessageCallback& callback) {
		CallbackT<T>* pd(new CallbackT<T>(callback));
		callbacks_[T::descriptor()] = pd;
	}

	typedef std::map<const Descriptor*, Callback*> CallbackMap;
	CallbackMap callbacks_;
	int (*defaultCallback_)(Message* message);
};

#endif /* PROTOBUFDISPATCHER_H_ */
