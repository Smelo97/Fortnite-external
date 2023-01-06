#pragma once
#include "Includes.h"

namespace Options
{
	namespace Aimbot
	{
		bool MemAimbot = false;
		bool Silent = false;
		bool FovCircle = false;
		float FovCircleValue = 120;
		float AimbotSmooth = 0;
		namespace Bones
		{
			int Bone = 1;
		}
	}
	namespace Visuals
	{
		bool Box = false;
		bool BoxFilled = false;
		bool Skeleton = false;
		bool VisibleCheck = false;
		bool Distance = false;
		bool Weapon = false;
		bool AimLine = false;

		namespace Colors
		{
			namespace Visible
			{
				ImColor BoxColor = {255,255,255,255};
				ImColor Distance = { 0,255,255,255 };
				ImColor AimLine = { 255,255,255,255 };
			}
			namespace InVisible
			{
				ImColor BoxColor = { 0,255,0,255 };
				ImColor Distance = { 0,255,255,255 };
				ImColor AimLine = { 0,255,0,255 };
			}
		}
	}

	namespace Misc
	{
		namespace RiskOnce
		{
			bool IsOn = false;
		}
		bool Spinbot = false;
		bool NoBloom = false;
		bool NoReload = false;
		bool FovChanger = false;
		float FovSize = 80.f;
	}
}