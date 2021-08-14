#pragma once

#ifndef CONTEXTTHREADPOOL_H
#define CONTEXTTHREADPOOL_H

#include <future>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

template<typename JobCls>
class CustomThreadPool
{
	using thread_context = typename JobCls::thread_context;
	using result_type = typename JobCls::job_result;

	using future_type = std::future<result_type>;
	using promise_type = std::promise<result_type>;
	using queue_type = std::pair<JobCls, promise_type>;

public:
	CustomThreadPool(int count)
		: _running(false)
	{
		start(count);
	}

	~CustomThreadPool()
	{
		stop();
	}

	bool isRunning() const
	{
		return _running.load();
	}

	void start(int count)
	{
		if (!isRunning())
		{
			_running = true;

			for (int i = 0; i < count; i++)
			{
				_workers.push_back(std::thread(&CustomThreadPool::workerThread, this));
			}
		}
	}

	void stop()
	{
		if (isRunning())
		{
			{
				std::scoped_lock lock(_queueLock);
				_running = false;
				_queueCond.notify_all();
			}

			for (auto& worker : _workers)
				worker.join();
			_workers.clear();
		}
	}

	/// <summary>
	/// Clear the queue of any pending jobs
	/// </summary>
	void clear()
	{
		if (isRunning())
		{
			std::queue<queue_type> temp_queue;

			{
				std::scoped_lock lock(_queueLock);
				_queue.swap(temp_queue);
				_queueCond.notify_all();
			}
		}
	}

	/// <summary>
	/// Queue a job into the threadpool, returning a future to the
	/// result object defined in the JobCls
	/// </summary>
	/// <param name="job"></param>
	/// <returns></returns>
	future_type queue(JobCls job)
	{
		queue_type job_pair({ std::move(job), promise_type() });
		future_type future = job_pair.second.get_future();

		{
			std::scoped_lock lock(_queueLock);
			_queue.push(std::move(job_pair));
			_queueCond.notify_one();
		}

		return future;
	}

	std::vector<future_type> queue(const std::vector<JobCls>& jobs)
	{
		std::vector<future_type> futureList;

		{
			std::scoped_lock lock(_queueLock);
			for (const auto& job : jobs)
			{
				queue_type job_pair = { job, promise_type() };
				futureList.push_back(job_pair.second.get_future());
				_queue.push(std::move(job_pair));
			}

			_queueCond.notify_all();
		}

		return std::move(futureList);
	}

private:
	std::atomic<bool> _running;
	std::vector<std::thread> _workers;

	std::queue<queue_type> _queue;
	std::mutex _queueLock;
	std::condition_variable _queueCond;

	void workerThread()
	{
		thread_context context{};
		JobCls::init(context);

		while (isRunning())
		{
			std::unique_lock lock(_queueLock);
			_queueCond.wait(lock, [this] {
				return !_queue.empty() || !_running;
			});

			if (isRunning())
			{
				queue_type job = std::move(_queue.front());
				_queue.pop();
				lock.unlock();

				job.first.run(context, job.second);
			}
		}
	}
};


#endif
