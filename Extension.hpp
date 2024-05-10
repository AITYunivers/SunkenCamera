#pragma once
#include "DarkEdif.hpp"

class Extension
{
public:

	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr; // you should not need to access this
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 254;
	static const int Version = 1;

	// If you change OEFLAGS, make sure you modify RUNDATA so the data is available, or you'll get crashes!
	// For example, OEFLAGS::VALUES makes use of the AltVals rv struct.
	static const OEFLAGS OEFLAGS = OEFLAGS::NONE;
	static const OEPREFS OEPREFS = OEPREFS::NONE;

	static const int WindowProcPriority = 100;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr);
#endif
	~Extension();

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
	bool HoriFlipped;
	bool VertFlipped;

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

	bool _dontScroll;

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



	/* These are called if there's no function linked to an ID */

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);




	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short FusionRuntimePaused();
	short FusionRuntimeContinued();
};
