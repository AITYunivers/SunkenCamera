// DarkEdifPreBuildTool: This file was generated for a Extension.cpp/h that was modified at 2024-05-09T03:27:15.9097296Z.
// This file is used by the DarkEdifPreBuildTool as an input template for the calltable generator.
// Do not modify or delete this file.
#if DARKEDIF_ACE_CALL_TABLE_INDEX==0
			case 0:
				return ext->CheckDisallowScrolling();
			case 5:
				return ext->CheckCenterDisplay();
			case 1:
				return ext->CheckEasing();
			case 2:
				return ext->CheckHoriScrolling();
			case 3:
				return ext->CheckVertScrolling();
			case 4:
				return ext->CheckPeytonphile();

#elif DARKEDIF_ACE_CALL_TABLE_INDEX==1
			case 0:
				return ext->SetDivisor(*(float *)&Parameters[0]), 0;
			case 1:
				return ext->SetMargin(*(float *)&Parameters[0]), 0;
			case 2:
				return ext->SetFactor(*(float *)&Parameters[0]), 0;
			case 3:
				return ext->SetDisallowScrolling(*(int *)&Parameters[0]), 0;
			case 8:
				return ext->SetCenterDisplay(*(int *)&Parameters[0]), 0;
			case 4:
				return ext->SetEasing(*(int *)&Parameters[0]), 0;
			case 5:
				return ext->SetHoriScrolling(*(int *)&Parameters[0]), 0;
			case 6:
				return ext->SetVertScrolling(*(int *)&Parameters[0]), 0;
			case 7:
				return ext->SetPeytonphile(*(int *)&Parameters[0]), 0;

#elif DARKEDIF_ACE_CALL_TABLE_INDEX==2
		case 0:
			*((float *)&Result) = ext->GetDivisor();
			break;
		case 1:
			*((float *)&Result) = ext->GetMargin();
			break;
		case 2:
			*((float *)&Result) = ext->GetFactor();
			break;
		case 3:
			*((float *)&Result) = ext->GetXScroll();
			break;
		case 4:
			*((float *)&Result) = ext->GetYScroll();
			break;
		case 5:
			*((float *)&Result) = ext->GetXScrollTarget();
			break;
		case 6:
			*((float *)&Result) = ext->GetYScrollTarget();
			break;
		case 7:
			*((float *)&Result) = ext->GetXSpeed();
			break;
		case 8:
			*((float *)&Result) = ext->GetYSpeed();
			break;

#else
	#error Incorrect DarkEdif ACE Call Table index
#endif
