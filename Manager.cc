#include <iostream>

#include "Manager.h"

namespace winui {

Manager& Manager::instance(void)
{
	static Manager theManager;
	return theManager;
}

Manager::Manager(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::string("ERR: initializing sdl. ") + SDL_GetError();
	}

	SDL_StartTextInput();

	running_ = true;
}

Manager::~Manager(void)
{
	SDL_Quit();
}

void Manager::tick(void)
{
	SDL_Event e;

	for(auto& p : windows_)
	{
		p.second->update();
	}

	while(SDL_PollEvent(&e) != 0)
	{
		if(e.type == SDL_QUIT)
		{
			running_ = false;
		}
		else if(e.type == SDL_WINDOWEVENT)
		{
			int id = e.window.windowID;
			auto i = windows_.find(id);

			if(i == windows_.end())
				throw std::string("ERR: window didn't register!");

			i->second->handle(e);
		}
		else
		{
			for(auto& p : windows_)
			{
				if(p.second->hasFocus())
				{
					p.second->handle(e);
					break;
				}
			}
		}
	}

	bool allClosed = true;
	for(auto& p : windows_)
	{
		if(!p.second->isHidden())
		{
			allClosed = false;
			break;
		}
	}

	if(allClosed) running_ = false;
}

void Manager::render(void)
{
	for(const auto& p : windows_)
	{
		p.second->render();
	}
}

void Manager::registerWindow(int id, Window& w)
{
	if(windows_.count(id) > 0)
		throw std::string("ERR: window already exists!");

	windows_[id] = &w;
}

void Manager::unregisterWindow(int id)
{
	auto i = windows_.find(id);

	if(i == windows_.end())
		throw std::string("ERR: no suck window!");
	
	windows_.erase(i);
}

SDL_Surface *Manager::loadImage(const std::string& path)
{
	auto i = images_.find(path);

	if(i == images_.end())
	{
		if((images_[path] = SDL_LoadBMP(path.c_str())) == nullptr)
		{
			throw "ERR: could not load image '" + path + "'!";
		}

		i = images_.find(path);
		r_images_[i->second] = path;
	}

	++imgCounts_[i->second];

	return i->second;
}

void Manager::unloadImage(SDL_Surface *img)
{
	if(imgCounts_[img] <= 0)
		throw std::string("ERR: unknown image!");
	
	if(--imgCounts_[img] == 0)
	{
		auto i = r_images_.find(img);

		images_.erase(images_.find(i->second));
		r_images_.erase(i);

		SDL_FreeSurface(img);
	}
}

}

