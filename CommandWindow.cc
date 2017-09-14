#include "CommandWindow.h"

namespace winui {

CommandWindow::CommandWindow(const std::string& title, const Dimension& screen, Image charset, const Dimension& charsize, uint ncolors)
	: CharacterWindow(title, Position::CENTER(), screen, charset, charsize, ncolors)
{
	run_ = [](const std::string&) { };
	prompt_ = "";
	offBuf_ = offCmd_ = offPrev_ = 0;

	enableBlink(true);
}

void CommandWindow::println(const std::string& line)
{
	std::string s = line;
	Dimension screen = getSize();

	while(true)
	{
		buffer_.push_back(s.substr(0, screen.w));

		if(s.size() < screen.w) break;

		s = s.substr(screen.w, s.size() - screen.w);
	}
}

void CommandWindow::onRender(void)
{
	clear();

	renderCursor();

	uint pl = prompt_.size() + cmd_.size() + 1;
	uint o = 0, i;
	Dimension screen = getSize();

	pl = (pl + screen.w - 1) / screen.w;

	if(buffer_.size() > screen.h - pl)
	{
		o = buffer_.size() - (screen.h - pl);
		o = o < offBuf_ ? 0 : o - offBuf_;
	}

	for(i = 0 ; i < screen.h ; ++i)
	{
		if(o + i >= buffer_.size()) break;

		renderString(Position(0, i), buffer_.at(o + i));
	}

	if(i < screen.h)
	{
		uint co = prompt_.size() + offCmd_;
		std::string s = prompt_ + cmd_;

		while(true)
		{
			if(s.size() > 0)
			{
				renderString(Position(0, i), s.substr(0, screen.w));
			}

			if(++i == screen.h) break;
			if(s.size() < screen.w) break;

			s = s.substr(screen.w, s.size() - screen.w);
			if(co >= screen.w) co -= screen.w;
		}
	}
}

void CommandWindow::onUpdate(uint ms)
{
	CharacterWindow::onUpdate(ms);
}

void CommandWindow::onEvent(const SDL_Event& e)
{
	if(hasFocus()) switch(e.type)
	{
		case SDL_WINDOWEVENT:
			if(e.window.windowID == getID()) switch(e.window.event)
			{
				case SDL_WINDOWEVENT_CLOSE:
					onClose_();
					break;
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

	Dimension screen = getSize();
	uint x = prompt_.size() + offCmd_;
	uint dy = (x + screen.w - 1) / screen.w;
	uint y = buffer_.size() + dy - 1;
	updateCursor(Position(x % screen.w, y >= screen.h ? screen.h - dy + (x / screen.w) : y));
}

void CommandWindow::input(char c)
{
	cmd_.insert(offCmd_++, &c, 1);
}

}

