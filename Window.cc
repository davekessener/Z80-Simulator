#include "Window.h"
#include "Manager.h"
#include "lib.h"

namespace winui {

Window::Window(const std::string& title, const Space& s)
{
	Manager& manager = Manager::instance();

	if((window_ = SDL_CreateWindow(title.c_str(), s.pos.x, s.pos.y, s.dim.w, s.dim.h, SDL_WINDOW_SHOWN)) == nullptr)
		throw std::string("ERR: couldn't create window! ") + SDL_GetError();
	
	if((surface_ = SDL_GetWindowSurface(window_)) == nullptr)
		throw std::string("ERR: couldn't get window surface! ") + SDL_GetError();

	windowID_ = SDL_GetWindowID(window_);
	manager.registerWindow(windowID_, *this);

	hidden_ = false;
	focus_ = true;
	mouseOver_ = false;
	default_ = Color::WHITE();
	time_.reset();
	last_ = 0;
}

Window::~Window(void)
{
	Manager::instance().unregisterWindow(windowID_);
	SDL_FreeSurface(surface_);
	SDL_DestroyWindow(window_);
}

void Window::hide(void)
{
	SDL_HideWindow(window_);
}

void Window::show(void)
{
	SDL_ShowWindow(window_);
}

void Window::update(void)
{
	uint t = time_.elapsed().count() + last_;

	onUpdate(t / 1000);

	last_ = t % 1000;
}

void Window::render(void)
{
	onRender();

	SDL_UpdateWindowSurface(window_);
}

void Window::clear(void)
{
	SDL_FillRect(surface_, nullptr, SDL_MapRGB(surface_->format, default_.r, default_.g, default_.b));
}

void Window::setWindowIcon(Image i)
{
	SDL_SetWindowIcon(window_, i.getSurface());
}

void Window::handle(const SDL_Event& e)
{
	switch(e.type)
	{
		case SDL_WINDOWEVENT:
			if(e.window.windowID == windowID_) switch(e.window.event)
			{
				case SDL_WINDOWEVENT_SHOWN:
					hidden_ = false;
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					hidden_ = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					focus_ = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					focus_ = false;
					break;
				case SDL_WINDOWEVENT_CLOSE:
					hide();
					break;
				case SDL_WINDOWEVENT_ENTER:
					mouseOver_ = true;
//					SDL_RaiseWindow(window_);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					mouseOver_ = false;
					break;
			}
	}

	onEvent(e);
}

}

