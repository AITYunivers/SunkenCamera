/*
	This DarkEdif Template Fusion extension HTML5 port is under MIT license.

	Modification for purposes of tuning to your own HTML5 application is permitted, but must retain this notice and not be redistributed,
	outside of its (hopefully minified) presence inside your HTML5 website's source code.
*/

/* global console, darkEdif, globalThis, alert, CRunExtension, FinalizationRegistry, CServices */
/* jslint esversion: 6, sub: true */

// This is strict, but that can be assumed
// "use strict";

// Global data, including sub-applications, just how God intended.
// Note: This will allow newer SDK versions in later SDKs to take over.
// We need this[] and globalThis[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
globalThis['darkEdif'] = (globalThis['darkEdif'] && globalThis['darkEdif'].sdkVersion >= 19) ? globalThis['darkEdif'] :
	new (/** @constructor */ function() {
	// window variable is converted into __scope for some reason, so globalThis it is.
	this.data = {};
	this.getGlobalData = function (key) {
		key = key.toLowerCase();
		if (key in this.data) {
			return this.data[key];
		}
		return null;
	};
	this.setGlobalData = function (key, value) {
		key = key.toLowerCase();
		this.data[key] = value;
	};

	this.getCurrentFusionEventNumber = function (ext) {
		return ext.rh.rhEvtProg.rhEventGroup.evgLine || -1;
	};
	this.sdkVersion = 19;
	this.checkSupportsSDKVersion = function (sdkVer) {
		if (sdkVer < 16 || sdkVer > 19) {
			throw "HTML5 DarkEdif SDK does not support SDK version " + this.sdkVersion;
		}
	};

	// minifier will rename notMinified, so we can detect minifier simply
	this.minified = false;
	if (!this.hasOwnProperty('minified')) {
		this['minified'] = true;
	}

	this.consoleLog = function (ext, str) {
		// Log if DebugMode not defined, or true
		if (ext == null || ext['DebugMode'] == null || ext['DebugMode']) {
			// Exts will be renamed in minified
			if (this['minified'] && ext != null && ext['DebugMode'] == true) {
				console.warn("DebugMode left true for an extension in minified mode. Did the ext developer not set it false before distributing?");
				ext['DebugMode'] = false;
			}

			const extName = (ext == null || this['minified']) ? "Unknown DarkEdif ext" :
				ext['ExtensionName'] || ext.constructor.name.replaceAll('CRun', '').replaceAll('_',' ');
			console.log(extName + " - " + str);
		}
	};
	if (!this['minified']) {
		let that = this;
		// Use this for debugging to make sure objects are deleted.
		// Note they're not garbage collected when last holder releases it, but at any point after,
		// when the GC decides to.
		// On Chrome, it took half a minute or so, and delay was possibly affected by whether the page has focus.
		// GC is not required, remember - the cleanup may not happen at all in some browsers.
		this.finalizer = new FinalizationRegistry(function(desc) {
			that.consoleLog(null, "Noting the destruction of [" + desc + "].");
		});
	}
	else {
		this.finalizer = { register: function(desc) { } };
	}

	this['Properties'] = function(ext, edPtrFile, extVersion) {
		// DarkEdif SDK stores offset of DarkEdif props away from start of EDITDATA inside private data.
		// eHeader is 20 bytes, so this should be 20+ bytes.
		if (ext.ho.privateData < 20) {
			throw "Not smart properties - eHeader missing?";
		}
		// DarkEdif SDK header read:
		// header uint32, hash uint32, hashtypes uint32, numprops uint16, pad uint16, sizeBytes uint32 (includes whole EDITDATA)
		// then checkbox list, one bit per checkbox, including non-checkbox properties
		// so skip numProps / 8 bytes
		// then moving to Data list:
		// size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

		let header = new Uint8Array(edPtrFile.readBuffer(4 + 4 + 4 + 2 + 2 + 4));
		if (String.fromCharCode.apply('', [header[3], header[2], header[1], header[0]]) != 'DAR1') {
			throw "Did you read this.ho.privateData bytes?";
		}

		let headerDV = new DataView(header.buffer);
		this.numProps = headerDV.getUint16(4 + 4 + 4, true); // Skip past hash and hashTypes
		this.sizeBytes = headerDV.getUint32(4 + 4 + 4 + 4, true); // skip past numProps and pad

		let editData = edPtrFile.readBuffer(this.sizeBytes - header.byteLength);
		this.chkboxes = editData.slice(0, Math.ceil(this.numProps / 8));
		let that = this;
		let GetPropertyIndex = function(chkIDOrName) {
			if (typeof chkIDOrName == 'number') {
				if (that.numProps >= chkIDOrName) {
					throw "Invalid property ID " + chkIDOrName + ", max ID is " + (that.numProps - 1) + ".";
				}
				return chkIDOrName;
			}
			const p = that.props.find(function(p) { return p.propName == chkIDOrName; });
			if (p == null) {
				throw "Invalid property name \"" + chkIDOrName + "\"";
			}
			return p.index;
		};
		this['IsPropChecked'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return 0;
			}
			return (that.chkboxes[Math.floor(idx / 8)] & (1 << idx % 8)) != 0;
		};
		this['GetPropertyStr'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return "";
			}
			const prop = that.props[idx];
			const textPropIDs = [
				5, // PROPTYPE_EDIT_STRING:
				22, // PROPTYPE_EDIT_MULTILINE:
				16, // PROPTYPE_FILENAME:
				19, // PROPTYPE_PICTUREFILENAME:
				26, // PROPTYPE_DIRECTORYNAME:
				7, // PROPTYPE_COMBOBOX:
				20, // PROPTYPE_COMBOBOXBTN:
				24 // PROPTYPE_ICONCOMBOBOX:
			];
			if (textPropIDs.indexOf(prop.propTypeID) != -1) {
				let t = that.textDecoder.decode(prop.propData);
				if (prop.propTypeID == 22) { //PROPTYPE_EDIT_MULTILINE
					t = t.replaceAll('\r', ''); // CRLF to LF
				}
				return t;
			}
			throw "Property " + prop.propName + " is not textual.";
		};
		this['GetPropertyNum'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return 0.0;
			}
			const prop = that.props[idx];
			const numPropIDsInteger = [
				6, // PROPTYPE_EDIT_NUMBER
				9, // PROPTYPE_COLOR
				11, // PROPTYPE_SLIDEREDIT
				12, // PROPTYPE_SPINEDIT
				13 // PROPTYPE_DIRCTRL
			];
			const numPropIDsFloat = [
				21, // PROPTYPE_EDIT_FLOAT
				27 // PROPTYPE_SPINEDITFLOAT
			];
			if (numPropIDsInteger.indexOf(prop.propTypeID) != -1) {
				return new DataView(prop.propData.buffer).getUint32(0, true);
			}
			if (numPropIDsFloat.indexOf(prop.propTypeID) != -1) {
				return new DataView(prop.propData.buffer).getFloat32(0, true);
			}
			throw "Property " + prop.propName + " is not numeric.";
		};

		this.props = [];
		const data = editData.slice(this.chkboxes.length);
		const dataDV = new DataView(new Uint8Array(data).buffer);

		this.textDecoder = null;
		if (globalThis['TextDecoder'] != null) {
			this.textDecoder = new globalThis['TextDecoder']();
		}
		else {
			// one byte = one char - should suffice for basic ASCII property names
			this.textDecoder = {
				decode: function(txt) {
					return String.fromCharCode.apply("", txt);
				}
			};
		}

		for (let i = 0, pt = 0, propSize, propEnd; i < this.numProps; ++i) {
			propSize = dataDV.getUint32(pt, true);
			propEnd = pt + propSize;
			const propTypeID = dataDV.getUint16(pt + 4, true);
			const propNameLength = dataDV.getUint8(pt + 4 + 2);
			pt += 4 + 2 + 1;
			const propName = this.textDecoder.decode(new Uint8Array(data.slice(pt, pt + propNameLength)));
			pt += propNameLength;
			const propData = new Uint8Array(data.slice(pt, pt + propSize - (4 + 2 + 1 + propNameLength)));

			this.props.push({ index: i, propTypeID: propTypeID, propName: propName, propData: propData });
			pt = propEnd;
		}
	};
})();

/** @constructor */
function CRunSunkenCamera() {
	/// <summary> Constructor of Fusion object. </summary>

	// DarkEdif SDK exts should have these four variables defined.
	// We need this[] and globalThis[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
	this['ExtensionVersion'] = 1; // To match C++ version
	this['SDKVersion'] = 19; // To match C++ version
	this['DebugMode'] = true;
	this['ExtensionName'] = 'DarkEdif Template';

	// Can't find DarkEdif wrapper
	if (!globalThis.hasOwnProperty('darkEdif')) {
		throw "a wobbly";
	}
	globalThis['darkEdif'].checkSupportsSDKVersion(this.SDKVersion);

	// ======================================================================================================
	// Actions
	// ======================================================================================================
	this.Action_SetDivisor = function (divisor) {
		this.Divisor = divisor;
	};
	this.Action_SetMargin = function (margin) {
		this.Margin = margin;
	};
	this.Action_SetFactor = function (factor) {
		this.Factor = this.Clamp(factor, 0, 100);
	};
	this.Action_SetDisallowScrolling = function (setting) {
		this._dontScroll = setting != 0;
	};
	this.Action_SetCenterDisplay = function (setting) {
		this.CenterDisplay = setting != 0;
	};
	this.Action_SetHoriScrolling = function (setting) {
		this.HoriScrolling = setting != 0;
	};
	this.Action_SetVertScrolling = function (setting) {
		this.VertScrolling = setting != 0;
	};
	this.Action_SetEasing = function (setting) {
		this.Easing = setting != 0;
	};
	this.Action_SetPeytonphile = function (setting) {
		this.Peytonphile = setting != 0;
	};

	// ======================================================================================================
	// Conditions
	// ======================================================================================================
	this.Condition_CheckDisallowScrolling = function () {
		return _dontScroll;
	};
	this.Condition_CheckCenterDisplay = function () {
		return CenterDisplay;
	};
	this.Condition_CheckEasing = function () {
		return Easing;
	};
	this.Condition_CheckHoriScrolling = function () {
		return HoriScrolling;
	};
	this.Condition_CheckVertScrolling = function () {
		return VertScrolling;
	};
	this.Condition_CheckPeytonphile = function () {
		return Peytonphile;
	};

	// =============================
	// Expressions
	// =============================

	this.Expression_GetDivisor = function () {
		return Divisor;
	};
	this.Expression_GetMargin = function () {
		return Margin;
	};
	this.Expression_GetFactor = function () {
		return Factor;
	};
	this.Expression_GetXScroll = function () {
		return _scrollingX;
	};
	this.Expression_GetYScroll = function () {
		return _scrollingY;
	};
	this.Expression_GetXScrollTarget = function () {
		return _scrollingXTarget;
	};
	this.Expression_GetYScrollTarget = function () {
		return _scrollingYTarget;
	};
	this.Expression_GetXSpeed = function () {
		return _xSpeed;
	};
	this.Expression_GetYSpeed = function () {
		return _ySpeed;
	};

	// =============================
	// Function arrays
	// =============================

	this.$actionFuncs = [
	/* 0 */ this.Action_SetDivisor,
	/* 1 */ this.Action_SetMargin,
	/* 2 */ this.Action_SetFactor,

	/* 3 */ this.Action_SetDisallowScrolling,
	/* 4 */ this.Action_SetEasing,
	/* 5 */ this.Action_SetHoriScrolling,
	/* 6 */ this.Action_SetVertScrolling,
	/* 7 */ this.Action_SetPeytonphile,
	/* 8 */ this.Action_SetCenterDisplay
	];

	this.$conditionFuncs = [
	/* 0 */ this.Condition_CheckDisallowScrolling,
	/* 1 */ this.Condition_CheckEasing,
	/* 2 */ this.Condition_CheckHoriScrolling,
	/* 3 */ this.Condition_CheckVertScrolling,
	/* 4 */ this.Condition_CheckPeytonphile,
	/* 5 */ this.Condition_CheckCenterDisplay
	];

	this.$expressionFuncs = [
	/* 0 */ this.Expression_GetDivisor,
	/* 1 */ this.Expression_GetMargin,
	/* 2 */ this.Expression_GetFactor,

	/* 3 */ this.Expression_GetXScroll,
	/* 4 */ this.Expression_GetYScroll,
	/* 5 */ this.Expression_GetXScrollTarget,
	/* 6 */ this.Expression_GetYScrollTarget,
	/* 7 */ this.Expression_GetXSpeed,
	/* 8 */ this.Expression_GetYSpeed
	];
}
//
CRunSunkenCamera.prototype = CServices.extend(new CRunExtension(), {
	/// <summary> Prototype definition </summary>
	/// <description> This class is a sub-class of CRunExtension, by the mean of the
	/// CServices.extend function which copies all the properties of
	/// the parent class to the new class when it is created.
	/// As all the necessary functions are defined in the parent class,
	/// you only need to keep the ones that you actually need in your code. </description>

	getNumberOfConditions: function() {
		/// <summary> Returns the number of conditions </summary>
		/// <returns type="Number" isInteger="true"> Warning, if this number is not correct, the application _will_ crash</returns>
		return 6; // $conditionFuncs not available yet
	},

	createRunObject: function(file, cob, version) {
		/// <summary> Creation of the Fusion extension. </summary>
		/// <param name="file"> A CFile object, pointing to the object's data zone </param>
		/// <param name="cob"> A CCreateObjectInfo containing infos about the created object</param>
		/// <param name="version"> version : the version number of the object, as defined in the C code </param>
		/// <returns type="Boolean"> Always false, as it is unused. </returns>

		// Use the "file" parameter to call the CFile object, and
		// gather the data of the object in the order as they were saved
		// (same order as the definition of the data in the EDITDATA structure
		// of the C code).
		// Please report to the documentation for more information on the CFile object

		if (this.ho == null) {
			throw "HeaderObject not defined when needed to be.";
		}

		// DarkEdif properties are accessible as on other platforms: IsPropChecked(), GetPropertyStr(), GetPropertyNum()
		let props = new darkEdif['Properties'](this, file, version);

		this.Divisor = props['GetPropertyNum']("Divisor");
		this.Margin = props['GetPropertyNum']("Margin");
		this.Factor = this.Clamp(props['GetPropertyNum']("Factor"), 0, 100);

		this.CenterDisplay = props['IsPropChecked']("Auto Center Display");
		this.Easing = props['IsPropChecked']("Easing");
		this.HoriScrolling = props['IsPropChecked']("Horizontal Scrolling");
		this.VertScrolling = props['IsPropChecked']("Vertical Scrolling");
		this.Peytonphile = props['IsPropChecked']("Peytonphile Scrolling");

		this._marginMiddleX = this._marginMiddleY = this._dt =
		this._xSpeed = this._ySpeed = this._lastX = this._lastY = 0;
		this._dontScroll = this._savedLast = this._changed = false;

		this._resX = this.GetFrameRight() - this.GetFrameLeft();
		this._resY = this.GetFrameBottom() - this.GetFrameTop();

		this._scrollingX = this.GetVirtualWidth() / 2;
		this._scrollingXTarget = this._scrollingX;
		if (this.CenterDisplay)
			this.SetFrameCenterX(this._scrollingX);

		this._scrollingY = this.GetVirtualHeight() / 2;
		this._scrollingYTarget = this._scrollingY;
		if (this.CenterDisplay)
			this.SetFrameCenterY(this._scrollingY);

		// The return value is not used in this version of the runtime: always return false.
		return false;
	},

	handleRunObject: function() {
		/// <summary> This function is called at every loop of the game. You have to perform
		/// in it all the tasks necessary for your object to function. </summary>
		/// <returns type="Number"> One of two options:
		///							   0 : this function will be called during the next loop
		/// CRunExtension.REFLAG_ONESHOT : this function will not be called anymore,
		///								   unless this.reHandle() is called. </returns>
		
		this._resX = this.GetFrameRight() - this.GetFrameLeft();
		this._resY = this.GetFrameBottom() - this.GetFrameTop();

		this._dt = this.GetDelta();

		if (!this.Peytonphile)
		{
			this._marginMiddleX = this.Clamp(this.Clamp(this.GetMouseX(), this.GetFrameLeft(), this.GetFrameRight()) - this.GetFrameLeft(), ((this._resX / 2) - (this.Margin / 2)), ((this._resX / 2) + (this.Margin / 2)));
			this._marginMiddleY = this.Clamp(this.Clamp(this.GetMouseY(), this.GetFrameTop(), this.GetFrameBottom()) - this.GetFrameTop(), ((this._resY / 2) - (this.Margin / 2)), ((this._resY / 2) + (this.Margin / 2)));
			this._xSpeed = 0;
			this._ySpeed = 0;
		}

		if (!this._dontScroll)
			this._changed = false;

		if (!this._dontScroll && this.HoriScrolling && !this.Peytonphile)
			this._xSpeed = ((this.Clamp(this.GetMouseX(), this.GetFrameLeft(), this.GetFrameRight()) - this.GetFrameLeft()) - this._marginMiddleX) / this.Divisor;

		if (!this._dontScroll && this.VertScrolling && !this.Peytonphile)
			this._ySpeed = ((this.Clamp(this.GetMouseY(), this.GetFrameTop(), this.GetFrameBottom()) - this.GetFrameTop()) - this._marginMiddleY) / (this.Divisor + 0.0) * ((this._resX + 0.0) / this._resY);

		if (!this.Peytonphile && this.HoriScrolling)
			this._scrollingXTarget = this.Clamp((this._scrollingXTarget + (this._xSpeed * this._dt)), (this._resX / 2), (this.GetVirtualWidth() - (this._resX / 2)));

		if (!this.Peytonphile && this.VertScrolling)
			this._scrollingYTarget = this.Clamp((this._scrollingYTarget + (this._ySpeed * this._dt)), (this._resY / 2), (this.GetVirtualHeight() - (this._resY / 2)));

		if (!this.Easing && this.HoriScrolling && !this._dontScroll)
		{
			this._scrollingX = this._scrollingXTarget;
			if (this.CenterDisplay)
				this.SetFrameCenterX(this._scrollingX);
		}

		if (!this.Easing && this.VertScrolling && !this._dontScroll)
		{
			this._scrollingY = this._scrollingYTarget;
			if (this.CenterDisplay)
				this.SetFrameCenterY(this._scrollingY);
		}

		if (this.Easing && this.HoriScrolling && !this._dontScroll)
		{
			this._scrollingX = this._scrollingX + (this._scrollingXTarget - this._scrollingX) * ((this.Factor / 100.0) * this._dt);
			if (this.CenterDisplay)
				this.SetFrameCenterX(this._scrollingX);
		}

		if (this.Easing && this.VertScrolling && !this._dontScroll)
		{
			this._scrollingY = this._scrollingY + (this._scrollingYTarget - this._scrollingY) * ((this.Factor / 100.0) * this._dt);
			if (this.CenterDisplay)
				this.SetFrameCenterY(this._scrollingY);
		}

		if (this.Easing && this._dontScroll)
		{
			if (!this._savedLast)
			{
				this._lastX = this.GetFrameLeft() + (this._resX / 2);
				this._lastY = this.GetFrameTop() + (this._resY / 2);
			}
			this._savedLast = true;
		}
		else
			this._savedLast = false;

		if (this.Easing && this._dontScroll && !this._changed)
		{
			if (this.GetFrameLeft() + (this._resX / 2) != this._lastX || this.GetFrameTop() + (this._resY / 2) != this._lastY)
				this._changed = true;
			else
			{
				if (this.HoriScrolling)
				{
					this._scrollingX = this._scrollingX + (this._scrollingXTarget - this._scrollingX) * ((this.Factor / 100.0) * this._dt);
					if (this.CenterDisplay)
						this.SetFrameCenterX(this._scrollingX);
				}

				if (this.VertScrolling)
				{
					this._scrollingY = this._scrollingY + (this._scrollingYTarget - this._scrollingY) * ((this.Factor / 100.0) * this._dt);
					if (this.CenterDisplay)
						this.SetFrameCenterY(this._scrollingY);
				}

				this._lastX = this.GetFrameLeft() + (this._resX / 2);
				this._lastY = this.GetFrameBottom() + (this._resY / 2);
			}
		}

		if (this.Peytonphile)
		{
			this._xSpeed = 0;
			this._ySpeed = 0;

			if (!this._dontScroll && this.HoriScrolling)
				this._scrollingXTarget = this.Clamp((this.GetVirtualWidth() / 2) + (((this.GetMouseX() - ((this.GetVirtualWidth()) / 2))) * ((this.GetVirtualWidth() - (this._resX + 0.0)) / this.GetVirtualWidth())), (this._resX / 2), this.GetVirtualWidth() - (this._resX / 2));

			if (!this._dontScroll && this.VertScrolling)
				this._scrollingYTarget = this.Clamp((this.GetVirtualHeight() / 2) + (((this.GetMouseY() - ((this.GetVirtualHeight()) / 2))) * ((this.GetVirtualHeight() - (this._resY + 0.0)) / this.GetVirtualHeight())), (this._resY / 2), this.GetVirtualHeight() - (this._resY / 2));
		}

		return 0;
	},

	condition: function(num, cnd) {
		/// <summary> Called when a condition of this object is tested. </summary>
		/// <param name="num" type="Number" integer="true"> The number of the condition; 0+. </param>
		/// <param name="cnd" type="CCndExtension"> a CCndExtension object, allowing you to retreive the parameters
		//			of the condition. </param>
		/// <returns type="Boolean"> True if the condition is currently true. </returns>

		const func = this.$conditionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised condition ID " + (~~num) + " passed to DarkEdif Template.";
		}

		// Note: New Direction parameter is not supported by this, add a workaround based on condition and parameter index;
		// SDL Joystick's source has an example.
		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = cnd.getParamExpString(this.rh, i);
		}

		return func.apply(this, args);
	},
	action: function(num, act) {
		/// <summary> Called when an action of this object is executed </summary>
		/// <param name="num" type="Number"> The ID/number of the action, as defined by
		///		its array index. </param>
		/// <param name="act" type="CActExtension"> A CActExtension object, allowing you to
		///		retrieve the parameters of the action </param>

		const func = this.$actionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised action ID " + (~~num) + " passed to DarkEdif Template.";
		}

		// Note: New Direction parameter is not supported by this, add a workaround based on action and parameter index;
		// SDL Joystick's source has an example.
		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = act.getParamExpression(this.rh, i);
		}

		func.apply(this, args);
	},
	expression: function(num) {
		/// <summary> Called during the evaluation of an expression. </summary>
		/// <param name="num" type="Number"> The ID/number of the expression. </param>
		/// <returns> The result of the calculation, a number or a string </returns>

		// Note that it is important that your expression function asks for
		// each and every one of the parameters of the function, each time it is
		// called. The runtime will crash if you miss parameters.

		const func = this.$expressionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised expression ID " + (~~num) + " passed to DarkEdif Template.";
		}

		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = this.ho.getExpParam();
		}

		return func.apply(this, args);
	},
	GetFrameRight: function()
	{
		var r = this.rh.rhWindowX;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayX;
		r += this.rh.rh3WindowSx;
		if (r > this.rh.rhLevelSx)
			r = this.rh.rhLevelSx;
		return r;
	},
	GetFrameLeft: function()
	{
		var r = this.rh.rhWindowX;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayX;
		if (r < 0)
			r = 0;
		return r;
	},
	GetFrameBottom: function()
	{
		var r = this.rh.rhWindowY;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayY;
		r += this.rh.rh3WindowSy;
		if (r > this.rh.rhLevelSy)
			r = this.rh.rhLevelSy;
		return r;
	},
	GetFrameTop: function()
	{
		var r = this.rh.rhWindowY;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayY;
		if (r < 0)
			r = 0;
		return r;
	},
	SetFrameCenterX: function(centerX)
	{
		this.rh.setDisplay(centerX, 0, -1, 1)
	},
	SetFrameCenterY: function(centerY)
	{
		this.rh.setDisplay(0, centerY, -1, 2)
	},
	Clamp: function(value, min, max)
	{
		if (value < min)
			value = min;
		if (value > max)
			value = max;
		return value;
	},
	GetMouseX: function()
	{
		return this.rh.rh2MouseX;
	},
	GetMouseY: function()
	{
		return this.rh.rh2MouseY;
	},
	GetDelta: function()
	{
		return this.rh.rh4MvtTimerCoef;
	},
	GetVirtualWidth: function()
	{
		return this.rh.rhFrame.leVirtualRect.right;
	},
	GetVirtualHeight: function()
	{
		return this.rh.rhFrame.leVirtualRect.bottom;
	}

	// No comma for the last function : the Google compiler
	// we use for final projects does not accept it
});
