#pragma once

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <functional>

enum class PayloadType : uint16_t
{
	DEFAULT
};

template<class Payload, typename TypeCls = PayloadType>
class EventHandler
{
	using cb_catchall_type = std::function<void(Payload&)>;
	using cb_type = std::function<void(const Payload&)>;

	struct cb_data {
		TypeCls type;
		cb_type func;

		void operator()(const Payload& data) const {
			func(data);
		}
	};

public:
	EventHandler() = default;

	EventHandler(cb_catchall_type cb)
	{
		setCatchAll(cb);
	}

	void addCallback(TypeCls type, cb_type cb)
	{
		if (cb)
		{
			_callbacks.push_back({ type, std::move(cb) });
		}
	}

	void setCatchAll(cb_catchall_type cb)
	{
		if (cb)
		{
			_catchall = std::move(cb);
		}
	}

	void submitPayload(TypeCls type, Payload data)
	{
		for (const auto& callback : _callbacks)
		{
			if (callback.type == type)
				callback(data);
		}

		if (_catchall)
		{
			_catchall(data);
		}
	}

	void addCallback(cb_type cb)
	{
		if (cb)
		{
			if constexpr (std::is_same<TypeCls, PayloadType>::value)
			{
				addCallback(PayloadType::DEFAULT, std::move(cb));
			}
			else
			{
				addCallback(TypeCls(0), std::move(cb));
			}
		}
	}

	void submitPayload(Payload data)
	{
		if constexpr (std::is_same<TypeCls, PayloadType>::value)
		{
			submitPayload(PayloadType::DEFAULT, std::move(data));
		}
		else
		{
			submitPayload(TypeCls(0), std::move(data));
		}
	}

	/*
	template<class Cls = TypeCls>
	typename std::enable_if<std::is_same<Cls, PayloadType>::value>::type addCallback(cb_type cb)
	{
		addCallback(PayloadType::DEFAULT, std::move(cb));
	}

	template<class Cls = TypeCls>
	typename std::enable_if<std::is_same<Cls, PayloadType>::value>::type submitPayload(Payload data)
	{
		submitPayload(PayloadType::DEFAULT, std::move(data));
	}
	*/

private:
	cb_catchall_type _catchall;
	std::vector<cb_data> _callbacks;
};

#endif
