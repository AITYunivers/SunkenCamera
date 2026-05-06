#include "Common.hpp"
#include <iomanip>
using namespace std::chrono_literals;

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#ifdef _WIN32
Extension::Extension(RunObject* const _rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->get_rHo()->get_AdRunHeader()), Runtime(this), FusionDebugger(this)
#elif defined(__ANDROID__)
Extension::Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr) :
	javaExtPtr(javaExtPtr, "Extension::javaExtPtr from Extension ctor"),
	Runtime(this, this->javaExtPtr), FusionDebugger(this)
#else
Extension::Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr) :
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
	LinkAction(15, SetPeytonphileToEdges);

	LinkAction(9, SetCameraPosX);
	LinkAction(10, SetCameraPosY);
	LinkAction(11, SetCameraTargetX);
	LinkAction(12, SetCameraTargetY);

	LinkAction(13, FlipHorizontally);
	LinkAction(14, FlipVertically);


	LinkCondition(0, CheckDisallowScrolling);
	LinkCondition(5, CheckCenterDisplay);
	LinkCondition(1, CheckEasing);
	LinkCondition(2, CheckHoriScrolling);
	LinkCondition(3, CheckVertScrolling);
	LinkCondition(4, CheckPeytonphile);
	LinkCondition(8, CheckPeytonphileToEdges);
	LinkCondition(6, CheckHoriFlipped);
	LinkCondition(7, CheckVertFlipped);


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

	// Don't use "this" inside these lambda functions, always ext.
	// There can be nothing in the [] section of the lambda.
	// If you're not sure about lambdas, you can remove this debugger stuff without any side effects;
	// it's just an example of how to use the debugger. You can view it in Fusion itself to see.
#if defined(EditorBuild) && defined(_WIN32)
	FusionDebugger.AddItemToDebugger(
		_T("Divisor: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->Divisor);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Margin: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->Margin);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Factor: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->Factor);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Disallow Scrolling: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->DontScroll != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Center Display: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->CenterDisplay != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Easing: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->Easing != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Horizontal Scrolling: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->HoriScrolling != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Vertical Scrolling: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->VertScrolling != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Peytonphile Scrolling: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->Peytonphile != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Peytonphile Scrolling to Edges: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->PeytonphileToEdges != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Input Flipped Horizontally: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->HoriFlipped != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Input Flipped Vertically: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = ext->VertFlipped != 0 ? _T("True") : _T("False");
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("X Scroll: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->_scrollingX);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Y Scroll: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->_scrollingY);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("X Speed: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->_xSpeed);
		},
		NULL, 10, NULL
	);
	FusionDebugger.AddItemToDebugger(
		_T("Y Speed: "sv), NULL,
		[](Extension* ext, std::tstring& writeTo) {
			writeTo = std::to_tstring(ext->_ySpeed);
		},
		NULL, 10, NULL
	);
#endif

	// Read object DarkEdif properties; you can pass property name, or property index
	// This will work on all platforms the same way.
	// See edPtr->Props functions.
	Divisor = edPtr->Props.GetPropertyNum(0);
	Margin = edPtr->Props.GetPropertyNum(1);
	Factor = Clamp(edPtr->Props.GetPropertyNum(2), 0, 100);

	DontScroll = edPtr->Props.IsPropChecked(3);
	CenterDisplay = edPtr->Props.IsPropChecked(4);
	Easing = edPtr->Props.IsPropChecked(5);
	HoriScrolling = edPtr->Props.IsPropChecked(6);
	VertScrolling = edPtr->Props.IsPropChecked(7);
	Peytonphile = edPtr->Props.IsPropChecked(8);
	PeytonphileToEdges = edPtr->Props.IsPropChecked(9);

	HoriFlipped = edPtr->Props.IsPropChecked(10);
	VertFlipped = edPtr->Props.IsPropChecked(11);

#if defined(__ANDROID__) || defined(__APPLE__)
	IgnoreLast = edPtr->Props.IsPropChecked(12);
#endif

	_marginMiddleX = _marginMiddleY =
		_dt = _xSpeed = _ySpeed = 0;

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

#if TEXT_OEFLAG_EXTENSION
	// Copy from edittime data into runtime data
	font.CopyFromEditFont(this, edPtr->font);

	// Set Runtime.fontChangedFunc to trigger a function if the runtime
	// changes your ext's font via the built-in font actions
	Runtime.fontChangedFunc = &Extension::OnFontChanged;
#endif

#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	Runtime.SetSurfaceWithSize(edPtr->objSize.width, edPtr->objSize.height);
	// surf is already constructed
	// To start, let's make the image a solid blue fill
	surf->FillImageWith(DarkEdif::SurfaceFill::Solid(DarkEdif::ColorRGB(0, 0, 127)));
#endif
}

Extension::~Extension()
{

}

// Runs every tick of Fusion's runtime, can be toggled off and back on
REFLAG Extension::Handle()
{
	_resX = GetFrameRight() - GetFrameLeft();
	_resY = GetFrameBottom() - GetFrameTop();

	_dt = GetDelta();

	int androidTap = true;
#if defined(__ANDROID__) || defined(__APPLE__)
	if (IgnoreLast && !IsTapped())
		androidTap = false;
#endif

	if (!Peytonphile && androidTap)
	{
		_marginMiddleX = Clamp(Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft(), ((_resX / 2) - (Margin / 2)), ((_resX / 2) + (Margin / 2)));
		_marginMiddleY = Clamp(Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop(), ((_resY / 2) - (Margin / 2)), ((_resY / 2) + (Margin / 2)));
		_xSpeed = 0;
		_ySpeed = 0;
	}

	if (!DontScroll && androidTap && HoriScrolling && !Peytonphile)
	{
		_xSpeed = ((Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft()) - _marginMiddleX) / Divisor;
		_scrollingXTarget = Clamp((_scrollingXTarget + (_xSpeed * _dt)), (_resX / 2), (GetVirtualWidth() - (_resX / 2)));
	}

	if (!DontScroll && androidTap && VertScrolling && !Peytonphile)
	{
		_ySpeed = ((Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop()) - _marginMiddleY) / (Divisor + 0.0f) * ((_resX + 0.0f) / _resY);
		_scrollingYTarget = Clamp((_scrollingYTarget + (_ySpeed * _dt)), (_resY / 2), (GetVirtualHeight() - (_resY / 2)));
	}

	if (Peytonphile && androidTap)
	{
		_xSpeed = 0;
		_ySpeed = 0;

		int mX = GetMouseX() - Margin;
		int mY = GetMouseY() - Margin;
		int mWidth = GetVirtualWidth() - Margin * 2;
		int mHeight = GetVirtualHeight() - Margin * 2;
		int eWidth = PeytonphileToEdges ? GetVirtualWidth() : mWidth;
		int eHeight = PeytonphileToEdges ? GetVirtualHeight() : mHeight;

		if (!DontScroll && HoriScrolling)
			_scrollingXTarget = Clamp((eWidth / 2) + (((mX - (mWidth / 2))) * ((eWidth - (_resX + 0.0)) / mWidth)), (_resX / 2), eWidth - (_resX / 2)) + (PeytonphileToEdges ? 0 : Margin);

		if (!DontScroll && VertScrolling)
			_scrollingYTarget = Clamp((eHeight / 2) + (((mY - (mHeight / 2))) * ((eHeight - (_resY + 0.0)) / mHeight)), (_resY / 2), eHeight - (_resY / 2)) + (PeytonphileToEdges ? 0 : Margin);
	}

	if (!Easing && HoriScrolling)
	{
		_scrollingX = _scrollingXTarget;
		if (CenterDisplay)
			SetFrameCenterX(_scrollingX);
	}

	if (!Easing && VertScrolling)
	{
		_scrollingY = _scrollingYTarget;
		if (CenterDisplay)
			SetFrameCenterY(_scrollingY);
	}

	if (Easing && HoriScrolling)
	{
		_scrollingX = _scrollingX + (_scrollingXTarget - _scrollingX) * Clamp((Factor / 100.0f) * _dt, 0.0, 1.0);
		if (CenterDisplay)
			SetFrameCenterX(_scrollingX);
	}

	if (Easing && VertScrolling)
	{
		_scrollingY = _scrollingY + (_scrollingYTarget - _scrollingY) * Clamp((Factor / 100.0f) * _dt, 0.0, 1.0);
		if (CenterDisplay)
			SetFrameCenterY(_scrollingY);
	}

	return REFLAG::NONE;
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
	if ((std::size_t)ID < Edif::SDK->ExpressionInfos.size() && Edif::SDK->ExpressionInfos[ID]->Flags.ef == ExpReturnType::String)
		return (long)Runtime.CopyString(_T(""));
	return 0;
}

int Extension::GetFrameRight()
{
	int r = rhPtr->get_WindowX();
	if ((rhPtr->GetRH3Scrolling() & 0x1) != 0) // RH3SCROLLING_SCROLL
		r = rhPtr->GetRH3DisplayX();
	r += rhPtr->GetRH3WindowSx();
	if (r > rhPtr->get_LevelSx())
		r = rhPtr->get_LevelSx();
	return r;
}

int Extension::GetFrameLeft()
{
	int r = rhPtr->get_WindowX();
	if ((rhPtr->GetRH3Scrolling() & 0x1) != 0) // RH3SCROLLING_SCROLL
		r = rhPtr->GetRH3DisplayX();
	if (r < 0)
		r = 0;
	return r;
}

int Extension::GetFrameBottom()
{
	int r = rhPtr->get_WindowY();
	if ((rhPtr->GetRH3Scrolling() & 0x1) != 0) // RH3SCROLLING_SCROLL
		r = rhPtr->GetRH3DisplayY();
	r += rhPtr->GetRH3WindowSy();
	if (r > rhPtr->get_LevelSy())
		r = rhPtr->get_LevelSy();
	return r;
}

int Extension::GetFrameTop()
{
	int r = rhPtr->get_WindowY();
	if ((rhPtr->GetRH3Scrolling() & 0x1) != 0) // RH3SCROLLING_SCROLL
		r = rhPtr->GetRH3DisplayY();
	if (r < 0)
		r = 0;
	return r;
}

void Extension::SetFrameCenterX(int centerX)
{
	centerX = Clamp(centerX - rhPtr->GetRH3WindowSx() / 2, 0, GetVirtualWidth() - rhPtr->GetRH3WindowSx());
	rhPtr->SetRH3DisplayX(centerX);
	Runtime.Redisplay();
}

void Extension::SetFrameCenterY(int centerY)
{
	centerY = Clamp(centerY - rhPtr->GetRH3WindowSy() / 2, 0, GetVirtualHeight() - rhPtr->GetRH3WindowSy());
	rhPtr->SetRH3DisplayY(centerY);
	Runtime.Redisplay();
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
	if (HoriFlipped)
		return (_resX - rhPtr->getRH2MouseClientX()) + rhPtr->GetRH3DisplayX();
	else
		return rhPtr->getRH2MouseX();
}

int Extension::GetMouseY()
{
	if (VertFlipped)
		return (_resY - rhPtr->getRH2MouseClientY()) + rhPtr->GetRH3DisplayY();
	else
		return rhPtr->getRH2MouseY();
}

double Extension::GetDelta()
{
	return rhPtr->getRH4MvtTimerCoef();
}

int Extension::GetVirtualWidth()
{
	return rhPtr->getFrameVirtualWidth();
}

int Extension::GetVirtualHeight()
{
	return rhPtr->getFrameVirtualHeight();
}

#if defined(__ANDROID__) || defined(__APPLE__)
bool Extension::IsTapped()
{
#ifdef __ANDROID__
	return AndroidMMFRuntime::get(&Runtime)->get_TouchManager()->anyTouchDown();
#else
	return rhPtr->getMouseDown();
#endif
}
#endif
