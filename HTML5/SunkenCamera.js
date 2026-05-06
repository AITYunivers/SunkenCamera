/*
	This SunkenCamera Fusion extension HTML5 port is under MIT license.

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
globalThis['darkEdif'] = (globalThis['darkEdif'] && globalThis['darkEdif'].sdkVersion >= 20) ? globalThis['darkEdif'] :
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
	this.sdkVersion = 20;
	this.checkSupportsSDKVersion = function (sdkVer) {
		if (sdkVer < 16 || sdkVer > 20) {
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
		// if prop set v2, then uint64 editor checkbox ptr
		// then checkbox list, one bit per checkbox, including non-checkbox properties
		// so skip numProps / 8 bytes
		// then moving to Data list:
		// size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

		let bytes = edPtrFile.ccfBytes.slice(edPtrFile.pointer);
		
		edPtrFile.skipBytes(ext.ho.privateData - 20); // sub size of eHeader; edPtrFile won't start with eHeader
		const verBuff = new Uint8Array(edPtrFile.readBuffer(4));
		const verStr = String.fromCharCode.apply('', verBuff.reverse());
		let propVer;
		if (verStr == 'DAR2') {
			propVer = 2;
		} else if (verStr == 'DAR1') {
			propVer = 1;
		} else {
			throw "Version string " + verStr + " unknown. Did you restore the file offset?";
		}
		// Pull out hash, hashTypes, numProps, pad, sizeBytes, visibleEditorProps
		let header = new Uint8Array(edPtrFile.readBuffer(4 + 4 + 2 + 2 + 4 + (propVer > 1 ? 8 : 0)));
		let headerDV = new DataView(header.buffer);
		this.numProps = headerDV.getUint16(4 + 4, true); // Skip past hash and hashTypes
		this.sizeBytes = headerDV.getUint32(4 + 4 + 4, true); // skip past numProps and pad

		let editData = edPtrFile.readBuffer(
			this.sizeBytes -
			// skip area between eHeader -> Props
			(ext.ho.privateData - 20) -
			// Skip DarkEdif header
			header.byteLength
		);
		this.chkboxes = editData.slice(0, Math.ceil(this.numProps / 8));
		let that = this;
		let IsComboBoxProp = function(propTypeID) {
			// PROPTYPE_COMBOBOX, PROPTYPE_COMBOBOXBTN, PROPTYPE_ICONCOMBOBOX
			return propTypeID == 7 || propTypeID == 20 || propTypeID == 24;
		};
		let RuntimePropSet = function(data) {
			let rsDV = new DataView(data.propData.buffer);
			let rs = /* RuntimePropSet */ { 
				// Always 'S', compared with 'L' for non-set list.
				setIndicator: String.fromCharCode(rsDV.getUint8(0)),
				// Number of repeats of this set, 1 is minimum and means one of this set
				numRepeats: rsDV.getUint16(1, true),
				// Property that ends this set's data, as a JSON index, inclusive
				lastSetJSONPropIndex: rsDV.getUint16(1 + 2, true),
				// First property that begins this set's data, as a JSON index
				firstSetJSONPropIndex: rsDV.getUint16(1 + (2 * 2), true),
				// Name property JSON index that will appear in list when switching set entry
				setNameJSONPropIndex: rsDV.getUint16(1 + (2 * 3), true),
				// Current set index selected (0+), present at runtime too, but not used there
				getIndexSelected: function() {
					return rsDV.getUint16(1 + (2 * 4), true);
				},
				setIndexSelected: function(i) {
					rsDV.setUint16(1 + (2 * 4), i, true);
				},
				// Set name, as specified in JSON. Don't confuse with user-specified set name.
				setName: that.textDecoder.decode(data.propData.slice(1 + (2 * 5))),
			};
			if (rs.setIndicator != 'S')
				throw "Not a prop set!";
			return rs;
		};
		let GetPropertyIndex = function(chkIDOrName) {
			if (propVer > 1) {
				let jsonIdx = -1;
				if (typeof chkIDOrName == 'number') {
					const p = that.props.find(function(p) { return p.index == chkIDOrName; });
					if (p == null) {
						throw "Invalid property name \"" + chkIDOrName + "\"";
					}
					jsonIdx = p.propJSONIndex;
				} else {
					const p = that.props.find(function(p) { return p.propName == chkIDOrName; });
					if (p == null) {
						throw "Invalid property name \"" + chkIDOrName + "\"";
					}
					jsonIdx = p.propJSONIndex;
				}
				// Look up prop index from JSON index - DarkEdif::Properties::PropIdxFromJSONIdx
				let data = that.props[0], i = 0;
				while (data.propJSONIndex != jsonIdx) {
					if (i >= that.numProps) {
						throw "Couldn't find property of JSON ID " + jsonIdx + ", hit property " + i + " of " + that.numProps + " stored.\n";
					}
					if (IsComboBoxProp(data.propTypeID) && String.fromCharCode(data.propData[0]) == 'S') {
						let rs = new RuntimePropSet(data);
						let rsContainer = data;
						// We're beyond all of this set's JSON range: skip past all repeats
						if (jsonIdx > rs.lastSetJSONPropIndex) {
							while (data.propJSONIndex != rs.lastSetJSONPropIndex) {
								data = that.props[i++];
							}
							rs = rsContainer = null;
						}
						// It's within this set's range
						else if (jsonIdx >= rs.firstSetJSONPropIndex && jsonIdx <= rs.lastSetJSONPropIndex) {
							if (rs.getIndexSelected() > 0) {
								for (let j = 0; ;) {
									data = that.props[++i];
									
									// Skip until end of this entry, then move to next prop
									if (data.propJSONIndex == rs.lastSetJSONPropIndex) {
										if (++j == rs.getIndexSelected()) {
											data = that.props[++i];
											break;
										}
									}
								}
								continue;
							} else {
								data = that.props[++i];
								continue;
							}
						}
						// else it's not in this set: continue to standard loop
						else {
							rs = rsContainer = null;
						}
					}
					
					data = that.props[++i];
				}
				return data.index;
			}
			if (typeof chkIDOrName == 'number') {
				if (that.numProps <= chkIDOrName) {
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
			];
			if (textPropIDs.indexOf(prop.propTypeID) != -1 || IsComboBoxProp(prop.propTypeID)) {
				// Prop ver 2 added repeating prop sets
				if (propVer == 2 && IsComboBoxProp(prop.propTypeID)) {
					const setIndicator = String.fromCharCode(prop.propData[0]);
					if (setIndicator == 'L') {
						return that.textDecoder.decode(prop.propData.slice(1));
					} else if (setIndicator == 'S') {
						throw "Property " + prop.propName + " is not textual.";
					}
					throw "Property " + prop.propName + " is not a valid list property.";
				}
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
		this['GetPropertyImageID'] = function(chkIDOrName, imgID) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 23) { // PROPTYPE_IMAGELIST
				throw "Property " + prop.propName + " is not an image list.";
			}
			
			if ((~~imgID != imgID) || imgID < 0) {
				throw "Image index " + imgID + " is invalid.";
			}
			const dv = new DataView(prop.propData.buffer);
			if (imgID >= dv.getUint16(0, true)) {
				return -1;
			}
			
			return imgID.getUint16(2 * (1 + idx), true)
		};
		this['GetPropertyNumImages'] = function(chkIDOrName, imgID) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 23) { // PROPTYPE_IMAGELIST
				throw "Property " + prop.propName + " is not an image list.";
			}
			
			return new DataView(prop.propData.buffer).getUint16(0, true);
		};
		this['GetSizeProperty'] = function(chkIDOrName) {
			const idx = GetPropertyIndex(chkIDOrName);
			if (idx == -1) {
				return -1;
			}
			const prop = that.props[idx];
			if (prop.propTypeID != 8) { // PROPTYPE_SIZE
				throw "Property " + prop.propName + " is not an size property.";
			}
			
			const dv = new DataView(prop.propData.buffer);
			return { width: dv.getInt32(0, true), height: dv.getInt32(4, true) };
		};

		this['PropSetIterator'] = this.PropSetIterator = function(nameListJSONIdx, numSkippedSetsBefore, runSetEntry, props) {
			this.nameListJSONIdx = nameListJSONIdx;
			this.numSkippedSetsBefore = numSkippedSetsBefore;
			this.props = that.props;
			this.runSetEntry = runSetEntry;
			
			this.runPropSet = new RuntimePropSet(runSetEntry);
			this.runPropSet.setIndexSelected(0);
			this.firstIt = true;
			let thatToo = this;
			this.next = function() {
				// next() is called for first iterator
				if (thatToo.firstIt) {
					thatToo.firstIt = false;
				} else {
					thatToo.runPropSet.setIndexSelected(thatToo.runPropSet.getIndexSelected() + 1);
				}
				return {
					value: thatToo.runPropSet.getIndexSelected(),
					done: thatToo.runPropSet.getIndexSelected() >= thatToo.runPropSet.numRepeats
				};
			};
			this[Symbol.iterator] = function () { return this; };
		};
		this['LoopPropSet'] = this.LoopPropSet = function(setName, numSkips = 0) {
			let d;
			for (let i = 0, j = 0; i < that.numProps; ++i) {
				d = that.props[i];
				if (IsComboBoxProp(d.propTypeID) && String.fromCharCode(d.propData[0]) == 'S') {
					if (new RuntimePropSet(d).setName == setName && ++j > numSkips)
						return new that.PropSetIterator(i, j - 1, d, this);
				}
			}
			throw "No set found with name " + setName + ".";
		}

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
			pt += 4;
			const propTypeID = dataDV.getUint16(pt, true);
			pt += 2;
			// propJSONIndex does not exist in Data in DarkEdif smart props ver 1, so JSON index is same as prop index
			let propJSONIndex = i;
			if (propVer == 2) {
				propJSONIndex = dataDV.getUint16(pt, true);
				pt += 2;
			}
			const propNameLength = dataDV.getUint8(pt);
			pt += 1;
			const propName = this.textDecoder.decode(new Uint8Array(data.slice(pt, pt + propNameLength)));
			pt += propNameLength;
			const propData = new Uint8Array(data.slice(pt, propEnd));

			this.props.push({ index: i, propTypeID: propTypeID, propJSONIndex: propJSONIndex, propName: propName, propData: propData });
			pt = propEnd;
		}
	};
	this['Surface'] = function(rhPtr, needBitmapFuncs, needTextFuncs, width, height, alpha) {
		if (rhPtr == null || needBitmapFuncs == null || needTextFuncs == null || width == null || height == null || alpha == null)
			throw "Invalid Surface ctor arguments";
		this.rhPtr = rhPtr;
		this.hasGeometryCapacity = needBitmapFuncs;
		this.hasTextCapacity = needTextFuncs;
		this.canvas = document.createElement("canvas");
		this.context = this.canvas.getContext("2d");
		this.altered = false;
		this.canvas.width = width;
		this.canvas.height = height;
		this.mosaic = 0;
		this.xSpot = this.ySpot = 0;
		
		let surf = this;
		this.faux = { img: surf.canvas, mosaic: 0, xSpot: 0, ySpot: 0 };
		this['FillImageWith'] = function(sf) {
			if (sf.fillType == darkEdif['SurfaceFill']['FillType']['Flat']) {
				surf.context.rect(0, 0, surf.canvas.width, surf.canvas.height);
				surf.context.fillStyle = sf.color;
				surf.context.fill();
				this.altered = true;
				return true;
			}
		};
		this['GetAndResetAltered'] = function() {
			if (!this.altered) {
				return false;
			}
			this.altered = false;
			return true;
		}
		this.ext = null;
		this['SetAsExtensionDisplay'] = function(ext) {
			surf.ext = ext;
		};
		this['BlitToFrameWithExtEffects'] = function(renderer, pt) {
			const x = this.ext.ho.hoX + (pt ? pt.x : 0),
				y = this.ext.ho.hoY + (pt ? pt.y : 0);
			let angle = 0, scaleX = 1, scaleY = 1, inkEffect = 1, inkEffectParam = 0;
			if ((this.ext.ho.hoOEFlags & CObjectCommon.OEFLAG_SPRITES) != 0) {
				angle = this.ext.ho.roc.rcAngle;
				scaleX = this.ext.ho.roc.rcScaleX;
				scaleY = this.ext.ho.roc.rcScaleY;
				inkEffect = this.ext.ho.ros.rsEffect;
				inkEffectParam = this.ext.ho.ros.rsEffectParam;
				this.faux.xSpot = this.ext.ho.hoImgXSpot;
				this.faux.ySpot = this.ext.ho.hoImgYSpot;
			}
			
			surf.context.save();
			renderer._context.save();
			renderer.renderImage(this.faux, x, y, angle, scaleX, scaleY, inkEffect, inkEffectParam);
			renderer._context.restore();
			surf.context.restore();
		};
		this.img = surf.canvas;
		
		return this;
	};
	this['SurfaceFill'] = {
		'FillType': {
			'Flat': 0
		},
		'Solid': function(color) {
			this.fillType = darkEdif['SurfaceFill']['FillType']['Flat'];
			this.color = color;
			return this;
		}
	};
	this['ColorRGB'] = function(r,g,b) {
		return `rgba(${r}, ${g}, ${b}, 1.0)`;
	};
})();

/** @constructor */
function CRunSunkenCamera() {
	/// <summary> Constructor of Fusion object. </summary>

	// DarkEdif SDK exts should have these four variables defined.
	// We need this[] and globalThis[] instead of direct because HTML5 Final Project minifies and breaks the names otherwise
	this['ExtensionVersion'] = 4; // To match C++ version
	this['SDKVersion'] = 20; // To match C++ version
	this['DebugMode'] = true;
	this['ExtensionName'] = 'SunkenCamera';

	// Can't find DarkEdif wrapper
	if (!globalThis.hasOwnProperty('darkEdif')) {
		throw "a wobbly";
	}
	globalThis['darkEdif'].checkSupportsSDKVersion(this.SDKVersion);

	// ======================================================================================================
	// Actions
	// ======================================================================================================
	this.ACT_SetDivisor = function (divisor) {
		this.divisor = divisor;
	};
	this.ACT_SetMargin = function (margin) {
		this.margin = margin;
	};
	this.ACT_SetFactor = function (factor) {
		this.factor = this.clamp(factor, 0, 100);
	};
	this.ACT_SetDisallowScrolling = function (setting) {
		this.dontScroll = setting != 0;
	};
	this.ACT_SetCenterDisplay = function (setting) {
		this.centerDisplay = setting != 0;
	};
	this.ACT_SetHoriScrolling = function (setting) {
		this.horiScrolling = setting != 0;
	};
	this.ACT_SetVertScrolling = function (setting) {
		this.vertScrolling = setting != 0;
	};
	this.ACT_SetEasing = function (setting) {
		this.easing = setting != 0;
	};
	this.ACT_SetPeytonphile = function (setting) {
		this.peytonphile = setting != 0;
	};
	this.ACT_SetPeytonphileToEdges = function (setting) {
		this.peytonphileToEdges = setting != 0;
	};
	this.ACT_SetCameraPosX = function (cameraX) {
		this.scrollingX = cameraX;
		this.scrollingXTarget = cameraX;
	};
	this.ACT_SetCameraPosY = function (cameraY) {
		this.scrollingY = cameraY;
		this.scrollingYTarget = cameraY;
	};
	this.ACT_SetCameraTargetX = function (cameraX) {
		this.scrollingXTarget = cameraX;
	};
	this.ACT_SetCameraTargetY = function (cameraY) {
		this.scrollingYTarget = cameraY;
	};
	this.ACT_FlipHorizontally = function () {
		this.horiFlipped = !this.horiFlipped;
	};
	this.ACT_FlipVertically = function () {
		this.vertFlipped = !this.vertFlipped;
	};

	// ======================================================================================================
	// Conditions
	// ======================================================================================================
	this.CND_CheckDisallowScrolling = function () {
		return this.dontScroll;
	};
	this.CND_CheckCenterDisplay = function () {
		return this.centerDisplay;
	};
	this.CND_CheckEasing = function () {
		return this.easing;
	};
	this.CND_CheckHoriScrolling = function () {
		return this.horiScrolling;
	};
	this.CND_CheckVertScrolling = function () {
		return this.vertScrolling;
	};
	this.CND_CheckPeytonphile = function () {
		return this.peytonphile;
	};
	this.CND_CheckPeytonphileToEdges = function () {
		return this.peytonphileToEdges;
	};
	this.CND_CheckHoriFlipped = function () {
		return this.horiFlipped;
	};
	this.CND_CheckVertFlipped = function () {
		return this.vertFlipped;
	};

	// =============================
	// Expressions
	// =============================
	this.EXP_GetDivisor = function () {
		return this.divisor;
	};
	this.EXP_GetMargin = function () {
		return this.margin;
	};
	this.EXP_GetFactor = function () {
		return this.factor;
	};
	this.EXP_GetXScroll = function () {
		return this.scrollingX;
	};
	this.EXP_GetYScroll = function () {
		return this.scrollingY;
	};
	this.EXP_GetXScrollTarget = function () {
		return this.scrollingXTarget;
	};
	this.EXP_GetYScrollTarget = function () {
		return this.scrollingYTarget;
	};
	this.EXP_GetXSpeed = function () {
		return this.xSpeed;
	};
	this.EXP_GetYSpeed = function () {
		return this.ySpeed;
	};

	// =============================
	// Function arrays
	// =============================

	this.$actionFuncs = [
	/* 0 */ this.ACT_SetDivisor,
	/* 1 */ this.ACT_SetMargin,
	/* 2 */ this.ACT_SetFactor,
	/* 3 */ this.ACT_SetDisallowScrolling,
	/* 4 */ this.ACT_SetEasing,
	/* 5 */ this.ACT_SetHoriScrolling,
	/* 6 */ this.ACT_SetVertScrolling,
	/* 7 */ this.ACT_SetPeytonphile,
	/* 8 */ this.ACT_SetCenterDisplay,
	/* 9 */ this.ACT_SetCameraPosX,
	/* 10 */ this.ACT_SetCameraPosY,
	/* 11 */ this.ACT_SetCameraTargetX,
	/* 12 */ this.ACT_SetCameraTargetY,
	/* 13 */ this.ACT_FlipHorizontally,
	/* 14 */ this.ACT_FlipVertically,
	/* 15 */ this.ACT_SetPeytonphileToEdges
	];
	this.$conditionFuncs = [
	/* 0 */ this.CND_CheckDisallowScrolling,
	/* 1 */ this.CND_CheckEasing,
	/* 2 */ this.CND_CheckHoriScrolling,
	/* 3 */ this.CND_CheckVertScrolling,
	/* 4 */ this.CND_CheckPeytonphile,
	/* 5 */ this.CND_CheckCenterDisplay,
	/* 6 */ this.CND_CheckHoriFlipped,
	/* 7 */ this.CND_CheckVertFlipped,
	/* 8 */ this.CND_CheckPeytonphileToEdges
	];
	this.$expressionFuncs = [
	/* 0 */ this.EXP_GetDivisor,
	/* 1 */ this.EXP_GetMargin,
	/* 2 */ this.EXP_GetFactor,
	/* 3 */ this.EXP_GetXScroll,
	/* 4 */ this.EXP_GetYScroll,
	/* 5 */ this.EXP_GetXScrollTarget,
	/* 6 */ this.EXP_GetYScrollTarget,
	/* 7 */ this.EXP_GetXSpeed,
	/* 8 */ this.EXP_GetYSpeed
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
		return 9; // $conditionFuncs not available yet
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

		this.divisor = props['GetPropertyNum'](0);
		this.margin = props['GetPropertyNum'](1);
		this.factor = this.clamp(props['GetPropertyNum'](2), 0, 100);

		this.dontScroll = props['IsPropChecked'](3);
		this.centerDisplay = props['IsPropChecked'](4);
		this.easing = props['IsPropChecked'](5);
		this.horiScrolling = props['IsPropChecked'](6);
		this.vertScrolling = props['IsPropChecked'](7);
		this.peytonphile = props['IsPropChecked'](8);
		this.peytonphileToEdges = props['IsPropChecked'](9);
		
		this.horiFlipped = props['IsPropChecked'](10);
		this.vertFlipped = props['IsPropChecked'](11);

		this.marginMiddleX = 0;
		this.marginMiddleY = 0;
		this.dt = 0;
		this.xSpeed = 0;
		this.ySpeed = 0;

		this.resX = this.getFrameRight() - this.getFrameLeft();
		this.resY = this.getFrameBottom() - this.getFrameTop();

		this.scrollingX = this.getVirtualWidth() / 2;
		this.scrollingXTarget = this.scrollingX;
		if (this.centerDisplay)
			this.setFrameCenterX(this.scrollingX);

		this.scrollingY = this.getVirtualHeight() / 2;
		this.scrollingYTarget = this.scrollingY;
		if (this.centerDisplay)
			this.setFrameCenterY(this.scrollingY);
		
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
		
		this.resX = this.getFrameRight() - this.getFrameLeft();
		this.resY = this.getFrameBottom() - this.getFrameTop();

		this.dt = this.getDelta();

		if (!this.peytonphile)
		{
			this.marginMiddleX = this.clamp(this.clamp(this.getMouseX(), this.getFrameLeft(), this.getFrameRight()) - this.getFrameLeft(), ((this.resX / 2) - (this.margin / 2)), ((this.resX / 2) + (this.margin / 2)));
			this.marginMiddleY = this.clamp(this.clamp(this.getMouseY(), this.getFrameTop(), this.getFrameBottom()) - this.getFrameTop(), ((this.resY / 2) - (this.margin / 2)), ((this.resY / 2) + (this.margin / 2)));
			this.xSpeed = 0;
			this.ySpeed = 0;
		}

		if (!this.dontScroll && this.horiScrolling && !this.peytonphile)
		{
			this.xSpeed = ((this.clamp(this.getMouseX(), this.getFrameLeft(), this.getFrameRight()) - this.getFrameLeft()) - this.marginMiddleX) / this.divisor;
			this.scrollingXTarget = this.clamp((this.scrollingXTarget + (this.xSpeed * this.dt)), (this.resX / 2), (this.getVirtualWidth() - (this.resX / 2)));
		}

		if (!this.dontScroll && this.vertScrolling && !this.peytonphile)
		{
			this.ySpeed = ((this.clamp(this.getMouseY(), this.getFrameTop(), this.getFrameBottom()) - this.getFrameTop()) - this.marginMiddleY) / this.divisor;
			this.scrollingYTarget = this.clamp((this.scrollingYTarget + (this.ySpeed * this.dt)), (this.resY / 2), (this.getVirtualHeight() - (this.resY / 2)));
		}

		if (this.peytonphile)
		{
			this.xSpeed = 0;
			this.ySpeed = 0;

			let mX = this.getMouseX() - this.margin;
			let mY = this.getMouseY() - this.margin;
			let mWidth = this.getVirtualWidth() - this.margin * 2;
			let mHeight = this.getVirtualHeight() - this.margin * 2;
			let eWidth = this.peytonphileToEdges ? this.getVirtualWidth() : mWidth;
			let eHeight = this.peytonphileToEdges ? this.getVirtualHeight() : mHeight;

			if (!this.dontScroll && this.horiScrolling)
				this.scrollingXTarget = this.clamp((eWidth / 2) + (((mX - (mWidth / 2))) * ((eWidth - (this.resX + 0.0)) / mWidth)), (this.resX / 2), eWidth - (this.resX / 2)) + (this.peytonphileToEdges ? 0 : this.margin);

			if (!this.dontScroll && this.VertScrolling)
				this.scrollingYTarget = this.clamp((eHeight / 2) + (((mY - (mHeight / 2))) * ((eHeight - (this.resY + 0.0)) / mHeight)), (this.resY / 2), eHeight - (this.resY / 2)) + (this.peytonphileToEdges ? 0 : this.margin);
		}

		if (!this.easing && this.horiScrolling)
		{
			this.scrollingX = this.scrollingXTarget;
			if (this.centerDisplay)
				this.setFrameCenterX(this.scrollingX);
		}

		if (!this.easing && this.vertScrolling)
		{
			this.scrollingY = this.scrollingYTarget;
			if (this.centerDisplay)
				this.setFrameCenterY(this.scrollingY);
		}

		if (this.easing && this.horiScrolling)
		{
			this.scrollingX = this.scrollingX + (this.scrollingXTarget - this.scrollingX) * this.clamp((this.factor / 100.0) * this.dt, 0.0, 1.0);
			if (this.centerDisplay)
				this.setFrameCenterX(this.scrollingX);
		}
		
		if (this.easing && this.vertScrolling)
		{
			this.scrollingY = this.scrollingY + (this.scrollingYTarget - this.scrollingY) * this.clamp((this.factor / 100.0) * this.dt, 0.0, 1.0);
			if (this.centerDisplay)
				this.setFrameCenterY(this.scrollingY);
		}

		return 0;
	},
	displayRunObject: function (renderer, xDraw, yDraw) {
		this.surf.BlitToFrameWithExtEffects(renderer);
	},

	condition: function(num, cnd) {
		/// <summary> Called when a condition of this object is tested. </summary>
		/// <param name="num" type="Number" integer="true"> The number of the condition; 0+. </param>
		/// <param name="cnd" type="CCndExtension"> a CCndExtension object, allowing you to retreive the parameters
		//			of the condition. </param>
		/// <returns type="Boolean"> True if the condition is currently true. </returns>

		const func = this.$conditionFuncs[~~num];
		if (func == null) {
			throw "Unrecognised condition ID " + (~~num) + " passed to SunkenCamera.";
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
			throw "Unrecognised action ID " + (~~num) + " passed to SunkenCamera.";
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
			throw "Unrecognised expression ID " + (~~num) + " passed to SunkenCamera.";
		}

		const args = new Array(func.length);
		for (let i = 0; i < args.length; ++i) {
			args[i] = this.ho.getExpParam();
		}

		return func.apply(this, args);
	},
	getFrameRight: function() {
		let r = this.rh.rhWindowX;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayX;
		r += this.rh.rh3WindowSx;
		if (r > this.rh.rhLevelSx)
			r = this.rh.rhLevelSx;
		return r;
	},
	getFrameLeft: function() {
		let r = this.rh.rhWindowX;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayX;
		if (r < 0)
			r = 0;
		return r;
	},
	getFrameBottom: function() {
		let r = this.rh.rhWindowY;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayY;
		r += this.rh.rh3WindowSy;
		if (r > this.rh.rhLevelSy)
			r = this.rh.rhLevelSy;
		return r;
	},
	getFrameTop: function() {
		let r = this.rh.rhWindowY;
		if ((this.rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
			r = this.rh.rh3DisplayY;
		if (r < 0)
			r = 0;
		return r;
	},
	setFrameCenterX: function(centerX) {
		//centerX = this.clamp(centerX - this.rh.rh3WindowSx / 2, 0, this.getVirtualWidth() - this.rh.rh3WindowSx);
		this.rh.setDisplay(centerX, 0, -1, 1);
		// Redisplay?
	},
	setFrameCenterY: function(centerY) {
		//centerY = this.clamp(centerY - this.rh.rh3WindowSy / 2, 0, this.getVirtualHeight() - this.rh.rh3WindowSy);
		this.rh.setDisplay(0, centerY, -1, 2);
		// Redisplay?
	},
	clamp: function(value, min, max) {
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	},
	getMouseX: function() {
		if (this.horiFlipped)
			return (this.resX - this.rh.rhApp.mouseX) + this.rh.rhWindowX - this.rh.rhApp.xOffset
		return this.rh.getXMouse();
	},
	getMouseY: function() {
		if (this.vertFlipped)
			return (this.resY - this.rh.rhApp.mouseY) + this.rh.rhWindowY - this.rh.rhApp.yOffset
		return this.rh.getYMouse();
	},
	getDelta: function()
	{
		return this.rh.rh4MvtTimerCoef;
	},
	getVirtualWidth: function()
	{
		return this.rh.rhFrame.leVirtualRect.right;
	},
	getVirtualHeight: function()
	{
		return this.rh.rhFrame.leVirtualRect.bottom;
	}
});
