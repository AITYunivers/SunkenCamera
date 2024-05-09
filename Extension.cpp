#include "Common.hpp"

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#ifdef _WIN32
Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this), FusionDebugger(this)
#elif defined(__ANDROID__)
Extension::Extension(const EDITDATA* const edPtr, const jobject javaExtPtr) :
	javaExtPtr(javaExtPtr, "Extension::javaExtPtr from Extension ctor"),
	Runtime(this, this->javaExtPtr), FusionDebugger(this)
#else
Extension::Extension(const EDITDATA* const edPtr, void* const objCExtPtr) :
	objCExtPtr(objCExtPtr), Runtime(this, objCExtPtr), FusionDebugger(this)
#endif
{
	/*
		Link all your action/condition/expression functions to their IDs to match the
		IDs in the JSON here
	*/

	LinkAction(0, SetDivisor);
	LinkAction(1, SetMargin);
	LinkAction(2, SetFactor);

	LinkAction(3, SetDisallowScrolling);
	LinkAction(8, SetCenterDisplay);
	LinkAction(4, SetEasing);
	LinkAction(5, SetHoriScrolling);
	LinkAction(6, SetVertScrolling);
	LinkAction(7, SetPeytonphile);


	LinkCondition(0, CheckDisallowScrolling);
	LinkCondition(5, CheckCenterDisplay);
	LinkCondition(1, CheckEasing);
	LinkCondition(2, CheckHoriScrolling);
	LinkCondition(3, CheckVertScrolling);
	LinkCondition(4, CheckPeytonphile);


	LinkExpression(0, GetDivisor);
	LinkExpression(1, GetMargin);
	LinkExpression(2, GetFactor);

	LinkExpression(3, GetXScroll);
	LinkExpression(4, GetYScroll);
	LinkExpression(5, GetXScrollTarget);
	LinkExpression(6, GetYScrollTarget);
	LinkExpression(7, GetXSpeed);
	LinkExpression(8, GetYSpeed);

	/*
		This is where you'd do anything you'd do in CreateRunObject in the original SDK

		It's the only place you'll get access to edPtr at runtime, so you should transfer
		anything from edPtr to the extension class here.

	*/
#ifdef EditorBuild
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Divisor: ") + std::to_tstring(ext->Divisor);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Margin: ") + std::to_tstring(ext->Margin);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Factor: ") + std::to_tstring(ext->Factor);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Center Display: ") + std::to_tstring(ext->CenterDisplay);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Easing: ") + std::to_tstring(ext->Easing);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Horizontal Scrolling: ") + std::to_tstring(ext->HoriScrolling);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Vertical Scrolling: ") + std::to_tstring(ext->VertScrolling);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Peytonphile Scrolling: ") + std::to_tstring(ext->Peytonphile);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("X Scroll: ") + std::to_tstring(ext->_scrollingX);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Y Scroll: ") + std::to_tstring(ext->_scrollingY);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("X Speed: ") + std::to_tstring(ext->_xSpeed);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Y Speed: ") + std::to_tstring(ext->_ySpeed);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = _T("Disallow Scrolling: ") + std::to_tstring(ext->_dontScroll);
		},
		NULL, 10, NULL
	);
#endif

	// Read object DarkEdif properties; you can pass property name, or property index
	// This will work on all platforms the same way.
	Divisor = edPtr->Props.GetPropertyNum("Divisor"sv);
	Margin = edPtr->Props.GetPropertyNum("Margin"sv);
	Factor = Clamp(edPtr->Props.GetPropertyNum("Factor"sv), 0, 100);

	CenterDisplay = edPtr->Props.IsPropChecked("Auto Center Display"sv);
	Easing = edPtr->Props.IsPropChecked("Easing"sv);
	HoriScrolling = edPtr->Props.IsPropChecked("Horizontal Scrolling"sv);
	VertScrolling = edPtr->Props.IsPropChecked("Vertical Scrolling"sv);
	Peytonphile = edPtr->Props.IsPropChecked("Peytonphile Scrolling"sv);

	_resX = GetFrameRight() - GetFrameLeft();
	_resY = GetFrameBottom() - GetFrameTop();

	_scrollingX = GetVirtualWidth() / 2;
	_scrollingXTarget = _scrollingX;
	if (CenterDisplay)
		SetFrameCenterX(_scrollingX);

	_scrollingY = GetVirtualHeight() / 2;
	_scrollingYTarget = _scrollingY;
	if (CenterDisplay)
		SetFrameCenterY(_scrollingY);
}

Extension::~Extension()
{
	
}


REFLAG Extension::Handle()
{
	_resX = GetFrameRight() - GetFrameLeft();
	_resY = GetFrameBottom() - GetFrameTop();

	_dt = GetDelta();

	if (!Peytonphile)
	{
		_marginMiddleX = Clamp(Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft(), ((_resX / 2) - (Margin / 2)), ((_resX / 2) + (Margin / 2)));
		_marginMiddleY = Clamp(Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop(), ((_resY / 2) - (Margin / 2)), ((_resY / 2) + (Margin / 2)));
		_xSpeed = 0;
		_ySpeed = 0;
	}

	if (!_dontScroll)
		_changed = false;

	if (!_dontScroll && HoriScrolling && !Peytonphile)
		_xSpeed = ((Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft()) - _marginMiddleX) / Divisor;

	if (!_dontScroll && VertScrolling && !Peytonphile)
		_ySpeed = ((Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop()) - _marginMiddleY) / (Divisor + 0.0f) * ((_resX + 0.0f) / _resY);

	if (!Peytonphile && HoriScrolling)
		_scrollingXTarget = Clamp((_scrollingXTarget + (_xSpeed * _dt)), (_resX / 2), (GetVirtualWidth() - (_resX / 2)));

	if (!Peytonphile && VertScrolling)
		_scrollingYTarget = Clamp((_scrollingYTarget + (_ySpeed * _dt)), (_resY / 2), (GetVirtualHeight() - (_resY / 2)));

	if (!Easing && HoriScrolling && !_dontScroll)
	{
		_scrollingX = _scrollingXTarget;
		if (CenterDisplay)
			SetFrameCenterX(_scrollingX);
	}

	if (!Easing && VertScrolling && !_dontScroll)
	{
		_scrollingY = _scrollingYTarget;
		if (CenterDisplay)
			SetFrameCenterY(_scrollingY);
	}

	if (Easing && HoriScrolling && !_dontScroll)
	{
		_scrollingX = _scrollingX + (_scrollingXTarget - _scrollingX) * ((Factor / 100.0f) * _dt);
		if (CenterDisplay)
			SetFrameCenterX(_scrollingX);
	}

	if (Easing && VertScrolling && !_dontScroll)
	{
		_scrollingY = _scrollingY + (_scrollingYTarget - _scrollingY) * ((Factor / 100.0f) * _dt);
		if (CenterDisplay)
			SetFrameCenterY(_scrollingY);
	}

	if (Easing && _dontScroll)
	{
		if (!_savedLast)
		{
			_lastX = GetFrameLeft() + (_resX / 2);
			_lastY = GetFrameTop() + (_resY / 2);
		}
		_savedLast = true;
	}
	else
		_savedLast = false;

	if (Easing && _dontScroll && !_changed)
	{
		if (GetFrameLeft() + (_resX / 2) != _lastX || GetFrameTop() + (_resY / 2) != _lastY)
			_changed = true;
		else
		{
			if (HoriScrolling)
			{
				_scrollingX = _scrollingX + (_scrollingXTarget - _scrollingX) * ((Factor / 100.0f) * _dt);
				if (CenterDisplay)
					SetFrameCenterX(_scrollingX);
			}

			if (VertScrolling)
			{
				_scrollingY = _scrollingY + (_scrollingYTarget - _scrollingY) * ((Factor / 100.0f) * _dt);
				if (CenterDisplay)
					SetFrameCenterY(_scrollingY);
			}

			_lastX = GetFrameLeft() + (_resX / 2);
			_lastY = GetFrameBottom() + (_resY / 2);
		}
	}

	if (Peytonphile)
	{
		_xSpeed = 0;
		_ySpeed = 0;

		if (!_dontScroll && HoriScrolling)
			_scrollingXTarget = Clamp((GetVirtualWidth() / 2) + (((GetMouseX() - ((GetVirtualWidth()) / 2))) * ((GetVirtualWidth() - (_resX + 0.0)) / GetVirtualWidth())), (_resX / 2), GetVirtualWidth() - (_resX / 2));

		if (!_dontScroll && VertScrolling)
			_scrollingYTarget = Clamp((GetVirtualHeight() / 2) + (((GetMouseY() - ((GetVirtualHeight()) / 2))) * ((GetVirtualHeight() - (_resY + 0.0)) / GetVirtualHeight())), (_resY / 2), GetVirtualHeight() - (_resY / 2));
	}

	return REFLAG::NONE;
}

REFLAG Extension::Display()
{
	/*
		If you return REFLAG_DISPLAY in Handle() this routine will run.
	*/

	// Ok
	return REFLAG::DISPLAY;
}

short Extension::FusionRuntimePaused()
{

	// Ok
	return 0;
}

short Extension::FusionRuntimeContinued()
{

	// Ok
	return 0;
}


// These are called if there's no function linked to an ID

void Extension::UnlinkedAction(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedAction() called"), _T("Running a fallback for action ID %d. Make sure you ran LinkAction()."), ID);
}

long Extension::UnlinkedCondition(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedCondition() called"), _T("Running a fallback for condition ID %d. Make sure you ran LinkCondition()."), ID);
	return 0;
}

long Extension::UnlinkedExpression(int ID)
{
	DarkEdif::MsgBox::Error(_T("Extension::UnlinkedExpression() called"), _T("Running a fallback for expression ID %d. Make sure you ran LinkExpression()."), ID);
	// Unlinked A/C/E is fatal error, but try not to return null string and definitely crash it
	if ((size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}

int Extension::GetFrameRight()
{
#ifdef _WIN32
	int r = rhPtr->WindowX;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayX;
	r += rhPtr->rh3.WindowSx;
	if (r > rhPtr->LevelSx)
		r = rhPtr->LevelSx;
	return r;
#endif
}

int Extension::GetFrameLeft()
{
#ifdef _WIN32
	int r = rhPtr->WindowX;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayX;
	if (r < 0)
		r = 0;
	return r;
#endif
}

int Extension::GetFrameBottom()
{
#ifdef _WIN32
	int r = rhPtr->WindowY;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayY;
	r += rhPtr->rh3.WindowSy;
	if (r > rhPtr->LevelSy)
		r = rhPtr->LevelSy;
	return r;
#endif
}

int Extension::GetFrameTop()
{
#ifdef _WIN32
	int r = rhPtr->WindowY;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayY;
	if (r < 0)
		r = 0;
	return r;
#endif
}

void Extension::SetFrameCenterX(int centerX)
{
#ifdef _WIN32
	centerX = Clamp(centerX - rhPtr->rh3.WindowSx / 2, 0, GetVirtualWidth() - rhPtr->rh3.WindowSx);
	rhPtr->rh3.DisplayX = centerX;
	Runtime.Redisplay();
#endif
}

void Extension::SetFrameCenterY(int centerY)
{
#ifdef _WIN32
	centerY = Clamp(centerY - rhPtr->rh3.WindowSy / 2, 0, GetVirtualHeight() - rhPtr->rh3.WindowSy);
	rhPtr->rh3.DisplayY = centerY;
	Runtime.Redisplay();
#endif
}

double Extension::Clamp(double value, double min, double max)
{
	if (value < min)
		value = min;
	if (value > max)
		value = max;
	return value;
}

int Extension::GetMouseX()
{
#ifdef _WIN32
	return rhPtr->rh2.Mouse.x;
#endif
}

int Extension::GetMouseY()
{
#ifdef _WIN32
	return rhPtr->rh2.Mouse.y;
#endif
}

double Extension::GetDelta()
{
#ifdef _WIN32
	return rhPtr->rh4.mvtTimerCoef;
#endif
}

int Extension::GetVirtualWidth()
{
#ifdef _WIN32
	return rhPtr->Frame->VirtualRect.right;
#endif
}

int Extension::GetVirtualHeight()
{
#ifdef _WIN32
	return rhPtr->Frame->VirtualRect.bottom;
#endif
}
