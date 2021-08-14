#pragma once

#ifndef COMPILERTHREADJOB_H
#define COMPILERTHREADJOB_H

#include <future>
#include <memory>
#include <string>

#include "encoding/buffer.h"
#include "GS2Context.h"

/////// This class will allow you to queue lambda functions into a threadpool

class CallbackThreadJob
{
public:
	struct job_result {
		Buffer buf;
	};

	struct thread_context {
		GS2Context gs2context;
	};

	using future_type = std::future<job_result>;
	using promise_type = std::promise<job_result>;
	using callback_type = std::function<void(thread_context&, promise_type&)>;

public:
	CallbackThreadJob(callback_type s)
		: _fn(std::move(s))
	{
	}
	
	void run(thread_context& th_context, promise_type& promise)
	{
		_fn(th_context, promise);
	}

	static void init(thread_context& th_context)
	{

	}

private:
	callback_type _fn;
};

class CompilerThreadJob
{
public:
	struct job_result {
		Buffer buf;
	};

	struct thread_context {
		GS2Context gs2context;
	};

	using promise_type = std::promise<job_result>;

public:
	CompilerThreadJob(std::string s)
		: _src(std::make_shared<std::string>(s))
	{
	}

	void run(thread_context& th_context, promise_type& promise)
	{
		Buffer buf = th_context.gs2context.compile(*_src.get(), "weapon", "TestCode", true);
		promise.set_value({ std::move(buf) });
	}

	static void init(thread_context& th_context)
	{

	}

private:
	std::shared_ptr<std::string> _src;
};

#endif
