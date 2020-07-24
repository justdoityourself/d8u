/* Copyright (C) 2020 D8DATAWORKS - All Rights Reserved */

#pragma once

#include <array>
#include <mutex>
#include <queue>

#include <thread>

namespace d8u
{
	namespace async
	{
		template <typename T> class Queue // MCMP
		{
		public:
			Queue() {}

			void Push(T&& e,size_t max = 0)
			{
				while(max && q.size() >= max)
					std::this_thread::sleep_for(std::chrono::milliseconds(100)); //Cool down

				std::unique_lock<std::mutex> lck(mtx);

				_total++;
				q.push(std::move(e));
				cv.notify_all();//cv.notify_one();
			}

			bool Try(T& e)
			{
				std::unique_lock<std::mutex> lck(mtx);

				if (!q.empty())
				{
					e = std::move(q.front());
					q.pop();
					return true;
				}

				return false;
			}

			bool TryWait(T& r, size_t count=1000)
			{
				std::unique_lock<std::mutex> lck(mtx);

				if (cv.wait_for(lck, std::chrono::milliseconds(count), [&] {return !q.empty(); }))
				{
					r = std::move(q.front());
					q.pop();
					return true;
				}

				return false;
			}

			bool Next(const bool& run, T& r)
			{
				std::unique_lock<std::mutex> lck(mtx);
				while (run)
				{
					if (cv.wait_for(lck, std::chrono::milliseconds(1000), [&] {return !q.empty(); }))
					{
						r = std::move(q.front());
						q.pop();
						return true;
					}
				}

				if (cv.wait_for(lck, std::chrono::milliseconds(1000), [&] {return !q.empty(); }))
				{
					r = std::move(q.front());
					q.pop();
					return true;
				}

				return false;
			}

			size_t size() const
			{
				return q.size();
			}

			size_t total () const
			{
				return _total;
			}
		private:
			std::queue<T> q;
			size_t _total = 0;

			std::mutex mtx;
			std::condition_variable cv;
		};

		template < typename P, size_t max = 10> class Pipeline
		{
			size_t dx = 0;
			std::array<bool, max> running = {};
			std::array<Queue<P>,max> queues;
			std::list<std::thread> threads;

		public:

			Pipeline() { }

			void Print()
			{
				for (auto& q : queues)
					std::cout << q.size() << " ";
			}

			void Total()
			{
				for (auto& q : queues)
					std::cout << q.total() << " ";
			}

			void Online()
			{
				for (auto& b : running)
					std::cout << ((b) ? "true" : "false") << " ";
			}

			~Pipeline()
			{
				for (auto& thread : threads)
					thread.join();
			}

			const bool& Running()
			{
				return running[dx - 1];
			}

			void Push(P&& e, size_t dx = 0)
			{
				queues[dx].Push(std::move(e));
			}

			bool Try(P& e)
			{
				return queues.back.Try(e);
			}

			bool Next(bool& run, P& r)
			{
				return queues.back.Next(run, r);
			}

			template < typename F > void Start(F f, size_t count=1)
			{
				running[dx] = true;
				auto* prev = &queues[dx++];
				auto* next = &queues[dx];

				for (size_t i = 0; i < count; i++)
					threads.emplace_back([&running = running,f, prev, next,dx=dx-1]()
					{ 
						f(*prev, *next); 

						running[dx] = false;
					});
			}

			template < typename F > void Stream(F f, size_t count = 1)
			{
				running[dx] = true;
				auto* prev = &queues[dx++];
				auto* next = &queues[dx];

				for (size_t i = 0; i < count; i++)
					threads.emplace_back([&running = running,f, prev, next, dx = dx-1]()
					{
						P packet;
						while (prev->Next(running[dx - 1], packet))
						{
							if (!f(std::move(packet), *next))
								break;
						}				

						running[dx] = false;
					});
			}

		private:
		}; 
	}
}