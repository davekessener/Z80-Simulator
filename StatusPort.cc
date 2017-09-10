#include "StatusPort.h"

namespace z80 {

void StatusPort::reset(void)
{
	queue_.clear();
	idRead_ = 0;
}

void StatusPort::write(uint8_t port, uint8_t data)
{
}

uint8_t StatusPort::read(uint8_t port)
{
	uint8_t r = 0;

	switch(port)
	{
		case 0x00: // control port
			break;
		case 0x01: // int port
			if(queue_.size() > 0)
			{
				r = queue_.front();
				queue_.pop_front();
			}
			break;
		case 0x0F: // periph-id port
			r = idRead_++ & 1 ? STATUS_ID : Peripheral::PER_ID;
			break;
	}

	return r;
}

void StatusPort::registerInt(uint8_t id, int_t i)
{
	i.listen([this, id, i](bool& v)
	{
		if(v)
		{
			int_t reset(i);
			queue_.push_back(id);
			reset.set(false);
			onInt_();
		}
	});
}

}

