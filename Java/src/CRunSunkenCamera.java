//----------------------------------------------------------------------------------
//
// CRUNTEMPLATE: Template extension object by Yunivers
//
//----------------------------------------------------------------------------------

import Actions.CActExtension;
import Application.CRunApp;
import Conditions.CCndExtension;
import Expressions.CValue;
import Extensions.CRunExtension;
import RunLoop.CCreateObjectInfo;
import RunLoop.CRun;
import Services.CBinaryFile;

import java.awt.*;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

@SuppressWarnings("unused")
public class CRunSunkenCamera extends CRunExtension
{
    // DarkEdif SDK exts should have these four variables defined.
    public final int ExtensionVersion = 4; // To match C++ version
    public final int SDKVersion = 20; // To match C++ version
    public final boolean DebugMode = false;
    public final String ExtensionName = "SunkenCamera";

    // You can put variables exclusive to this extension here.
    public double Divisor;
    public double Margin;
    public double Factor;
    public boolean DontScroll;
    public boolean CenterDisplay;
    public boolean Easing;
    public boolean HoriScrolling;
    public boolean VertScrolling;
    public boolean Peytonphile;
    public boolean PeytonphileToEdges;
    public boolean HoriFlipped;
    public boolean VertFlipped;

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

    @Override
    public boolean createRunObject(CBinaryFile file, CCreateObjectInfo cob, int version) {
	    // This function is called when the object is created.
        //
        // As Java object are just created when needed, there is no EDITDATA structure.
        // Instead, I send to the function a CBinaryFile object, pointing directly to
        // the data of the object (the EDITDATA structure, on disc).
        //
        // The CBinaryFile object allows you to read the data.
        // It automatically converts PC-like ordering (big or little endian
        // I cant remember) into Java ordering.
        // It contains functions to read bytes, shorts, int, colors and strings.
        // Read the documentation about this class at the end of the document.
        //
        // "Version" contains the version value contained in the extHeader
        // structure of the EDITDATA.
        //
        // So all you have to do, is read the data from the CBinaryFile object,
        // and initialise your object accordingly.

        // DarkEdif properties are accessible as on other platforms: IsPropChecked(), GetPropertyStr(), GetPropertyNum()
        DarkEdif.DarkEdifProperties props = DarkEdif.getProperties(this, file, version);

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

        return false;
    }

    @Override
    public void destroyRunObject(boolean bFast) {
        // Called when the object is destroyed.
        // Due to garbage collection, this routine should not have much to do,
        // as all the data reserved in the object will be freed at the next GC.
        // bFast is true if the object is destroyed at end of frame.
        // It is false if the object is destroyed in the middle of the application.
    }

    @Override
    public int handleRunObject() {
        // Same as the C++ function.
        // Perform all the tasks needed for your object in this function.
        // As the C function, this function returns value indicating what to do :
        //
        // - REFLAG_ONESHOT : handleRunObject will not be called anymore
        // - REFLAG_DISPLAY : displayRunObject is called at next refresh
        // - Return 0 and the handleRunObject method will be called at the next loop

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

    @Override
    public void displayRunObject(Graphics2D graphics2D) {
        // Called to display the object.
        // The parameter given is Graphics2D object of the frame,
        // where you must draw your object at the correct co-ordinates.
        // For controls, this function is also called when you resize the window,
        // this time with a Graphics2D equal to null.
        //
        // When you resize the window, I explore the list of object,
        // and call the ones with a OEFLAG_WINDOWPROC defined (this should include all
        // the controls).
        //
        // You should reposition the control at the correct position (using
        // getXOffet and getYOffset).
    }

    // Conditions
    // --------------------------------------------------
    final int CND_CheckDisallowScrolling = 0;
    final int CND_CheckEasing = 1;
    final int CND_CheckHoriScrolling = 2;
    final int CND_CheckVertScrolling = 3;
    final int CND_CheckPeytonphile = 4;
    final int CND_CheckCenterDisplay = 5;
    final int CND_CheckHoriFlipped = 6;
    final int CND_CheckVertFlipped = 7;
    final int CND_CheckPeytonphileToEdges = 8;

    @Override
    public int getNumberOfConditions() {
        // This function should return the number of conditions contained in the object
        return 9;
    }

    @Override
    public boolean condition(int num, CCndExtension cnd) {
        // The main entry for the evaluation of the conditions.
        // - num : number of the condition (equivalent to the CND_ definitions in ext.h)
        // - cnd : a pointer to a CCndExtension object that contains useful callback
        // functions to get the parameters.
        // - This function should return true or false, depending on the condition.
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

    public boolean cndCheckDisallowScrolling()
    {
        return DontScroll;
    }

    public boolean cndCheckCenterDisplay()
    {
        return CenterDisplay;
    }

    public boolean cndCheckEasing()
    {
        return Easing;
    }

    public boolean cndCheckHoriScrolling()
    {
        return HoriScrolling;
    }

    public boolean cndCheckVertScrolling()
    {
        return VertScrolling;
    }

    public boolean cndCheckPeytonphile()
    {
        return Peytonphile;
    }

    public boolean cndCheckPeytonphileToEdges()
    {
        return PeytonphileToEdges;
    }

    public boolean cndCheckHoriFlipped()
    {
        return HoriFlipped;
    }

    public boolean cndCheckVertFlipped()
    {
        return VertFlipped;
    }

    // Actions
    // -------------------------------------------------
    final int ACT_SetDivisor = 0;
    final int ACT_SetMargin = 1;
    final int ACT_SetFactor = 2;
    final int ACT_SetDisallowScrolling = 3;
    final int ACT_SetEasing = 4;
    final int ACT_SetHoriScrolling = 5;
    final int ACT_SetVertScrolling = 6;
    final int ACT_SetPeytonphile = 7;
    final int ACT_SetCenterDisplay = 8;
    final int ACT_SetCameraPosX = 9;
    final int ACT_SetCameraPosY = 10;
    final int ACT_SetCameraTargetX = 11;
    final int ACT_SetCameraTargetY = 12;
    final int ACT_FlipHorizontally = 13;
    final int ACT_FlipVertically = 14;
    final int ACT_SetPeytonphileToEdges = 15;

    @Override
    public void action(int num, CActExtension act) {
        // The main entry for the actions.
        // - num : number of the action, as defined in ext.h
        // - act : pointer to a CActExtension object that contains callback
        // functions to get the parameters.
        double param = num != ACT_FlipHorizontally && num != ACT_FlipVertically ? act.getParamExpDouble(rh, 0) : 0;
        switch (num)
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

    // Expressions
    // --------------------------------------------
    final int EXP_GetDivisor = 0;
    final int EXP_GetMargin = 1;
    final int EXP_GetFactor = 2;
    final int EXP_GetXScroll = 3;
    final int EXP_GetYScroll = 4;
    final int EXP_GetXScrollTarget = 5;
    final int EXP_GetYScrollTarget = 6;
    final int EXP_GetXSpeed = 7;
    final int EXP_GetYSpeed = 8;

    @Override
    public CValue expression(int num) {
        // The main entry for expressions.
        // - num : number of the expression
        //
        // To get the expression parameters, you have to call the getExpParam() method
        // defined in the "ho" variable, for each of the parameters.
        // This function returns a CValue which contains the parameter.
        // You then do a getInt(), getDouble() or getString() with the
        // CValue object to grab the actual value.
        //
        // This method should return a CValue object containing the value to return.
        // The content of the CValue can be a integer, a double or a String.
        // There is no need to set the HOF_STRING flags if your return a string :
        // the CValue object contains the type of the returned value.
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

    @SuppressWarnings("ReassignedVariable")
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

    @SuppressWarnings("ReassignedVariable")
    public int GetFrameLeft()
    {
        int r = rh.rhWindowX;
        if ((rh.rh3Scrolling & CRun.RH3SCROLLING_SCROLL) != 0)
            r = rh.rh3DisplayX;
        if (r < 0)
            r = 0;
        return r;
    }

    @SuppressWarnings("ReassignedVariable")
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

    @SuppressWarnings("ReassignedVariable")
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
        return Math.max(Math.min(value, max), min);
    }

    @SuppressWarnings("ReassignedVariable")
    public int GetMouseX()
    {
        if (HoriFlipped)
        {
            int rh2MouseX = (int)ResX - rh.rhApp.mouseX;
            if ((rh.rhGameFlags & CRun.GAMEFLAGS_REALGAME) != 0 && (rh.rhApp.gaFlags & CRunApp.GA_STRETCH) != 0)
            {
                double xScale = (double)rh.rhFrame.leEditWinWidth / (double)rh.rhApp.sxComponent;
                rh2MouseX = (int)((double)rh2MouseX * xScale);
            }
            return rh2MouseX + rh.rhWindowX;
        }
        return this.rh.getXMouse();
    }

    @SuppressWarnings("ReassignedVariable")
    public int GetMouseY()
    {
        if (VertFlipped)
        {
            int rh2MouseY = (int)ResX - rh.rhApp.mouseY;
            if ((rh.rhGameFlags & CRun.GAMEFLAGS_REALGAME) != 0 && (rh.rhApp.gaFlags & CRunApp.GA_STRETCH) != 0)
            {
                double yScale = (double)rh.rhFrame.leEditWinHeight / (double)rh.rhApp.syComponent;
                rh2MouseY = (int)((double)rh2MouseY * yScale);
            }
            return rh2MouseY + rh.rhWindowY;
        }
        return this.rh.getXMouse();
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

    private static class DarkEdif {
        public static class DarkEdifProperty {
            public long index;
            public long propTypeID;
            public long propJSONIndex;
            public String propName;
            public ByteBuffer propData;

            public DarkEdifProperty(long index, long propTypeID, long propJSONIndex, String propName, ByteBuffer propData)
            {
                this.index = index;
                this.propTypeID = propTypeID;
                this.propJSONIndex = propJSONIndex;
                this.propName = propName;
                this.propData = propData;
            }
        }

        public static class DarkEdifPropSet {
            public String setIndicator;
            public long numRepeats;
            public long lastSetJSONPropIndex;
            public long firstSetJSONPropIndex;
            public long setNameJSONPropIndex;
            public String setName;

            private final ByteBuffer rsDV;

            public DarkEdifPropSet(ByteBuffer rsDV)
            {
                // Always 'S', compared with 'L' for non-set list.
                if (rsDV.remaining() > 0)
                {
                    setIndicator = String.valueOf((char)(rsDV.get() & 0xFF));
                }
                // Number of repeats of this set, 1 is minimum and means one of this set
                numRepeats = rsDV.getShort() & 0xFFFF;
                // Property that ends this set's data, as a JSON index, inclusive
                lastSetJSONPropIndex = rsDV.getShort() & 0xFFFF;
                // First property that begins this set's data, as a JSON index
                firstSetJSONPropIndex = rsDV.getShort() & 0xFFFF;
                // Name property JSON index that will appear in list when switching set entry
                setNameJSONPropIndex = rsDV.getShort() & 0xFFFF;
                // Set name, as specified in JSON. Don't confuse with user-specified set name.
                long bytesAvailable = rsDV.limit() - rsDV.position();
                byte[] bytes = new byte[(int)bytesAvailable];
                rsDV.get(bytes);
                try {
                    setName = new String(bytes, "UTF-8");
                } catch (UnsupportedEncodingException ex) {
                    Logger.getLogger(DarkEdifPropSet.class.getName()).log(Level.SEVERE, null, ex);
                }

                this.rsDV = rsDV;
            }

            public long getIndexSelected()
            {
                rsDV.position(1 + (2 * 4));
                return rsDV.getShort() & 0xFFFF;
            }

            public void setIndexSelected(long i)
            {
                rsDV.position(1 + (2 * 4));
                rsDV.putShort((short)i);
            }
        }

        public static class DarkEdifPropSetIterator implements Iterator<Integer>
        {
            public final int nameListJSONIdx;
            public final int numSkippedSetsBefore;
            public final DarkEdifProperties props;
            public final DarkEdifProperty runSetEntry;
            public final DarkEdifPropSet runPropSet;
            public boolean firstIt;

            public DarkEdifPropSetIterator(int nameListJSONIdx, int numSkippedSetsBefore, DarkEdifProperty runSetEntry, DarkEdifProperties props)
            {
                this.nameListJSONIdx = nameListJSONIdx;
                this.numSkippedSetsBefore = numSkippedSetsBefore;
                this.props = props;
                this.runSetEntry = runSetEntry;

                this.runPropSet = new DarkEdifPropSet(runSetEntry.propData);
                this.runPropSet.setIndexSelected(0);
                this.firstIt = true;
            }

            @Override
            public boolean hasNext()
            {
                return runPropSet.getIndexSelected() < runPropSet.numRepeats;
            }

            @Override
            public Integer next()
            {
                if (firstIt)
                    firstIt = false;
                else
                    runPropSet.setIndexSelected(runPropSet.getIndexSelected() + 1);
                return (int)runPropSet.getIndexSelected();
            }

            @Override
            public void remove()
            {
                throw new UnsupportedOperationException("You cannot remove a PropSet from this iterator!");
            }
        }

        public static class DarkEdifProperties {
            private final long numProps;
            private final int propVer;
            private final byte[] chkboxes;
            private final List<DarkEdifProperty> props;

            private static int GetFileLength(CBinaryFile file)
            {
                try {
                    java.lang.reflect.Field field = CBinaryFile.class.getDeclaredField("data");
                    field.setAccessible(true);
                    byte[] data = (byte[]) field.get(file);
                    return data.length;
                } catch (IllegalArgumentException ex) {
                    Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                } catch (IllegalAccessException ex) {
                    Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                } catch (NoSuchFieldException ex) {
                    Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                } catch (SecurityException ex) {
                    Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                }
                return 0;
            }

            public DarkEdifProperties(CRunExtension ext, CBinaryFile edPtrFile, int extVersion)
            {
                // DarkEdif SDK stores offset of DarkEdif props away from start of EDITDATA inside private data.
                // eHeader is 20 bytes, so this should be 20+ bytes.
                if (ext.ho.privateData < 20)
                {
                    throw new RuntimeException("Not smart properties - eHeader missing?");
                }
                // DarkEdif SDK header read:
                // header uint32, hash uint32, hashtypes uint32, numprops uint16, pad uint16, sizeBytes uint32 (includes whole EDITDATA)
                // if prop set v2, then uint64 editor checkbox ptr
                // then checkbox list, one bit per checkbox, including non-checkbox properties
                // so skip numProps / 8 bytes
                // then moving to Data list:
                // size uint32 (includes whole Data), propType uint16, propNameSize uint8, propname u8 (lowercased), then data bytes

                int oldPos = edPtrFile.getFilePointer();
                byte[] bytes = new byte[GetFileLength(edPtrFile)];
                edPtrFile.seek(0);
                edPtrFile.read(bytes);
                edPtrFile.seek(oldPos);
                byte[] verBuff = new byte[4];
                edPtrFile.skipBytes(ext.ho.privateData - 20); // sub size of eHeader; edPtrFile won't start with eHeader
                edPtrFile.read(verBuff);
                StringBuilder verStr = new StringBuilder();
                for (int i = verBuff.length - 1; i >= 0; i--)
                {
                    verStr.append((char) verBuff[i]);
                }
                if (verStr.toString().equals("DAR2"))
                {
                    propVer = 2;
                }
                else if (verStr.toString().equals("DAR1"))
                {
                    propVer = 1;
                }
                else
                {
                    throw new RuntimeException("Version string " + verStr + " unknown. Did you restore the file offset?");
                }
                // Pull out hash, hashTypes, numProps, pad, sizeBytes, visibleEditorProps
                byte[] headerBytes = new byte[4 + 4 + 2 + 2 + 4 + (propVer > 1 ? 8 : 0)];
                edPtrFile.read(headerBytes);
                ByteBuffer header = ByteBuffer.wrap(headerBytes);
                header.order(ByteOrder.LITTLE_ENDIAN);
                header.position(4 + 4); // Skip past hash and hashTypes
                numProps = header.getShort() & 0xFFFF;
                header.position(4 + 4 + 4); // skip past numProps and pad
                long sizeBytes = header.getInt() & 0xFFFFFFFFL;

                byte[] editDataBytes = new byte[
                    (int)sizeBytes -
                    // skip eHeader
                    ext.ho.privateData -
                    // cursor offset
                    4 -
                    // Skip DarkEdif header
                    header.limit()
                ];
                edPtrFile.read(editDataBytes);
                ByteBuffer editData = ByteBuffer.wrap(editDataBytes);
                editData.order(ByteOrder.LITTLE_ENDIAN);
                editData.position(0);
                chkboxes = new byte[(int)Math.ceil(numProps / 8.0)];
                editData.get(chkboxes, 0, chkboxes.length);

                props = new ArrayList<DarkEdifProperty>();
                editData.position(chkboxes.length);
                ByteBuffer data = editData.slice();
                data.order(ByteOrder.LITTLE_ENDIAN);

                // Dont need TextDecoder

                long propSize;
                long propEnd;
                data.position(0); // pt
                for (long i = 0; i < numProps; ++i)
                {
                    propSize = data.getInt() & 0xFFFFFFFFL;
                    propEnd = data.position() - 4 + propSize;
                    long propTypeID = data.getShort() & 0xFFFF;
                    // propJSONIndex does not exist in Data in DarkEdif smart props ver 1, so JSON index is same as prop index
                    long propJSONIndex = i;
                    if (propVer == 2)
                    {
                        propJSONIndex = data.getShort() & 0xFFFF;
                    }
                    int propNameLength = data.get() & 0xFF;
                    byte[] propNameBytes = new byte[propNameLength];
                    data.get(propNameBytes);
                    String propName = "";
                    try {
                        propName = new String(propNameBytes, "UTF-8");
                    } catch (UnsupportedEncodingException ex) {
                        Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                    }

                    byte[] propDataBytes = new byte[(int)(propEnd - data.position())];
                    data.get(propDataBytes);
                    ByteBuffer propData = ByteBuffer.wrap(propDataBytes);
                    propData.order(ByteOrder.LITTLE_ENDIAN);

                    props.add(new DarkEdifProperty(i, propTypeID, propJSONIndex, propName, propData));
                    data.position((int)propEnd);
                }
            }

            public boolean IsComboBoxProp(int propTypeID)
            {
                // PROPTYPE_COMBOBOX, PROPTYPE_COMBOBOXBTN, PROPTYPE_ICONCOMBOBOX
                return propTypeID == 7 || propTypeID == 20 || propTypeID == 24;
            }

            public DarkEdifPropSet RuntimePropSet(DarkEdifProperty data)
            {
                DarkEdifPropSet rs = new DarkEdifPropSet(data.propData);
                if (!rs.setIndicator.equals("S"))
                    throw new RuntimeException("Not a prop set!");
                return rs;
            }

            public int GetPropertyIndex(Object chkIDOrName)
            {
                if (propVer > 1)
                {
                    int jsonIdx;
                    DarkEdifProperty p = null;
                    if (chkIDOrName instanceof Integer || chkIDOrName instanceof Long || chkIDOrName instanceof Float || chkIDOrName instanceof Double)
                    {
                        long id = ((Number)chkIDOrName).longValue();
                        for (DarkEdifProperty darkEdifProperty : props)
                        {
                            if (darkEdifProperty.index == id)
                            {
                                p = darkEdifProperty;
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (DarkEdifProperty prop : props)
                        {
                            if (prop.propName.equals(chkIDOrName.toString()))
                            {
                                p = prop;
                                break;
                            }
                        }
                    }
                    if (p == null)
                    {
                        throw new RuntimeException("Invalid property name \"" + chkIDOrName + "\"");
                    }
                    jsonIdx = (int)p.propJSONIndex;

                    // Look up prop index from JSON index - DarkEdif::Properties::PropIdxFromJSONIdx
                    DarkEdifProperty data = props.get(0);
                    int i = 0;
                    while (data.propJSONIndex != jsonIdx)
                    {
                        if (i >= numProps)
                        {
                            throw new RuntimeException("Couldn't find property of JSON ID " + jsonIdx + ", hit property " + i + " of " + numProps + " stored.\n");
                        }

                        char propDataIdentifier = 0;
                        if (data.propData.remaining() > 0)
                        {
                            propDataIdentifier = (char)data.propData.get(0);
                        }
                        if (IsComboBoxProp((int)data.propTypeID) && propDataIdentifier == 'S')
                        {
                            DarkEdifPropSet rs = RuntimePropSet(data);
                            if (jsonIdx > rs.lastSetJSONPropIndex)
                            {
                                while (data.propJSONIndex != rs.lastSetJSONPropIndex)
                                {
                                    data = props.get(i++);
                                }
                            }
                            // It's within this set's range
                            else if (jsonIdx >= rs.firstSetJSONPropIndex && jsonIdx <= rs.lastSetJSONPropIndex)
                            {
                                if (rs.getIndexSelected() > 0)
                                {
                                    int j = 0;
                                    while (true)
                                    {
                                        data = props.get(++i);

                                        // Skip until end of this entry, then move to next prop
                                        if (data.propJSONIndex == rs.lastSetJSONPropIndex)
                                        {
                                            if (++j == rs.getIndexSelected())
                                            {
                                                data = props.get(++i);
                                                break;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    data = props.get(++i);
                                }
                                continue;
                            }
                            // else it's not in this set: continue to standard loop
                        }

                        data = props.get(++i);
                    }
                    return (int)data.index;
                }
                if (chkIDOrName instanceof Integer || chkIDOrName instanceof Long || chkIDOrName instanceof Float || chkIDOrName instanceof Double)
                {
                    long id = ((Number)chkIDOrName).longValue();
                    if (numProps <= id)
                    {
                        throw new RuntimeException("Invalid property ID " + chkIDOrName + ", max ID is " + (numProps - 1) + ".");
                    }
                    return (int)id;
                }
                DarkEdifProperty p2 = null;
                for (DarkEdifProperty prop : props)
                {
                    if (prop.propName.equals(chkIDOrName.toString()))
                    {
                        p2 = prop;
                        break;
                    }
                }
                if (p2 == null)
                {
                    throw new RuntimeException("Invalid property name \"" + chkIDOrName + "\"");
                }
                return (int)p2.index;
            }

            public boolean IsPropChecked(Object chkIDOrName)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return false;
                }
                return (chkboxes[(int)Math.floor(idx / 8.0)] & (1 << idx % 8)) != 0;
            }

            static final List<Integer> textPropIDs = Arrays.asList(
                5,  // PROPTYPE_EDIT_STRING
                22, // PROPTYPE_EDIT_MULTILINE
                16, // PROPTYPE_FILENAME
                19, // PROPTYPE_PICTUREFILENAME
                26 // PROPTYPE_DIRECTORYNAME
            );

            public String GetPropertyStr(Object chkIDOrName)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return null;
                }
                DarkEdifProperty prop = props.get(idx);
                if (textPropIDs.contains((int)prop.propTypeID) || IsComboBoxProp((int)prop.propTypeID))
                {
                    // Prop ver 2 added repeating prop sets
                    if (propVer == 2 && IsComboBoxProp((int)prop.propTypeID))
                    {
                        char setIndicator = 0;
                        if (prop.propData.remaining() > 0)
                        {
                            setIndicator = (char)prop.propData.get(0);
                        }
                        if (setIndicator == 'L')
                        {
                            prop.propData.position(1);
                            byte[] propStrBytes = new byte[prop.propData.remaining()];
                            prop.propData.get(propStrBytes);
                            try {
                                return new String(propStrBytes, "UTF-8");
                            } catch (UnsupportedEncodingException ex) {
                                Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                            }
                            return "";
                        }
                        else if (setIndicator == 'S')
                        {
                            throw new RuntimeException("Property " + prop.propName + " is not textual.");
                        }
                        throw new RuntimeException("Property " + prop.propName + " is not a valid list property.");
                    }
                    prop.propData.position(0);
                    byte[] tBytes = new byte[prop.propData.remaining()];
                    prop.propData.get(tBytes);
                    String t = "";
                    try {
                        t = new String(tBytes, "UTF-8");
                    } catch (UnsupportedEncodingException ex) {
                        Logger.getLogger(DarkEdifProperties.class.getName()).log(Level.SEVERE, null, ex);
                    }
                    if (prop.propTypeID == 22) //PROPTYPE_EDIT_MULTILINE
                    {
                        t = t.replace("\r", ""); // CRLF to LF
                    }
                    return t;
                }
                throw new RuntimeException("Property " + prop.propName + " is not textual.");
            }

            static final List<Integer> numPropIDsInteger = Arrays.asList(
                6, // PROPTYPE_EDIT_NUMBER
                9, // PROPTYPE_COLOR
                11, // PROPTYPE_SLIDEREDIT
                12, // PROPTYPE_SPINEDIT
                13 // PROPTYPE_DIRCTRL
            );

            static final List<Integer> numPropIDsFloat = Arrays.asList(
                21, // PROPTYPE_EDIT_FLOAT
                27 // PROPTYPE_SPINEDITFLOAT
            );

            public double GetPropertyNum(Object chkIDOrName)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return 0.0;
                }
                DarkEdifProperty prop = props.get(idx);
                if (numPropIDsInteger.contains((int)prop.propTypeID))
                {
                    return prop.propData.getInt(0) & 0xFFFFFFFFL;
                }
                if (numPropIDsFloat.contains((int)prop.propTypeID))
                {
                    return prop.propData.getFloat(0);
                }
                throw new RuntimeException("Property " + prop.propName + " is not numeric.");
            }

            public int GetPropertyImageID(Object chkIDOrName, int imgID)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return -1;
                }
                DarkEdifProperty prop = props.get(idx);
                if (prop.propTypeID != 23) // PROPTYPE_IMAGELIST
                {
                    throw new RuntimeException("Property " + prop.propName + " is not an image list.");
                }

                if (imgID < 0)
                {
                    throw new RuntimeException("Image index " + imgID + " is invalid.");
                }
                if (imgID >= (prop.propData.getShort(0) & 0xFFFF))
                {
                    return -1;
                }

                return prop.propData.getShort(2 * (1 + idx)) & 0xFFFF;
            }

            public int GetPropertyNumImages(Object chkIDOrName, int imgID)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return -1;
                }
                DarkEdifProperty prop = props.get(idx);
                if (prop.propTypeID != 23) // PROPTYPE_IMAGELIST
                {
                    throw new RuntimeException("Property " + prop.propName + " is not an image list.");
                }

                return prop.propData.getShort(0) & 0xFFFF;
            }

            public Point GetSizeProperty(Object chkIDOrName)
            {
                int idx = GetPropertyIndex(chkIDOrName);
                if (idx == -1)
                {
                    return null;
                }
                DarkEdifProperty prop = props.get(idx);
                if (prop.propTypeID != 8) // PROPTYPE_SIZE
                {
                    throw new RuntimeException("Property " + prop.propName + " is not an size property.");
                }

                Point size = new Point();
                size.x = prop.propData.getInt(0);
                size.y = prop.propData.getInt(4);
                return size;
            }

            @SuppressWarnings("UnusedReturnValue")
            public DarkEdifPropSetIterator LoopPropSet(String setName)
            {
                return LoopPropSet(setName, 0);
            }

            public DarkEdifPropSetIterator LoopPropSet(String setName, int numSkips)
            {
                DarkEdifProperty d;
                for (int i = 0, j = 0; i < numProps; ++i)
                {
                    d = props.get(i);
                    if (IsComboBoxProp((int)d.propTypeID) && (char)d.propData.get(0) == 'S')
                    {
                        if (new DarkEdifPropSet(d.propData).setName.equals(setName) && ++j > numSkips)
                            return new DarkEdifPropSetIterator(i, j - 1, d, this);
                    }
                }
                throw new RuntimeException("No set found with name " + setName + ".");
            }
        }

        private static final Map<String, Object> data = new HashMap<String, Object>();
        private static final int sdkVersion = 20;

        private DarkEdif()
        {
            throw new RuntimeException("DarkEdif is a static class, you cannot initialize it!");
        }

        public static Object getGlobalData(String key)
        {
            key = key.toLowerCase();
            if (data.containsKey(key))
            {
                return data.get(key);
            }
            return null;
        }

        public static void setGlobalData(String key, Object value)
        {
            key = key.toLowerCase();
            data.put(key, value);
        }

        public int getCurrentFusionEventNumber(CRunExtension ext)
        {
            return ext.rh.rhEvtProg.rhEventGroup.evgIdentifier;
        }

        public static void checkSupportsSDKVersion(int sdkVer)
        {
            if (sdkVer < 16 || sdkVer > 20)
            {
                throw new RuntimeException("Flash DarkEdif SDK does not support SDK version " + sdkVersion);
            }
        }

        public static void consoleLog(CRunSunkenCamera ext, String str)
        {
            if (ext == null || ext.DebugMode)
            {
                final String extName = ext == null ? "Unknown DarkEdif ext" : ext.ExtensionName;
                Logger.getLogger(extName).log(Level.INFO, str);
            }
        }

        public static DarkEdifProperties getProperties(CRunExtension ext, CBinaryFile edPtrFile, int extVersion)
        {
            return new DarkEdifProperties(ext, edPtrFile, extVersion);
        }
    }
}
