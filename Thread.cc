#include "Thread.h"

namespace lib {

Thread::Thread(thread_fn f)
	: f_(f)
{
	if((thread_ = SDL_CreateThread(&Thread::threadHandler, "", this)) == nullptr)
	{
		throw std::string("ERR: couldn't create thread! ") + SDL_GetError();
	}
}

void Thread::join(void)
{
	SDL_WaitThread(thread_, nullptr);
}

int Thread::threadHandler(void *pThread)
{
	Thread& that(*static_cast<Thread *>(pThread));

	that.f_();

	return 0;
}

}

