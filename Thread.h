#ifndef LIB_THREAD_H
#define LIB_THREAD_H

#include <functional>

#include <SDL_thread.h>

namespace lib
{
	class Lock
	{
		public:
			Lock(SDL_mutex *mtx) : mtx_(mtx) { SDL_LockMutex(mtx_); }
			~Lock( ) { SDL_UnlockMutex(mtx_); }
		private:
			SDL_mutex *mtx_;
	};

	class Thread
	{
		public:
		typedef std::function<void(void)> thread_fn;

		public:
			Thread(thread_fn);
			void join( );
		private:
			static int threadHandler(void *);

		private:
			SDL_Thread *thread_;
			thread_fn f_;
	};
}

#endif

