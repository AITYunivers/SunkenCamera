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
	crectClass(mainThreadJNIEnv->FindClass("Services/CRect"), "Services/CRect"),
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

	LinkAction(0,  SetDivisor);
	LinkAction(1,  SetMargin);
	LinkAction(2,  SetFactor);

	LinkAction(3,  SetDisallowScrolling);
	LinkAction(8,  SetCenterDisplay);
	LinkAction(4,  SetEasing);
	LinkAction(5,  SetHoriScrolling);
	LinkAction(6,  SetVertScrolling);
	LinkAction(7,  SetPeytonphile);

	LinkAction(9,  SetCameraPosX);
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
	Divisor = edPtr->Props.GetPropertyNum(0);
	Margin = edPtr->Props.GetPropertyNum(1);
	Factor = Clamp(edPtr->Props.GetPropertyNum(2), 0, 100);

	CenterDisplay = edPtr->Props.IsPropChecked(3);
	Easing = edPtr->Props.IsPropChecked(4);
	HoriScrolling = edPtr->Props.IsPropChecked(5);
	VertScrolling = edPtr->Props.IsPropChecked(6);
	Peytonphile = edPtr->Props.IsPropChecked(7);

	HoriFlipped = edPtr->Props.IsPropChecked(8);
	VertFlipped = edPtr->Props.IsPropChecked(9);

#if __ANDROID__
	IgnoreLast = edPtr->Props.IsPropChecked(10);
#endif

	_marginMiddleX = _marginMiddleY =
	_dt = _xSpeed = _ySpeed = 0;
	_dontScroll = false;

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

	int androidTap = true;
#if __ANDROID__
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

	if (!_dontScroll && androidTap && HoriScrolling && !Peytonphile)
	{
		_xSpeed = ((Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft()) - _marginMiddleX) / Divisor;
		_scrollingXTarget = Clamp((_scrollingXTarget + (_xSpeed * _dt)), (_resX / 2), (GetVirtualWidth() - (_resX / 2)));
	}

	if (!_dontScroll && androidTap && VertScrolling && !Peytonphile)
	{
		_ySpeed = ((Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop()) - _marginMiddleY) / (Divisor + 0.0f) * ((_resX + 0.0f) / _resY);
		_scrollingYTarget = Clamp((_scrollingYTarget + (_ySpeed * _dt)), (_resY / 2), (GetVirtualHeight() - (_resY / 2)));
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

	if (Peytonphile && androidTap)
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
#elif __ANDROID__
	jfieldID _fid_rhWindowX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhWindowX", "I");
	JNIExceptionCheck();
	int rhWindowX = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhWindowX);
	JNIExceptionCheck();

	jfieldID _fid_rh3Scrolling = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3Scrolling", "C");
	JNIExceptionCheck();
	char rh3Scrolling = mainThreadJNIEnv->GetCharField(rhPtr->crun, _fid_rh3Scrolling);
	JNIExceptionCheck();

	int r = rhWindowX;
	if ((rh3Scrolling & 1) != 0)
	{
		jfieldID _fid_rh3DisplayX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayX", "I");
		JNIExceptionCheck();
		int rh3DisplayX = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayX);
		JNIExceptionCheck();

		r = rh3DisplayX;
	}

	jfieldID _fid_rh3WindowSx = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3WindowSx", "I");
	JNIExceptionCheck();
	int rh3WindowSx = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3WindowSx);
	JNIExceptionCheck();

	r += rh3WindowSx;

	jfieldID _fid_rhLevelSx = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhLevelSx", "I");
	JNIExceptionCheck();
	int rhLevelSx = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhLevelSx);
	JNIExceptionCheck();

	if (r > rhLevelSx)
		r = rhLevelSx;
#endif

	return r;
}

int Extension::GetFrameLeft()
{
#ifdef _WIN32
	int r = rhPtr->WindowX;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayX;
#elif __ANDROID__
	jfieldID _fid_rhWindowX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhWindowX", "I");
	JNIExceptionCheck();
	int rhWindowX = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhWindowX);
	JNIExceptionCheck();

	jfieldID _fid_rh3Scrolling = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3Scrolling", "C");
	JNIExceptionCheck();
	char rh3Scrolling = mainThreadJNIEnv->GetCharField(rhPtr->crun, _fid_rh3Scrolling);
	JNIExceptionCheck();

	int r = rhWindowX;
	if ((rh3Scrolling & 1) != 0)
	{
		jfieldID _fid_rh3DisplayX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayX", "I");
		JNIExceptionCheck();
		int rh3DisplayX = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayX);
		JNIExceptionCheck();

		r = rh3DisplayX;
	}
#endif

	if (r < 0)
		r = 0;
	return r;
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
#elif __ANDROID__
	jfieldID _fid_rhWindowY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhWindowY", "I");
	JNIExceptionCheck();
	int rhWindowY = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhWindowY);
	JNIExceptionCheck();

	jfieldID _fid_rh3Scrolling = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3Scrolling", "C");
	JNIExceptionCheck();
	char rh3Scrolling = mainThreadJNIEnv->GetCharField(rhPtr->crun, _fid_rh3Scrolling);
	JNIExceptionCheck();

	int r = rhWindowY;
	if ((rh3Scrolling & 1) != 0)
	{
		jfieldID _fid_rh3DisplayY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayY", "I");
		JNIExceptionCheck();
		int rh3DisplayY = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayY);
		JNIExceptionCheck();

		r = rh3DisplayY;
	}

	jfieldID _fid_rh3WindowSy = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3WindowSy", "I");
	JNIExceptionCheck();
	int rh3WindowSy = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3WindowSy);
	JNIExceptionCheck();

	r += rh3WindowSy;

	jfieldID _fid_rhLevelSy = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhLevelSy", "I");
	JNIExceptionCheck();
	int rhLevelSy = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhLevelSy);
	JNIExceptionCheck();

	if (r > rhLevelSy)
		r = rhLevelSy;
#endif

	return r;
}

int Extension::GetFrameTop()
{
#ifdef _WIN32
	int r = rhPtr->WindowY;
	if ((rhPtr->rh3.Scrolling & RH3SCROLLING_SCROLL) != 0)
		r = rhPtr->rh3.DisplayY;
#elif __ANDROID__
	jfieldID _fid_rhWindowY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rhWindowY", "I");
	JNIExceptionCheck();
	int rhWindowY = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rhWindowY);
	JNIExceptionCheck();

	jfieldID _fid_rh3Scrolling = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3Scrolling", "C");
	JNIExceptionCheck();
	char rh3Scrolling = mainThreadJNIEnv->GetCharField(rhPtr->crun, _fid_rh3Scrolling);
	JNIExceptionCheck();

	int r = rhWindowY;
	if ((rh3Scrolling & 1) != 0)
	{
		jfieldID _fid_rh3DisplayY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayY", "I");
		JNIExceptionCheck();
		int rh3DisplayY = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayY);
		JNIExceptionCheck();

		r = rh3DisplayY;
	}
#endif

	if (r < 0)
		r = 0;
	return r;
}

void Extension::SetFrameCenterX(int centerX)
{
#ifdef _WIN32
	centerX = Clamp(centerX - rhPtr->rh3.WindowSx / 2, 0, GetVirtualWidth() - rhPtr->rh3.WindowSx);
	rhPtr->rh3.DisplayX = centerX;
#elif __ANDROID__
	jfieldID _fid_rh3WindowSx = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3WindowSx", "I");
	JNIExceptionCheck();
	int rh3WindowSx = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3WindowSx);
	JNIExceptionCheck();

	centerX = Clamp(centerX - rh3WindowSx / 2, 0, GetVirtualWidth() - rh3WindowSx);

	jfieldID _fid_rh3DisplayX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayX", "I");
	JNIExceptionCheck();
	mainThreadJNIEnv->SetIntField(rhPtr->crun, _fid_rh3DisplayX, centerX);
	JNIExceptionCheck();
#endif

	Runtime.Redisplay();
}

void Extension::SetFrameCenterY(int centerY)
{
#ifdef _WIN32
	centerY = Clamp(centerY - rhPtr->rh3.WindowSy / 2, 0, GetVirtualHeight() - rhPtr->rh3.WindowSy);
	rhPtr->rh3.DisplayY = centerY;
#elif __ANDROID__
	jfieldID _fid_rh3WindowSy = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3WindowSy", "I");
	JNIExceptionCheck();
	int rh3WindowSy = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3WindowSy);
	JNIExceptionCheck();

	centerY = Clamp(centerY - rh3WindowSy / 2, 0, GetVirtualHeight() - rh3WindowSy);

	jfieldID _fid_rh3DisplayY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayY", "I");
	JNIExceptionCheck();
	mainThreadJNIEnv->SetIntField(rhPtr->crun, _fid_rh3DisplayY, centerY);
	JNIExceptionCheck();
#endif

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
#ifdef _WIN32
	if (HoriFlipped)
		return (_resX - rhPtr->rh2.MouseClient.x) + rhPtr->rh3.DisplayX;
	else
		return rhPtr->rh2.Mouse.x;
#elif __ANDROID__
	jfieldID _fid_mouseX = mainThreadJNIEnv->GetFieldID(rhPtr->get_App()->meClass, "mouseX", "I");
	JNIExceptionCheck();
	int mouseX = mainThreadJNIEnv->GetIntField(rhPtr->get_App()->me, _fid_mouseX);
	JNIExceptionCheck();

	jfieldID _fid_rh3DisplayX = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayX", "I");
	JNIExceptionCheck();
	int rh3DisplayX = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayX);
	JNIExceptionCheck();

	if (HoriFlipped)
		return (_resX - mouseX) + rh3DisplayX;
	else
		return mouseX + rh3DisplayX;
#endif
}

int Extension::GetMouseY()
{
#ifdef _WIN32
	if (VertFlipped)
		return (_resY - rhPtr->rh2.MouseClient.y) + rhPtr->rh3.DisplayY;
	else
		return rhPtr->rh2.Mouse.y;
#elif __ANDROID__
	jfieldID _fid_mouseY = mainThreadJNIEnv->GetFieldID(rhPtr->get_App()->meClass, "mouseY", "I");
	JNIExceptionCheck();
	int mouseY = mainThreadJNIEnv->GetIntField(rhPtr->get_App()->me, _fid_mouseY);
	JNIExceptionCheck();

	jfieldID _fid_rh3DisplayY = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh3DisplayY", "I");
	JNIExceptionCheck();
	int rh3DisplayY = mainThreadJNIEnv->GetIntField(rhPtr->crun, _fid_rh3DisplayY);
	JNIExceptionCheck();

	if (VertFlipped)
		return (_resY - mouseY) + rh3DisplayY;
	else
		return mouseY + rh3DisplayY;
#endif
}

double Extension::GetDelta()
{
#ifdef _WIN32
	return rhPtr->rh4.mvtTimerCoef;
#elif __ANDROID__
	jfieldID _fid_rh4MvtTimerCoef = mainThreadJNIEnv->GetFieldID(rhPtr->crunClass, "rh4MvtTimerCoef", "D");
	JNIExceptionCheck();
	double rh4MvtTimerCoef = mainThreadJNIEnv->GetDoubleField(rhPtr->crun, _fid_rh4MvtTimerCoef);
	JNIExceptionCheck();
	return rh4MvtTimerCoef;
#endif
}

int Extension::GetVirtualWidth()
{
#ifdef _WIN32
	return rhPtr->Frame->VirtualRect.right;
#elif __ANDROID__
	CRunFrame* frame = rhPtr->get_App()->get_Frame();
	jfieldID _fid_leVirtualRect = mainThreadJNIEnv->GetFieldID(frame->meClass, "leVirtualRect", "LServices/CRect;");
	JNIExceptionCheck();
	jobject leVirtualRect = mainThreadJNIEnv->GetObjectField(frame->me, _fid_leVirtualRect);
	JNIExceptionCheck();
	jfieldID _fid_right = mainThreadJNIEnv->GetFieldID(crectClass, "right", "I");
	JNIExceptionCheck();
	int right = mainThreadJNIEnv->GetIntField(leVirtualRect, _fid_right);
	JNIExceptionCheck();
	return right;
#endif
}

int Extension::GetVirtualHeight()
{
#ifdef _WIN32
	return rhPtr->Frame->VirtualRect.bottom;
#elif __ANDROID__
	CRunFrame* frame = rhPtr->get_App()->get_Frame();
	jfieldID _fid_leVirtualRect = mainThreadJNIEnv->GetFieldID(frame->meClass, "leVirtualRect", "LServices/CRect;");
	JNIExceptionCheck();
	jobject leVirtualRect = mainThreadJNIEnv->GetObjectField(frame->me, _fid_leVirtualRect);
	JNIExceptionCheck();
	jfieldID _fid_bottom = mainThreadJNIEnv->GetFieldID(crectClass, "bottom", "I");
	JNIExceptionCheck();
	int bottom = mainThreadJNIEnv->GetIntField(leVirtualRect, _fid_bottom);
	JNIExceptionCheck();
	return bottom;
#endif
}

#if __ANDROID__
bool Extension::IsTapped()
{
	return AndroidMMFRuntime::get(&Runtime)->get_TouchManager()->anyTouchDown();
}
#endif
