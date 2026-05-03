#pragma once
#include "DarkEdif.hpp"

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 4;

	// Warning: OEFLAGS/OEPREFS cannot be freely modified when you have used them in MFAs.
	static constexpr OEFLAGS OEFLAGS = OEFLAGS::VALUES;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;
	// If OEFLAGS::WINDOW_PROC (otherwise you can delete)
	// static constexpr int WindowProcPriority = 100;
	// If OEFLAGS::TEXT (otherwise you can delete)
	// static constexpr TextCapacity TextCapacity = TextCapacity::None;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr);
#endif
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	// To add items to the Fusion Debugger, just uncomment this line.
	DarkEdif::FusionDebugger FusionDebugger;
	// After enabling it, you run FusionDebugger.AddItemToDebugger() inside Extension's constructor
	// As an example:
	std::tstring exampleDebuggerTextItem;


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr in old SDKs).

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	float Divisor;
	float Margin;
	float Factor;

	bool CenterDisplay;
	bool Easing;
	bool HoriScrolling;
	bool VertScrolling;
	bool Peytonphile;
	bool PeytonphileToEdges;
	bool HoriFlipped;
	bool VertFlipped;

#if defined(__ANDROID__) || defined(__APPLE__)
	bool IgnoreLast;
#endif

	float _scrollingX;
	float _scrollingY;
	float _scrollingXTarget;
	float _scrollingYTarget;
	float _marginMiddleX;
	float _marginMiddleY;
	float _resX;
	float _resY;
	float _dt;
	float _xSpeed;
	float _ySpeed;

	bool DontScroll;

	int GetFrameRight();
	int GetFrameLeft();
	int GetFrameBottom();
	int GetFrameTop();

	void SetFrameCenterX(int centerX);
	void SetFrameCenterY(int centerY);

	double Clamp(double value, double min, double max);

	int GetMouseX();
	int GetMouseY();

	double GetDelta();

	int GetVirtualWidth();
	int GetVirtualHeight();

#if defined(__ANDROID__) || defined(__APPLE__)
	bool IsTapped();
#endif


	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/

	/// Actions

	void SetDivisor(float divisor);
	void SetMargin(float margin);
	void SetFactor(float factor);

	void SetDisallowScrolling(int setting);
	void SetCenterDisplay(int setting);
	void SetEasing(int setting);
	void SetHoriScrolling(int setting);
	void SetVertScrolling(int setting);
	void SetPeytonphile(int setting);

	void SetCameraPosX(float cameraX);
	void SetCameraPosY(float cameraY);
	void SetCameraTargetX(float cameraX);
	void SetCameraTargetY(float cameraY);

	void FlipHorizontally();
	void FlipVertically();

	/// Conditions

	bool CheckDisallowScrolling();
	bool CheckCenterDisplay();
	bool CheckEasing();
	bool CheckHoriScrolling();
	bool CheckVertScrolling();
	bool CheckPeytonphile();

	bool CheckHoriFlipped();
	bool CheckVertFlipped();

	/// Expressions

	float GetDivisor();
	float GetMargin();
	float GetFactor();

	float GetXScroll();
	float GetYScroll();
	float GetXScrollTarget();
	float GetYScrollTarget();
	float GetXSpeed();
	float GetYSpeed();

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

#if TEXT_OEFLAG_EXTENSION
	// Extension text struct. Required for text exts.
	DarkEdif::FontInfoMultiPlat font;
	void OnFontChanged(bool colorEdit, DarkEdif::Rect* rc);
#endif
#if DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_SIMPLE
	// Extension display surface ptr. Required for simple display exts.
	DarkEdif::Surface * surf = nullptr;
#elif DARKEDIF_DISPLAY_TYPE == DARKEDIF_DISPLAY_MANUAL
	void Display();
	void GetZoneInfos();
	DarkEdif::Surface * GetDisplaySurface();
	DarkEdif::CollisionMask * GetCollisionMask(std::uint32_t flags);
#endif

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);

#if PAUSABLE_EXTENSION
	// Called when Fusion runtime is pausing - not just the F3 pause dialog
	void FusionRuntimePaused();
	// Called when Fusion runtime is resuming after a pause
	void FusionRuntimeContinued();
#endif // PAUSABLE_EXTENSION
};
