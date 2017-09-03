#include "CommandWindow.h"

namespace winui {

CommandWindow::CommandWindow(const std::string& title, const Dimension& screen, Image charset, const Dimension& charsize, uint ncolors)
	: window_(title, Space(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.w * charsize.w, screen.h * charsize.h)), charset_(charset)
{
	run_ = [](const std::string&) { };
	prompt_ = "";
	color_ = 0;
	nColors_ = ncolors;
	screen_ = screen;
	charsize_ = charsize;
	cursor_ = Position(0, 0);
	blink_ = 0;
	blinkSpeed_ = 1000;
	offBuf_ = offCmd_ = offPrev_ = 0;

	window_.onRender([this]( ) { onRender(); });
	window_.onUpdate([this](uint ms) { onUpdate(ms); });
	window_.onEvent([this](const SDL_Event& e) { onEvent(e); });
}

void CommandWindow::println(const std::string& line)
{
	std::string s(line);

	while(true)
	{
		buffer_.push_back(s.substr(0, screen_.w));

		if(s.size() < screen_.w) break;

		s = s.substr(screen_.w, s.size() - screen_.w);
	}
}

void CommandWindow::onRender(void)
{
	window_.clear();

	uint pl = prompt_.size() + cmd_.size() + 1;
	uint o = 0, i;

	pl = (pl + screen_.w - 1) / screen_.w;

	if(buffer_.size() > screen_.h - pl)
	{
		o = buffer_.size() - (screen_.h - pl);
		o = o < offBuf_ ? 0 : o - offBuf_;
	}

	for(i = 0 ; i < screen_.h ; ++i)
	{
		if(o + i >= buffer_.size()) break;

		renderString(i, buffer_.at(o + i));
	}

	if(i < screen_.h)
	{
		uint co = prompt_.size() + offCmd_;
		std::string s = prompt_ + cmd_;
		bool blinked_ = false;

		while(true)
		{
			if(s.size() > 0)
			{
				renderString(i, s.substr(0, screen_.w));
			}

			if(!blinked_ && co < screen_.w)
			{
				char ch = ' ';

				if(co < s.size())
				{
					ch = s.at(co);
				}

				renderChar(co, i, ch, isBlinking());
				blinked_ = true;
			}
			
			if(++i == screen_.h) break;
			if(s.size() < screen_.w) break;

			s = s.substr(screen_.w, s.size() - screen_.w);
			if(co >= screen_.w) co -= screen_.w;
		}
	}
}

void CommandWindow::onUpdate(uint ms)
{
	blink_ = (blink_ + ms) % blinkSpeed_;
}

void CommandWindow::onEvent(const SDL_Event& e)
{
	switch(e.type)
	{
		case SDL_WINDOWEVENT:
			switch(e.window.event)
			{
			}
			break;
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym)
			{
				case SDLK_LEFT:
					if(offCmd_ > 0) --offCmd_;
					offBuf_ = 0;
					break;
				case SDLK_RIGHT:
					if(offCmd_ < cmd_.size()) ++offCmd_;
					offBuf_ = 0;
					break;
				case SDLK_UP:
					if(prev_.size() > offPrev_)
					{
						if(offPrev_ == 0) tmp_ = cmd_;
						cmd_ = prev_.at(offPrev_++);
						if(offCmd_ > cmd_.size()) offCmd_ = cmd_.size();
					}
					offBuf_ = 0;
					break;
				case SDLK_DOWN:
					if(offPrev_ > 0)
					{
						if(--offPrev_ == 0) cmd_ = tmp_;
						else cmd_ = prev_.at(offPrev_ - 1);
						if(offCmd_ > cmd_.size()) offCmd_ = cmd_.size();
					}
					offBuf_ = 0;
					break;
				case SDLK_BACKSPACE:
					if(cmd_.size() > 0 && offCmd_ > 0)
					{
						cmd_.erase(--offCmd_, 1);
					}
					offBuf_ = 0;
					break;
				case SDLK_RETURN:
					if(!cmd_.empty())
					{
						println(prompt_ + cmd_);
						prev_.push_front(cmd_);
						run_(cmd_);
						tmp_ = "";
						cmd_ = "";
						offBuf_ = offCmd_ = offPrev_ = 0;
					}
					break;
				case SDLK_HOME:
					offCmd_ = 0;
					break;
				case SDLK_END:
					offCmd_ = cmd_.size();
					break;
			}
			break;
		case SDL_TEXTINPUT:
			cmd_.insert(offCmd_++, std::string(e.text.text));
			offBuf_ = 0;
			break;
	}
}

void CommandWindow::input(char c)
{
	cmd_.insert(offCmd_++, &c, 1);
}

void CommandWindow::renderChar(uint x, uint y, char c, bool inv)
{
	Space s(((uint) c) * charsize_.w, (color_ + (inv ? nColors_ : 0)) * charsize_.h, charsize_.w, charsize_.h);

	window_.draw(charset_.region(s), Position(x * charsize_.w, y * charsize_.h));
}

void CommandWindow::renderString(uint l, const std::string& s)
{
	if(l >= screen_.h) return;

	for(uint i = 0 ; i < screen_.w ; ++i)
	{
		if(i >= s.size()) break;

		renderChar(i, l, s.at(i), false);
	}
}

}

