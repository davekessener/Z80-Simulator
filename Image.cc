#include "Image.h"
#include "Manager.h"

namespace winui {

Image::Image(const std::string& path)
	: surface_(Manager::instance().loadImage(path)), c_(new uint), region_(0, 0)
{
	*c_ = 1;
	region_.dim = Dimension(surface_->w, surface_->h);
}

Image::Image(const Image& img)
	: surface_(img.surface_), c_(img.c_), region_(img.region_)
{
	++*c_;
}

Image::~Image(void)
{
	free();
}

Image& Image::operator=(const Image& img)
{
	free();

	surface_ = img.surface_;
	c_ = img.c_;

	++*c_;

	return *this;
}

void Image::free(void)
{
	if(--*c_ == 0)
	{
		delete c_;
		Manager::instance().unloadImage(surface_);

		c_ = nullptr;
		surface_ = nullptr;
	}
}

Image Image::region(const Space& s) const
{
	Image r(*this);

	r.region_ += s;

	return r;
}

void Image::blit(SDL_Surface *target, const Space& d)
{
	SDL_Rect src, dst;
	Space& s(region_);

	src.x = s.pos.x; src.y = s.pos.y;
	src.w = s.dim.w; src.h = s.dim.h;

	dst.x = d.pos.x; dst.y = d.pos.y;
	dst.w = d.dim.w; dst.h = d.dim.h;

	SDL_BlitScaled(surface_, &src, target, &dst);
}

}

