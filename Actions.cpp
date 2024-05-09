#include "Common.hpp"

void Extension::SetDivisor(float divisor)
{
	Divisor = divisor;
}

void Extension::SetMargin(float margin)
{
	Margin = margin;
}

void Extension::SetFactor(float factor)
{
	Factor = Clamp(factor, 0, 100);
}

void Extension::SetDisallowScrolling(int setting)
{
	_dontScroll = setting != 0;
}

void Extension::SetCenterDisplay(int setting)
{
	CenterDisplay = setting != 0;
}

void Extension::SetHoriScrolling(int setting)
{
	HoriScrolling = setting != 0;
}

void Extension::SetVertScrolling(int setting)
{
	VertScrolling = setting != 0;
}

void Extension::SetEasing(int setting)
{
	Easing = setting != 0;
}

void Extension::SetPeytonphile(int setting)
{
	Peytonphile = setting != 0;
}
