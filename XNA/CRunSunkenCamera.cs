#if NETFX_CORE
#define SIMPLE_JSON_TYPEINFO
#endif

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.GamerServices;
using RuntimeXNA.Extensions;
using RuntimeXNA.Services;
using RuntimeXNA.RunLoop;
using RuntimeXNA.Sprites;
using RuntimeXNA.Conditions;
using RuntimeXNA.Actions;
using RuntimeXNA.Expressions;
using RuntimeXNA.Objects;
using RuntimeXNA.Params;
using RuntimeXNA.Frame;
using RuntimeXNA.OI;
using RuntimeXNA.Movements;
using RuntimeXNA.Application;
using System.Reflection;
using System.Security.Cryptography;
using System.Net;
using System.IO.IsolatedStorage;

using System.CodeDom.Compiler;
using System.Collections;
#if !SIMPLE_JSON_NO_LINQ_EXPRESSION
using System.Linq.Expressions;
#endif
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
#if SIMPLE_JSON_DYNAMIC
using System.Dynamic;
#endif
using System.Globalization;
using System.Runtime.Serialization;
using SimpleJson.Reflection;
using System.Text.RegularExpressions;

namespace RuntimeXNA.Extensions
{
    class CRunSunkenCamera : CRunExtension
    {
        public const int ExtensionVersion = 4;
        public const int SDKVersion = 20;

        private const int ACT_SetDivisor = 0;
    	private const int ACT_SetMargin = 1;
    	private const int ACT_SetFactor = 2;
    	private const int ACT_SetDisallowScrolling = 3;
    	private const int ACT_SetEasing = 4;
    	private const int ACT_SetHoriScrolling = 5;
    	private const int ACT_SetVertScrolling = 6;
    	private const int ACT_SetPeytonphile = 7;
    	private const int ACT_SetCenterDisplay = 8;
    	private const int ACT_SetCameraPosX = 9;
    	private const int ACT_SetCameraPosY = 10;
    	private const int ACT_SetCameraTargetX = 11;
    	private const int ACT_SetCameraTargetY = 12;
    	private const int ACT_FlipHorizontally = 13;
    	private const int ACT_FlipVertically = 14;
    	private const int ACT_SetPeytonphileToEdges = 15;
    	private const int CND_CheckDisallowScrolling = 0;
    	private const int CND_CheckEasing = 1;
    	private const int CND_CheckHoriScrolling = 2;
    	private const int CND_CheckVertScrolling = 3;
    	private const int CND_CheckPeytonphile = 4;
    	private const int CND_CheckCenterDisplay = 5;
    	private const int CND_CheckHoriFlipped = 6;
    	private const int CND_CheckVertFlipped = 7;
        private const int CND_CheckPeytonphileToEdges = 8;
        private const int CND_Last = 9;
    	private const int EXP_GetDivisor = 0;
    	private const int EXP_GetMargin = 1;
    	private const int EXP_GetFactor = 2;
    	private const int EXP_GetXScroll = 3;
    	private const int EXP_GetYScroll = 4;
    	private const int EXP_GetXScrollTarget = 5;
    	private const int EXP_GetYScrollTarget = 6;
    	private const int EXP_GetXSpeed = 7;
        private const int EXP_GetYSpeed = 8;

        public double Divisor;
        public double Margin;
        public double Factor;
        public bool DontScroll;
        public bool CenterDisplay;
        public bool Easing;
        public bool HoriScrolling;
        public bool VertScrolling;
        public bool Peytonphile;
        public bool PeytonphileToEdges;
        public bool HoriFlipped;
        public bool VertFlipped;

        public double MarginMiddleX;
        public double MarginMiddleY;
        public double Dt;
        public double XSpeed;
        public double YSpeed;
        public double ResX;
        public double ResY;
        public double ScrollingX;
        public double ScrollingXTarget;
        public double ScrollingY;
        public double ScrollingYTarget;

        public CRunSunkenCamera()
        {
            DarkEdif.checkSupportsSDKVersion(SDKVersion);
        }

        public override bool createRunObject(CFile file, CCreateObjectInfo cob, int version)
        {
            if (ho == null)
            {
                throw new Exception("HeaderObject not defined when needed to be.");
            }

            // DarkEdif properties are accessible as on other platforms: IsPropChecked(), GetPropertyStr(), GetPropertyNum()
            DarkEdifProperties props = DarkEdif.getProperties(this, file, version);

            Divisor = props.GetPropertyNum(0);
            Margin = props.GetPropertyNum(1);
            Factor = Clamp(props.GetPropertyNum(2), 0, 100);

            DontScroll = props.IsPropChecked(3);
            CenterDisplay = props.IsPropChecked(4);
            Easing = props.IsPropChecked(5);
            HoriScrolling = props.IsPropChecked(6);
            VertScrolling = props.IsPropChecked(7);
            Peytonphile = props.IsPropChecked(8);
            PeytonphileToEdges = props.IsPropChecked(9);

            HoriFlipped = props.IsPropChecked(10);
            VertFlipped = props.IsPropChecked(11);

            ResX = GetFrameRight() - GetFrameLeft();
            ResY = GetFrameBottom() - GetFrameTop();

            ScrollingX = ScrollingXTarget = GetVirtualWidth() / 2.0;
            if (CenterDisplay)
                SetFrameCenterX(ScrollingX);

            ScrollingY = ScrollingYTarget = GetVirtualHeight() / 2.0;
            if (CenterDisplay)
                SetFrameCenterY(ScrollingY);
            
            // The return value is not used in this version of the runtime: always return false.
            return false;
        }

        public override int handleRunObject()
        {
            ResX = GetFrameRight() - GetFrameLeft();
            ResY = GetFrameBottom() - GetFrameTop();

            Dt = GetDelta();

            if (!Peytonphile)
            {
                MarginMiddleX = Clamp(Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft(), ((ResX / 2) - (Margin / 2)), ((ResX / 2) + (Margin / 2)));
                MarginMiddleY = Clamp(Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop(), ((ResY / 2) - (Margin / 2)), ((ResY / 2) + (Margin / 2)));
                XSpeed = 0;
                YSpeed = 0;
            }

            if (!DontScroll && HoriScrolling && !Peytonphile)
            {
                XSpeed = ((Clamp(GetMouseX(), GetFrameLeft(), GetFrameRight()) - GetFrameLeft()) - MarginMiddleX) / Divisor;
                ScrollingXTarget = Clamp((ScrollingXTarget + (XSpeed * Dt)), (ResX / 2), (GetVirtualWidth() - (ResX / 2)));
            }

            if (!DontScroll && VertScrolling && !Peytonphile)
            {
                YSpeed = ((Clamp(GetMouseY(), GetFrameTop(), GetFrameBottom()) - GetFrameTop()) - MarginMiddleY) / Divisor;
                ScrollingYTarget = Clamp((ScrollingYTarget + (YSpeed * Dt)), (ResY / 2), (GetVirtualHeight() - (ResY / 2)));
            }

            if (Peytonphile)
            {
                XSpeed = 0;
                YSpeed = 0;

                double mX = GetMouseX() - Margin;
                double mY = GetMouseY() - Margin;
                double mWidth = GetVirtualWidth() - Margin * 2;
                double mHeight = GetVirtualHeight() - Margin * 2;
                double eWidth = PeytonphileToEdges ? GetVirtualWidth() : mWidth;
                double eHeight = PeytonphileToEdges ? GetVirtualHeight() : mHeight;

                if (!DontScroll && HoriScrolling)
                    ScrollingXTarget = Clamp((eWidth / 2) + (((mX - (mWidth / 2))) * ((eWidth - (ResX + 0.0)) / mWidth)), (ResX / 2), eWidth - (ResX / 2)) + (PeytonphileToEdges ? 0 : Margin);

                if (!DontScroll && VertScrolling)
                    ScrollingYTarget = Clamp((eHeight / 2) + (((mY - (mHeight / 2))) * ((eHeight - (ResY + 0.0)) / mHeight)), (ResY / 2), eHeight - (ResY / 2)) + (PeytonphileToEdges ? 0 : Margin);
            }

            if (!Easing && HoriScrolling)
            {
                ScrollingX = ScrollingXTarget;
                if (CenterDisplay)
                    SetFrameCenterX(ScrollingX);
            }

            if (!Easing && VertScrolling)
            {
                ScrollingY = ScrollingYTarget;
                if (CenterDisplay)
                    SetFrameCenterY(ScrollingY);
            }

            if (Easing && HoriScrolling)
            {
                ScrollingX = ScrollingX + (ScrollingXTarget - ScrollingX) * Clamp((Factor / 100.0) * Dt, 0.0, 1.0);
                if (CenterDisplay)
                    SetFrameCenterX(ScrollingX);
            }

            if (Easing && VertScrolling)
            {
                ScrollingY = ScrollingY + (ScrollingYTarget - ScrollingY) * Clamp((Factor / 100.0) * Dt, 0.0, 1.0);
                if (CenterDisplay)
                    SetFrameCenterY(ScrollingY);
            }

            return 0;
        }

        // Actions
        // -------------------------------------------------
        public override void action(int num, CActExtension act)
        {
            double param = num != ACT_FlipHorizontally && num != ACT_FlipVertically ? act.getParamExpDouble(rh, 0) : 0;
            switch(num)
            {
                case ACT_SetDivisor:
                    actSetDivisor(param);
                    break;
                case ACT_SetMargin:
                    actSetMargin(param);
                    break;
                case ACT_SetFactor:
                    actSetFactor(param);
                    break;
                case ACT_SetDisallowScrolling:
                    actSetDisallowScrolling(param);
                    break;
                case ACT_SetEasing:
                    actSetEasing(param);
                    break;
                case ACT_SetHoriScrolling:
                    actSetHoriScrolling(param);
                    break;
                case ACT_SetVertScrolling:
                    actSetVertScrolling(param);
                    break;
                case ACT_SetPeytonphile:
                    actSetPeytonphile(param);
                    break;
                case ACT_SetCenterDisplay:
                    actSetCenterDisplay(param);
                    break;
                case ACT_SetCameraPosX:
                    actSetCameraPosX(param);
                    break;
                case ACT_SetCameraPosY:
                    actSetCameraPosY(param);
                    break;
                case ACT_SetCameraTargetX:
                    actSetCameraTargetX(param);
                    break;
                case ACT_SetCameraTargetY:
                    actSetCameraTargetY(param);
                    break;
                case ACT_FlipHorizontally:
                    actFlipHorizontally();
                    break;
                case ACT_FlipVertically:
                    actFlipVertically();
                    break;
                case ACT_SetPeytonphileToEdges:
                    actSetPeytonphileToEdges(param);
                    break;
			}
        }

        public void actSetDivisor(double divisor)
        {
            Divisor = divisor;
        }

        public void actSetMargin(double margin)
        {
            Margin = margin;
        }

        public void actSetFactor(double factor)
        {
            Factor = Clamp(factor, 0, 100);
        }

        public void actSetDisallowScrolling(double setting)
        {
            DontScroll = setting != 0;
        }

        public void actSetCenterDisplay(double setting)
        {
            CenterDisplay = setting != 0;
        }

        public void actSetHoriScrolling(double setting)
        {
            HoriScrolling = setting != 0;
        }

        public void actSetVertScrolling(double setting)
        {
            VertScrolling = setting != 0;
        }

        public void actSetEasing(double setting)
        {
            Easing = setting != 0;
        }

        public void actSetPeytonphile(double setting)
        {
            Peytonphile = setting != 0;
        }

        public void actSetPeytonphileToEdges(double setting)
        {
            PeytonphileToEdges = setting != 0;
        }

        public void actSetCameraPosX(double cameraX)
        {
            ScrollingX = cameraX;
            ScrollingXTarget = cameraX;
        }

        public void actSetCameraPosY(double cameraY)
        {
            ScrollingY = cameraY;
            ScrollingYTarget = cameraY;
        }

        public void actSetCameraTargetX(double cameraX)
        {
            ScrollingXTarget = cameraX;
        }

        public void actSetCameraTargetY(double cameraY)
        {
            ScrollingYTarget = cameraY;
        }

        public void actFlipHorizontally()
        {
            HoriFlipped = !HoriFlipped;
        }

        public void actFlipVertically()
        {
            VertFlipped = !VertFlipped;
        }

        // Conditions
        // -------------------------------------------------
        public override int getNumberOfConditions()
        {
            return CND_Last;
        }

        public override bool condition(int num, CCndExtension cnd)
        {
            switch (num)
            {
                case CND_CheckDisallowScrolling:
                    return cndCheckDisallowScrolling();
                case CND_CheckEasing:
                    return cndCheckEasing();
                case CND_CheckHoriScrolling:
                    return cndCheckHoriScrolling();
                case CND_CheckVertScrolling:
                    return cndCheckVertScrolling();
                case CND_CheckPeytonphile:
                    return cndCheckPeytonphile();
                case CND_CheckCenterDisplay:
                    return cndCheckCenterDisplay();
                case CND_CheckHoriFlipped:
                    return cndCheckHoriFlipped();
                case CND_CheckVertFlipped:
                    return cndCheckVertFlipped();
                case CND_CheckPeytonphileToEdges:
                    return cndCheckPeytonphileToEdges();
            }
            return false;
        }

        public bool cndCheckDisallowScrolling()
        {
            return DontScroll;
        }

        public bool cndCheckCenterDisplay()
        {
            return CenterDisplay;
        }

        public bool cndCheckEasing()
        {
            return Easing;
        }

        public bool cndCheckHoriScrolling()
        {
            return HoriScrolling;
        }

        public bool cndCheckVertScrolling()
        {
            return VertScrolling;
        }

        public bool cndCheckPeytonphile()
        {
            return Peytonphile;
        }

        public bool cndCheckPeytonphileToEdges()
        {
            return PeytonphileToEdges;
        }

        public bool cndCheckHoriFlipped()
        {
            return HoriFlipped;
        }

        public bool cndCheckVertFlipped()
        {
            return VertFlipped;
        }

        // Expressions
        // --------------------------------------------
        public override CValue expression(int num)
        {
            switch (num)
            {
                case EXP_GetDivisor:
                    return new CValue(expGetDivisor());
                case EXP_GetMargin:
                    return new CValue(expGetMargin());
                case EXP_GetFactor:
                    return new CValue(expGetFactor());
                case EXP_GetXScroll:
                    return new CValue(expGetXScroll());
                case EXP_GetYScroll:
                    return new CValue(expGetYScroll());
                case EXP_GetXScrollTarget:
                    return new CValue(expGetXScrollTarget());
                case EXP_GetYScrollTarget:
                    return new CValue(expGetYScrollTarget());
                case EXP_GetXSpeed:
                    return new CValue(expGetXSpeed());
                case EXP_GetYSpeed:
                    return new CValue(expGetYSpeed());
            }
            return null;
        }

        public double expGetDivisor()
        {
            return Divisor;
        }

        public double expGetMargin()
        {
            return Margin;
        }

        public double expGetFactor()
        {
            return Factor;
        }

        public double expGetXScroll()
        {
            return ScrollingX;
        }

        public double expGetYScroll()
        {
            return ScrollingY;
        }

        public double expGetXScrollTarget()
        {
            return ScrollingXTarget;
        }

        public double expGetYScrollTarget()
        {
            return ScrollingYTarget;
        }

        public double expGetXSpeed()
        {
            return XSpeed;
        }

        public double expGetYSpeed()
        {
            return YSpeed;
        }

        public int GetFrameRight()
        {
            int r = rh.rhWindowX;
            if ((rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
                r = rh.rh3DisplayX;
            r += rh.rh3WindowSx;
            if (r > rh.rhLevelSx)
                r = rh.rhLevelSx;
            return r;
        }

        public int GetFrameLeft()
        {
            int r = rh.rhWindowX;
            if ((rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
                r = rh.rh3DisplayX;
            if (r < 0)
                r = 0;
            return r;
        }

        public int GetFrameBottom()
        {
            int r = rh.rhWindowY;
            if ((rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
                r = rh.rh3DisplayY;
            r += rh.rh3WindowSy;
            if (r > rh.rhLevelSy)
                r = rh.rhLevelSy;
            return r;
        }

        public int GetFrameTop()
        {
            int r = rh.rhWindowY;
            if ((rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
                r = rh.rh3DisplayY;
            if (r < 0)
                r = 0;
            return r;
        }

        public void SetFrameCenterX(double centerX)
        {
            //centerX = Clamp(centerX - rh.rh3WindowSx / 2, 0, GetVirtualWidth() - rh.rh3WindowSx);
            rh.setDisplay((int)centerX, 0, -1, 1);
            // Redisplay?
        }

        public void SetFrameCenterY(double centerY)
        {
            //centerY = Clamp(centerY - rh.rh3WindowSy / 2, 0, GetVirtualHeight() - rh.rh3WindowSy);
            this.rh.setDisplay(0, (int)centerY, -1, 2);
            // Redisplay?
        }

        public double Clamp(double value, double min, double max)
        {
            return Math.Max(Math.Min(value, max), min);
        }

        public int GetMouseX()
        {
            if (HoriFlipped)
            {
                int rh2MouseX = ((int)ResX - rh.mouseX) + rh.rhWindowX;
                if (rh.rhApp.parentApp != null)
                {
                    rh2MouseX -= rh.rhApp.xOffset;
                }
                return rh2MouseX;
            }
            return this.rh.getXMouse();
        }

        public int GetMouseY()
        {
            if (VertFlipped)
            {
                int rh2MouseY = ((int)ResY - rh.mouseY) + rh.rhWindowY;
                if (rh.rhApp.parentApp != null)
                {
                    rh2MouseY -= rh.rhApp.yOffset;
                }
                return rh2MouseY;
            }
            return this.rh.getYMouse();
        }

        public double GetDelta()
        {
            return rh.rh4MvtTimerCoef;
        }

        public int GetVirtualWidth()
        {
            return rh.rhFrame.leVirtualRect.right;
        }

        public int GetVirtualHeight()
        {
            return rh.rhFrame.leVirtualRect.bottom;
        }

        public class DarkEdifProperty
        {
            public uint index;
            public uint propTypeID;
            public uint propJSONIndex;
            public string propName;
            public BinaryReader propData;

            public DarkEdifProperty(uint index, uint propTypeID, uint propJSONIndex, string propName, BinaryReader propData)
            {
                this.index = index;
                this.propTypeID = propTypeID;
                this.propJSONIndex = propJSONIndex;
                this.propName = propName;
                this.propData = propData;
            }
        }

        public class DarkEdifPropSet
        {
            public string setIndicator;
            public uint numRepeats;
            public uint lastSetJSONPropIndex;
            public uint firstSetJSONPropIndex;
            public uint setNameJSONPropIndex;
            public string setName;
        
            private BinaryReader rsDV;

            public DarkEdifPropSet(BinaryReader rsDV)
            {
                // Always 'S', compared with 'L' for non-set list.
                setIndicator = ((char)rsDV.ReadByte()).ToString();
                // Number of repeats of this set, 1 is minimum and means one of this set
                numRepeats = rsDV.ReadUInt16();
                // Property that ends this set's data, as a JSON index, inclusive
                lastSetJSONPropIndex = rsDV.ReadUInt16();
                // First property that begins this set's data, as a JSON index
                firstSetJSONPropIndex = rsDV.ReadUInt16();
                // Name property JSON index that will appear in list when switching set entry
                setNameJSONPropIndex = rsDV.ReadUInt16();
                // Set name, as specified in JSON. Don't confuse with user-specified set name.
                long bytesAvailable = rsDV.BaseStream.Length - rsDV.BaseStream.Position;
                setName = Encoding.UTF8.GetString(rsDV.ReadBytes((int)bytesAvailable));

                this.rsDV = rsDV;
            }

            public uint getIndexSelected()
            {
                rsDV.BaseStream.Position = 1 + (2 * 4);
                return rsDV.ReadUInt16();
            }

            public void setIndexSelected(uint i)
            {
                using (BinaryWriter writer = new BinaryWriter(rsDV.BaseStream))
                {
                    writer.BaseStream.Position = 1 + (2 * 4);
                    writer.Write((ushort)i);
                }
            }
        }

        public class DarkEdifProperties
        {
            private uint numProps = 0;
            private uint sizeBytes = 0;
            private int propVer = 0;
            private byte[] chkboxes;
            private List<DarkEdifProperty> props;

            private static int GetFileLength(CFile file)
            {
                var data = (byte[])typeof(CFile)
                    .GetField("data", BindingFlags.NonPublic | BindingFlags.Instance)
                    .GetValue(file);

                return data.Length;
            }

            public DarkEdifProperties(CRunExtension ext, CFile edPtrFile, int extVersion)
            {
                // DarkEdif SDK stores offset of DarkEdif props away from start of EDITDATA inside private data.
                // eHeader is 20 bytes, so this should be 20+ bytes.
                if (ext.ho.privateData < 20)
                {
                    throw new Exception("Not smart properties - eHeader missing?");
                }
                // DarkEdif SDK header read:
                // header uint32, hash uint32, hashtypes uint32, numprops uint16, pad uint16, sizeBytes uint32 (includes whole EDITDATA)
                // if prop set v2, then uint64 editor checkbox ptr
                // then checkbox list, one bit per checkbox, including non-checkbox properties
                // so skip numProps / 8 bytes
                // then moving to Data list:
                // size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

                int oldPos = edPtrFile.pointer;
                byte[] bytes = new byte[GetFileLength(edPtrFile)];
                edPtrFile.pointer = 0;
                edPtrFile.read(bytes);
                edPtrFile.pointer = oldPos;

                edPtrFile.skipBytes(ext.ho.privateData - 20); // sub size of eHeader; edPtrFile won't start with eHeader
                byte[] verBuff = edPtrFile.readArray(4);
		        string verStr = "";
                for (int i = verBuff.Length - 1; i >= 0; i--)
                {
                    verStr += ((char)verBuff[i]).ToString();
                }
                if (verStr == "DAR2")
                {
                    propVer = 2;
                }
                else if (verStr == "DAR1")
                {
                    propVer = 1;
                }
                else
                {
                    throw new Exception("Version string " + verStr + " unknown. Did you restore the file offset?");
                }
                // Pull out hash, hashTypes, numProps, pad, sizeBytes, visibleEditorProps
		        BinaryReader header = new BinaryReader(new MemoryStream(edPtrFile.readArray(4 + 4 + 2 + 2 + 4 + (propVer > 1 ? 8 : 0))));
                header.BaseStream.Position = 4 + 4; // Skip past hash and hashTypes
                numProps = header.ReadUInt16();
                header.BaseStream.Position = 4 + 4 + 4; // skip past numProps and pad
                sizeBytes = header.ReadUInt32();

                BinaryReader editData = new BinaryReader(new MemoryStream(edPtrFile.readArray(
                    (int)sizeBytes -
                    // skip eHeader
                    ext.ho.privateData -
                    // cursor offset
                    4 -
                    // Skip DarkEdif header
                    (int)header.BaseStream.Length
                )));
                editData.BaseStream.Position = 0;
		        chkboxes = editData.ReadBytes((int)Math.Ceiling(numProps / 8.0));

                props = new List<DarkEdifProperty>();
                editData.BaseStream.Position = chkboxes.Length;
                long bytesAvailable = editData.BaseStream.Length - editData.BaseStream.Position;
                BinaryReader data = new BinaryReader(new MemoryStream(editData.ReadBytes((int)bytesAvailable)));

                // Dont need TextDecoder

                uint propSize = 0;
                uint propEnd = 0;
                data.BaseStream.Position = 0; // pt
                for (uint i = 0; i < numProps; ++i)
                {
                    propSize = data.ReadUInt32();
                    propEnd = (uint)data.BaseStream.Position - 4 + propSize;
                    uint propTypeID = data.ReadUInt16();
                    // propJSONIndex does not exist in Data in DarkEdif smart props ver 1, so JSON index is same as prop index
			        uint propJSONIndex = i;
                    if (propVer == 2)
                    {
				        propJSONIndex = data.ReadUInt16();
                    }
                    int propNameLength = data.ReadByte();

                    string propName = Encoding.UTF8.GetString(data.ReadBytes(propNameLength));
                    BinaryReader propData = new BinaryReader(new MemoryStream(data.ReadBytes((int)(propEnd - data.BaseStream.Position))));

                    props.Add(new DarkEdifProperty(i, propTypeID, propJSONIndex, propName, propData));
                    data.BaseStream.Position = propEnd;
                }
            }

            public bool IsComboBoxProp(int propTypeID)
            {
			    // PROPTYPE_COMBOBOX, PROPTYPE_COMBOBOXBTN, PROPTYPE_ICONCOMBOBOX
			    return propTypeID == 7 || propTypeID == 20 || propTypeID == 24;
            }

            public DarkEdifPropSet RuntimePropSet(DarkEdifProperty data)
            {
                DarkEdifPropSet rs = new DarkEdifPropSet(data.propData);
			    if (rs.setIndicator != "S")
				    throw new Exception("Not a prop set!");
			    return rs;
            }

            public int GetPropertyIndex(object chkIDOrName)
            {
                if (propVer > 1)
                {
                    int jsonIdx = -1;
                    DarkEdifProperty p = null;
                    if (chkIDOrName is int || chkIDOrName is uint || chkIDOrName is float || chkIDOrName is double)
                    {
                        foreach (DarkEdifProperty prop in props)
                        {
                            if (prop.index == Convert.ToUInt32(chkIDOrName))
                            {
                                p = prop;
                                break;
                            }
                        }
                    }
                    else
                    {
                        foreach (DarkEdifProperty prop2 in props)
                        {
                            if (prop2.propName == chkIDOrName.ToString())
                            {
                                p = prop2;
                                break;
                            }
                        }
                    }
                    if (p == null)
                    {
                        throw new Exception("Invalid property name \"" + chkIDOrName + "\"");
                    }
                    jsonIdx = (int)p.propJSONIndex;

                    // Look up prop index from JSON index - DarkEdif::Properties::PropIdxFromJSONIdx
				    DarkEdifProperty data = props[0];
                    int i = 0;
                    while (data.propJSONIndex != jsonIdx)
                    {
                        if (i >= numProps)
                        {
						    throw new Exception("Couldn't find property of JSON ID " + jsonIdx + ", hit property " + i + " of " + numProps + " stored.\n");
					    }

                        long oldPos = data.propData.BaseStream.Position;
                        data.propData.BaseStream.Position = 0;
                        char propDataIdentifier = (char)data.propData.ReadByte();
                        data.propData.BaseStream.Position = oldPos;
                        if (IsComboBoxProp((int)data.propTypeID) && propDataIdentifier == 'S')
                        {
                            DarkEdifPropSet rs = RuntimePropSet(data);
                            DarkEdifProperty rsContainer = data;
                            if (jsonIdx > rs.lastSetJSONPropIndex)
                            {
                                while (data.propJSONIndex != rs.lastSetJSONPropIndex)
                                {
                                    data = props[i++];
                                }
                                rs = null;
                                rsContainer = null;
                            }
                            // It's within this set's range
                            else if (jsonIdx >= rs.firstSetJSONPropIndex && jsonIdx <= rs.lastSetJSONPropIndex)
                            {
                                if (rs.getIndexSelected() > 0)
                                {
                                    int j = 0;
                                    while (true)
                                    {
                                        data = props[++i];

                                        // Skip until end of this entry, then move to next prop
									    if (data.propJSONIndex == rs.lastSetJSONPropIndex)
                                        {
                                            if (++j == rs.getIndexSelected())
                                            {
                                                data = props[++i];
											    break;
                                            }
                                        }
                                    }
								    continue;
                                }
                                else
                                {
								    data = props[++i];
								    continue;
                                }
                            }
						    // else it's not in this set: continue to standard loop
						    else
                            {
							    rs = null;
                                rsContainer = null;
						    }
                        }
					
					    data = props[++i];
                    }
				    return (int)data.index;
                }
                if (chkIDOrName is int || chkIDOrName is uint || chkIDOrName is float || chkIDOrName is double)
                {
				    if (numProps <= Convert.ToUInt32(chkIDOrName))
                    {
					    throw new Exception("Invalid property ID " + chkIDOrName + ", max ID is " + (numProps - 1) + ".");
				    }
				    return (int)chkIDOrName;
			    }
                DarkEdifProperty p2 = null;
                foreach (DarkEdifProperty prop3 in props)
                {
                    if (prop3.propName == chkIDOrName.ToString())
                    {
                        p2 = prop3;
                        break;
                    }
                }
                if (p2 == null)
                {
                    throw new Exception("Invalid property name \"" + chkIDOrName + "\"");
                }
			    return (int)p2.index;
            }

            public bool IsPropChecked(object chkIDOrName)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
			    if (idx == -1)
                {
				    return false;
			    }
			    return (chkboxes[(int)Math.Floor(idx / 8.0)] & (1 << idx % 8)) != 0;
            }

            static readonly List<int> textPropIDs = new List<int>
            {
                5,  // PROPTYPE_EDIT_STRING
                22, // PROPTYPE_EDIT_MULTILINE
                16, // PROPTYPE_FILENAME
                19, // PROPTYPE_PICTUREFILENAME
                26, // PROPTYPE_DIRECTORYNAME
            };

            public string GetPropertyStr(object chkIDOrName)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
			    if (idx == -1)
                {
				    return null;
			    }
			    DarkEdifProperty prop = props[idx];
                if (textPropIDs.IndexOf((int)prop.propTypeID) != -1 || IsComboBoxProp((int)prop.propTypeID))
                {
                    // Prop ver 2 added repeating prop sets
				    if (propVer == 2 && IsComboBoxProp((int)prop.propTypeID))
                    {
                        long oldPos = prop.propData.BaseStream.Position;
                        prop.propData.BaseStream.Position = 0;
                        char setIndicator = (char)prop.propData.ReadByte();
                        prop.propData.BaseStream.Position = oldPos;
                        if (setIndicator == 'L')
                        {
                            prop.propData.BaseStream.Position = 1;
                            long bytesAvailable = prop.propData.BaseStream.Length - prop.propData.BaseStream.Position;
                            return Encoding.UTF8.GetString(prop.propData.ReadBytes((int)bytesAvailable));
                        }
                        else if (setIndicator == 'S')
                        {
                            throw new Exception("Property " + prop.propName + " is not textual.");
                        }
                        throw new Exception("Property " + prop.propName + " is not a valid list property.");
                    }
                    prop.propData.BaseStream.Position = 0;
                    long bytesAvailable2 = prop.propData.BaseStream.Length - prop.propData.BaseStream.Position;
                    string t = Encoding.UTF8.GetString(prop.propData.ReadBytes((int)bytesAvailable2));
                    if (prop.propTypeID == 22) //PROPTYPE_EDIT_MULTILINE
                    {
					    t = t.Replace("\r", ""); // CRLF to LF
				    }
				    return t;
                }
			    throw new Exception("Property " + prop.propName + " is not textual.");
            }

            static readonly List<int> numPropIDsInteger = new List<int>
            {
				6, // PROPTYPE_EDIT_NUMBER
				9, // PROPTYPE_COLOR
				11, // PROPTYPE_SLIDEREDIT
				12, // PROPTYPE_SPINEDIT
				13 // PROPTYPE_DIRCTRL
            };

            static readonly List<int> numPropIDsFloat = new List<int>
            {
				21, // PROPTYPE_EDIT_FLOAT
				27 // PROPTYPE_SPINEDITFLOAT
            };

            public double GetPropertyNum(object chkIDOrName)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
				    return 0.0;
			    }
                DarkEdifProperty prop = props[idx];
			    if (numPropIDsInteger.IndexOf((int)prop.propTypeID) != -1)
                {
                    prop.propData.BaseStream.Position = 0;
				    return prop.propData.ReadUInt32();
			    }
			    if (numPropIDsFloat.IndexOf((int)prop.propTypeID) != -1)
                {
                    prop.propData.BaseStream.Position = 0;
				    return prop.propData.ReadSingle();
			    }
			    throw new Exception("Property " + prop.propName + " is not numeric.");
            }

            public int GetPropertyImageID(object chkIDOrName, int imgID)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
				    return -1;
			    }
                DarkEdifProperty prop = props[idx];
			    if (prop.propTypeID != 23) // PROPTYPE_IMAGELIST
                {
				    throw new Exception("Property " + prop.propName + " is not an image list.");
			    }
			
			    if (imgID < 0)
                {
				    throw new Exception("Image index " + imgID + " is invalid.");
			    }
                prop.propData.BaseStream.Position = 0;
			    if (imgID >= prop.propData.ReadUInt16())
                {
				    return -1;
			    }
			
                prop.propData.BaseStream.Position = 2 * (1 + idx);
			    return prop.propData.ReadUInt16();
            }

            public int GetPropertyNumImages(object chkIDOrName, int imgID)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
				    return -1;
			    }
                DarkEdifProperty prop = props[idx];
			    if (prop.propTypeID != 23) // PROPTYPE_IMAGELIST
                {
				    throw new Exception("Property " + prop.propName + " is not an image list.");
			    }
			
                prop.propData.BaseStream.Position = 0;
			    return prop.propData.ReadUInt16();
            }

            public Point? GetSizeProperty(object chkIDOrName)
            {
			    int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
				    return null;
			    }
                DarkEdifProperty prop = props[idx];
			    if (prop.propTypeID != 8) // PROPTYPE_SIZE
                {
				    throw new Exception("Property " + prop.propName + " is not an size property.");
			    }

                Point size = new Point();
                prop.propData.BaseStream.Position = 0;
                size.X = prop.propData.ReadInt32();
                prop.propData.BaseStream.Position = 4;
                size.Y = prop.propData.ReadInt32();
                return size;
            }

            // TODO: PropSetIterator and LoopPropSet
        }

        public static class DarkEdif
        {
            private static Dictionary<string, object> data = new Dictionary<string, object>();
            private static int sdkVersion = 20;

            public static object getGlobalData(string key)
            {
                key = key.ToLower();
                if (data.ContainsKey(key))
                {
                    return data[key];
                }
                return null;
            }

            public static void setGlobalData(string key, object value)
            {
                key = key.ToLower();
                data[key] = value;
            }
        
            // Could not implement getCurrentFusionEventNumber: evgLine is not in Flash

            public static void checkSupportsSDKVersion(int sdkVer)
            {
                if (sdkVer < 16 || sdkVer > 20)
                {
                    throw new Exception("Flash DarkEdif SDK does not support SDK version " + sdkVersion);
                }
            }

            public static DarkEdifProperties getProperties(CRunExtension ext, CFile edPtrFile, int extVersion)
            {
                return new DarkEdifProperties(ext, edPtrFile, extVersion);
            }
        }
    }
}