#pragma once
#include <d3dx9tex.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <cstdint>
#include <utility>
#include "Includes.h"
#include "Settings.h"
#include <thread>
#pragma comment(lib, "d3dx9.lib")

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };
HWND MyWnd = NULL;
HWND GameWnd = NULL;
MSG Message = { NULL };
RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;
DWORD ScreenCenterX;
DWORD ScreenCenterY;
DWORD ScreenCenterZ;
static HWND Window = NULL;
static LPDIRECT3DDEVICE9 D3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;
const MARGINS Margin = { -1 };
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SetWindowToTarget();

namespace SDK {

	DWORD ProcessID;
	DWORD64 ModuleBase;
}

float Width = GetSystemMetrics(SM_CXSCREEN), Height = GetSystemMetrics(SM_CYSCREEN), Depth;

static HWND get_process_wnd(uint32_t pid) {
	std::pair<HWND, uint32_t> params = { 0, pid };

	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
		auto pParams = (std::pair<HWND, uint32_t>*)(lParam);
		uint32_t processId = 0;

		if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processId)) && processId == pParams->second) {
			SetLastError((uint32_t)-1);
			pParams->first = hwnd;
			return FALSE;
		}

		return TRUE;

		}, (LPARAM)&params);

	if (!bResult && GetLastError() == -1 && params.first)
		return params.first;

	return NULL;
}

HRESULT DirectXInit(HWND hWnd)
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.ConfigFlags |= 1 << 10;

	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFontConfig icons_config;

	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;

	ImFontConfig CustomFont;
	CustomFont.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.0f);

	bool show_demo_window = true, loader_window = false;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);


	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	return S_OK;
}

void SetupWindow()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEXA wcex = {
	sizeof(WNDCLASSEXA),
	0,
	DefWindowProcA,
	0,
	0,
	nullptr,
	LoadIcon(nullptr, IDI_APPLICATION),
	LoadCursor(nullptr, IDC_ARROW),
	nullptr,
	nullptr,
	(("goanigs")),
	LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyWnd = CreateWindowExA(NULL, ("goanigs"), ("goanigs"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	//SetWindowDisplayAffinity(MyWnd, 1);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);

	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);
}

bool menuopen = false;
int X, Y, alpha, hsv;

float color_red = 1.;
float color_green = 0;
float color_blue = 0;
float color_random = 0.0;
float color_speed = -10.0;

void ColorChange()
{
	static float Color[3];
	static DWORD Tickcount = 0;
	static DWORD Tickcheck = 0;
	ImGui::ColorConvertRGBtoHSV(color_red, color_green, color_blue, Color[0], Color[1], Color[2]);
	if (GetTickCount() - Tickcount >= 1)
	{
		if (Tickcheck != Tickcount)
		{
			Color[0] += 0.001f * color_speed;
			Tickcheck = Tickcount;
		}
		Tickcount = GetTickCount();
	}
	if (Color[0] < 0.0f) Color[0] += 1.0f;
	ImGui::ColorConvertHSVtoRGB(Color[0], Color[1], Color[2], color_red, color_green, color_blue);
}

ImGuiWindow& BeginScene() {
	ImGui_ImplDX9_NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##scene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}

VOID EndScene(ImGuiWindow& window) {
	window.DrawList->PushClipRectFullScreen();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::Render();
}


static const char* hitboxes[]
{
	"head",
	"neck",
	"body",
};

typedef struct _FNlEntity {
	uintptr_t Actor;
	uint64_t Mesh;
	int ID;
}FNlEntity;
std::vector<FNlEntity> PlayerPawns;

namespace autod
{
	uintptr_t GWorld = 0;
	uintptr_t GameInstance = 0;
	uintptr_t PersistentLevel = 0;
	uintptr_t LocalPlayers = 0;
	uintptr_t LocalPlayer = 0;
	uintptr_t PlayerController = 0;
	uintptr_t PlayerCameraManager = 0;
	uintptr_t LocalPawn = 0;
	UINT ActorCount = 0;
	uintptr_t Actors = 0;
	uintptr_t Mesh = 0;
}
//
//static std::string ReadGetNameFromFName(int key) {
//	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
//	uint16_t NameOffset = (uint16_t)key;
//
//	uint64_t NamePoolChunk = read<uint64_t>(SDK::ModuleBase + 0xDF86AC0 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
//	uint16_t nameEntry = read<uint16_t>(NamePoolChunk);
//
//	int nameLength = nameEntry >> 6;
//	char buff[1024];
//	if ((uint32_t)nameLength)
//	{
//		for (int x = 0; x < nameLength; ++x)
//		{
//			buff[x] = read<char>(NamePoolChunk + 4 + x);
//		}
//		char* v2 = buff; // rdi
//		int v4 = nameLength; // ebx
//		int v5; // edx
//		__int64 result; // rax
//		int v7; // ecx
//		unsigned int v8; // eax
//
//		v5 = 0;
//		result = 28;
//		if (v4)
//		{
//			do
//			{
//				++v2;
//				v7 = v5++ | 0xB000;
//				v8 = v7 + result;
//				v7 = v8 ^ ~*(v2 - 1);
//				result = v8 >> 2;
//				*(v2 - 1) = v7;
//			} while (v5 < v4);
//		}
//	}
//	else {
//		return "";
//	}
//}
//
//static std::string GetNameFromFName(int key)
//{
//	uint64_t NamePoolChunk = read<uint64_t>(SDK::ModuleBase + 0xDF86AC0 + (8 * (uint32_t)((int)(key) >> 16)) + 16) + (unsigned int)(4 * (uint16_t)key);
//	if (read<uint16_t>(NamePoolChunk) < 64)
//	{
//		auto a1 = read<DWORD>(NamePoolChunk + 4);
//		return ReadGetNameFromFName(a1);
//	}
//	else
//	{
//		return ReadGetNameFromFName(key);
//	}
//}

std::string AllAct;

void GetEntitiesChache()
{
	while (true)
	{
		std::vector<FNlEntity> tmpList;

		autod::GWorld = read<uintptr_t>(SDK::ModuleBase + 0xe9ea3f8);
		autod::GameInstance = read<uintptr_t>(autod::GWorld + 0x1B8);
		autod::PersistentLevel = read<uintptr_t>(autod::GWorld + 0x30);
		autod::LocalPlayers = read<uintptr_t>(autod::GameInstance + 0x38);
		autod::LocalPlayer = read<uintptr_t>(autod::LocalPlayers);
		autod::PlayerController = read<uintptr_t>(autod::LocalPlayer + 0x30);
		autod::PlayerCameraManager = read<uintptr_t>(autod::PlayerController + 0x340);
		autod::LocalPawn = read<uintptr_t>(autod::PlayerController + 0x330);
		autod::ActorCount = read<UINT>(autod::PersistentLevel + 0xA0);
		autod::Actors = read<uintptr_t>(autod::PersistentLevel + 0x98);

		for (int i = 0; i < autod::ActorCount; ++i) {
			uintptr_t CurrentActor = read<uintptr_t>(autod::Actors + i * 0x8);
			auto CurrentActorID = read<uintptr_t>(CurrentActor + 0x18);

			if (read<float>(CurrentActor + 0x4388) != 10) continue;
			_FNlEntity Actor{ };
			Actor.ID = CurrentActorID;
			Actor.Actor = CurrentActor;
			Actor.Mesh = read<uintptr_t>(CurrentActor + 0x310);
			tmpList.push_back(Actor);
		}
		PlayerPawns = tmpList;
		Sleep(250);
	}
}

//uintptr_t gPendingWeapon = 0;
//BOOL InstantReloadInProgress = FALSE;
//bool InstantReload = FALSE; // define whereu want in menu / settings
//static bool InstantReloadOnce = false;
//void InstantReloadThread()
//{
//	while (true)
//	{
//		static uintptr_t gCWeapon = 0;
//		if (gPendingWeapon) gCWeapon = gPendingWeapon; // current weapon
//		static bool RunOnce = false;
//
//		static bool wasInstantReloadEnabled = false;
//		static uintptr_t MatchingWeaponStats[1500];
//		static int MatchingReloadTypes[1500];
//		static float LastValidReloadTime = 0;
//		static uintptr_t lastWeapon = 0;
//		static int MatchingWeaponCount = 0;
//
//		if (InstantReload && gCWeapon)
//		{
//			wasInstantReloadEnabled = true;
//			InstantReloadInProgress = true;
//
//			for (int i = 0; i < MatchingWeaponCount; i++)
//			{
//				write<int>(MatchingWeaponStats[i] + 0xF0, -1);
//			}
//
//			bool isReloading = read<bool>(gCWeapon + 0x329);
//			static bool wasReloading = false;
//			if (isReloading && !wasReloading)
//			{
//				wasReloading = true;
//				float LastFireTime = read<float>(lastWeapon + 0xAB0);
//				write<float>(lastWeapon + 0xAB4, LastFireTime - 2.7f);
//			}
//			else if (!isReloading && wasReloading)
//			{
//				wasReloading = false;
//			}
//
//			bool reloading = read<bool>(gCWeapon + 0x329);
//			float ReloadTime = read<float>(gCWeapon + 0xB30);
//
//			if (ReloadTime)
//			{
//				if (lastWeapon != gCWeapon)
//				{
//					MatchingWeaponCount = 0;
//					uintptr_t Data = read<uintptr_t>(gCWeapon + 0x3F0);
//					uintptr_t StatHandle = read<uintptr_t>(Data + 0x9D8);
//					uintptr_t StatsTable = read<uintptr_t>(StatHandle + 0x30);
//
//					for (UINT64 i = 0x8; i < 0x3000; i += 0x8)
//					{
//						uintptr_t pointer = read<uintptr_t>(StatsTable + i);
//						if (read<float>(pointer + 0xB38) == ReloadTime)
//						{
//							lastWeapon = gCWeapon;
//							MatchingWeaponStats[MatchingWeaponCount] = pointer;
//							MatchingWeaponCount++;
//						}
//					}
//				}
//
//
//				if (MatchingWeaponCount)
//				{
//					for (int i = 0; i < MatchingWeaponCount; i++)
//					{
//						write<int>(MatchingWeaponStats[i] + 0xF0, -1);
//					}
//
//					// instant reload
//					if (true)
//					{
//
//						int AmmoCount = read<int>(gCWeapon + 0xB64);
//
//						if (gCWeapon == gPendingWeapon && AmmoCount == 0)
//						{
//							auto time = std::chrono::high_resolution_clock::now();
//							while (!read<bool>(lastWeapon + 0x329) && lastWeapon == gPendingWeapon && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() < 500);
//							time = std::chrono::high_resolution_clock::now();
//							while (read<bool>(lastWeapon + 0x329) && lastWeapon == gPendingWeapon && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time).count() < 500);
//
//							if (lastWeapon == gPendingWeapon)
//							{
//								float LastFireTime = read<float>(lastWeapon + 0xAB0);
//								write<float>(lastWeapon + 0xAB4, LastFireTime - 2.7f);
//							}
//						}
//					}
//				}
//			}
//		}
//		InstantReloadInProgress = false;
//		Sleep(30);
//
//	}
//}

#define M_PI 3.14159265358979323846264338327950288419716939937510

static class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline double Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vector3 operator/(double v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	inline double Distance(Vector3 v)
	{
		return sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0));
	}

	inline double Length() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(double flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }

	Vector3 ToVector()
	{
		float angle, sr, sp, sy, cr, cp, cy;

		angle = this->y * (M_PI * 2 / 360);
		sy = sin(angle);
		cy = cos(angle);

		angle = this->x * (M_PI * 2 / 360);
		sp = sin(angle);
		cp = cos(angle);

		angle = this->z * (M_PI * 2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		return Vector3(cp * cy, cp * sy, -sp);
	}
};

struct FPlane : Vector3
{
	double W = 0;


	Vector3 ToVector3()
	{
		Vector3 value;
		value.x = this->x;
		value.y = this->y;
		value.z = this->y;

		return value;
	}
};

struct FMatrix
{
	FPlane XPlane;
	FPlane YPlane;
	FPlane ZPlane;
	FPlane WPlane;
};

struct FTransform
{
	FPlane  rot;
	Vector3 translation;
	char    pad[8];
	Vector3 scale;

	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		double x2 = rot.x + rot.x;
		double y2 = rot.y + rot.y;
		double z2 = rot.z + rot.z;

		double xx2 = rot.x * x2;
		double yy2 = rot.y * y2;
		double zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		double yz2 = rot.y * z2;
		double wx2 = rot.W * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		double xy2 = rot.x * y2;
		double wz2 = rot.W * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		double xz2 = rot.x * z2;
		double wy2 = rot.W * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

static D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

static D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
	float radPitch = (rot.x * double(M_PI) / 180.f);
	float radYaw = (rot.y * double(M_PI) / 180.f);
	float radRoll = (rot.z * double(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

namespace camera
{
	float m_FovAngle;
	Vector3 m_CameraRotation, m_CameraLocation;
}


void Draw2DBox(int X, int Y, int W, int H, int thickness, ImColor color) {
	float lineW = (W / 1.5);
	float lineH = (H / 1.5);

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(color), thickness);
}

VOID GetCurrentCamera() {
	if (autod::LocalPawn)
	{
		const auto SetupCameraRotationAndFov = [](uintptr_t LocalPlayer, uintptr_t RootComponent, Vector3& CameraRotation, float& FOVAngle)
		{
			auto CameraRotChain_tmp0 = read<uint64_t>((uint64_t)LocalPlayer + 0xd0 + 0x10);

			camera::m_CameraRotation.x = asin(read<double>(CameraRotChain_tmp0 + 0x7D0)) * (180.0 / M_PI);
			camera::m_CameraRotation.y = read<double>((uintptr_t)RootComponent + 0x148);


			FOVAngle = 80.0f / (read<double>(CameraRotChain_tmp0 + 0x680) / 1.19f);
		};
		SetupCameraRotationAndFov(autod::LocalPlayer, read<uintptr_t>(autod::LocalPawn + 0x190), camera::m_CameraRotation, camera::m_FovAngle);
	}
	else {
		camera::m_CameraRotation = read<Vector3>((uintptr_t)autod::PlayerCameraManager + 0x28d0 + 0x10 + 0x18);
		camera::m_CameraRotation.z = 0;

		camera::m_FovAngle = read<float>((uintptr_t)autod::PlayerCameraManager + 0x28d0 + 0x10 + 0x30); //0x2180
	}



	const auto SetupCameraLocation = [](uintptr_t LocalPlayer, Vector3& CameraLocation)
	{
		CameraLocation = read<Vector3>(autod::GWorld + 0x100);
	};
	SetupCameraLocation(autod::LocalPlayer, camera::m_CameraLocation);
}
struct Camera
{
	float FieldOfView;
	Vector3 Rotation;
	Vector3 Location;
};

Camera FGC_Camera;
uintptr_t _GetViewPoint;

Camera GetCamera(__int64 a1, uintptr_t PlayerCameraManager)
{
	if (autod::LocalPawn)
	{
		__int64 v1;
		__int64 v6;
		__int64 v7;
		__int64 v8;
		
		v1 = read<__int64>(autod::LocalPlayer + 0xd0);
		__int64 v9 = read<__int64>(v1 + 0x8); // 0x10

		FGC_Camera.FieldOfView = 80.0f / (read<double>(v9 + 0x7F0) / 1.19f); // 0x680

		FGC_Camera.Rotation.x = read<double>(v9 + 0x9C0);
		FGC_Camera.Rotation.y = read<double>(a1 + 0x148);

		uint64_t FGC_Pointerloc = read<uint64_t>(autod::GWorld + 0x110);
		FGC_Camera.Location = read<Vector3>(FGC_Pointerloc);

		return FGC_Camera;
	}
	else
	{
		FGC_Camera.FieldOfView = read<float>((uintptr_t)PlayerCameraManager + 0x2a80 + 0x10 + 0x30);

		FGC_Camera.Rotation = read<Vector3>((uintptr_t)PlayerCameraManager + 0x2a80 + 0x10 + 0x18);
		FGC_Camera.Rotation.z = 0;

		FGC_Camera.Location = read<Vector3>((uintptr_t)PlayerCameraManager + 0x2a80 + 0x10);

	}

	return FGC_Camera;
}
namespace CustomFunc
{






	Vector3 ProjectWorldToScreen(Vector3 WorldLocation, uintptr_t Rootcomp, uintptr_t PlayerCameraManager)
	{
		Camera vCamera = GetCamera(Rootcomp, PlayerCameraManager);
		vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);


		D3DMATRIX tempMatrix = Matrix(vCamera.Rotation);

		Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
		Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
		Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

		Vector3 vDelta = WorldLocation - vCamera.Location;
		Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

		if (vTransformed.z < 1.f)
			vTransformed.z = 1.f;

		return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
	}



	FTransform GetBoneIndex(DWORD_PTR mesh, int index)
	{
		DWORD_PTR bonearray;
		bonearray = read<DWORD_PTR>(mesh + 0x5C0);

		if (bonearray == NULL)
		{
			bonearray = read<DWORD_PTR>(mesh + 0x5C0 + 0x10);  //(mesh + 0x5e8) + 0x5a));
		}
		return read<FTransform>(bonearray + (index * 0x60));
	}

	Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
	{
		FTransform bone = GetBoneIndex(mesh, id);
		FTransform ComponentToWorld = read<FTransform>(mesh + 0x240);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}
}

#include <vector>
int N = 250;
int lineMaxDist = 2000;
ImColor lineCol = { 255, 255, 255, 60 };
float lineThickness = 1.0f;

void setupPoints(std::vector<std::pair<ImVec2, ImVec2>>& n) {
	ImVec2 screenSize(ImGui::GetIO().DisplaySize);
	for (auto& p : n)
		p.second = p.first = ImVec2(rand() % (int)screenSize.x, rand() % (int)screenSize.y);
}

float length(ImVec2 x) { return x.x * x.x + x.y * x.y; }

void autoParticles(ImDrawList* d, ImVec2 b)
{
	while (true)
	{
		static std::vector<std::pair<ImVec2, ImVec2>> points(N);
		static auto once = (setupPoints(points), true);
		float Dist;
		for (auto& p : points) {
			Dist = sqrt(length(p.first - p.second));
			if (Dist > 0) p.first += (p.second - p.first) / Dist;
			if (Dist < 4) p.second = ImVec2(rand() % (int)b.x, rand() % (int)b.y);
		}
		for (int i = 0; i < N; i++) {
			for (int j = i + 1; j < N; j++) {
				Dist = length(points[i].first - points[j].first);
				if (Dist < lineMaxDist) d->AddLine(points[i].first, points[j].first, lineCol, lineThickness);
			}
		}
		Sleep(250);
	}
}

static auto string_To_UTF8(const std::string& str) -> std::string
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

static auto WStringToUTF8(const wchar_t* lpwcszWString) -> std::string
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

static auto MBytesToWString(const char* lpcszString) -> std::wstring
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
static auto DrawNewText(int x, int y, ImColor color, const char* str) -> void
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color), utf_8_2.c_str());
}

static auto DrawLine(int x1, int y1, int x2, int y2, ImColor color, int thickness) -> void
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), thickness);
}

static auto DrawFilledRect(int x, int y, int w, int h, ImColor color)-> void
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0);
}

void DrawFilledRect2(float x, float y, float w, float h, ImColor color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, NULL, NULL);
}

void DrawPlayerBar(int x, int y, ImColor back_color, ImColor main_color, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());

	const ImVec2 text_dimension = ImGui::CalcTextSize(text.c_str());
	const float text_width = text_dimension.x + 5.f;
	const float mid_width = x - (text_width / 2.f);
	ImColor green_color = { 0.05f, 0.00f, 0.45f, 0.35f };

	DrawFilledRect(mid_width, y - 24.f, text_width + 8.f, text_dimension.y + 5.f, back_color);
	//DrawLine(mid_width + 28, y - 25.f, mid_width + 28, (y - 25.f) + text_dimension.y + 5.f, ImColor(0.f, 0.f, 0.f, 1.f), 1.5f);
	DrawLine(mid_width, y - 6, mid_width + text_width + 8.f, y - 6, ImColor(0.f, 0.f, 0.f, 1.f), 1.5f);
	DrawNewText(mid_width + 6.f, (y - 25.f) + 2.f, main_color, text.c_str());
	DrawNewText(mid_width + 6.f, (y - 25.f) + 2.f, main_color, text.c_str());

}
namespace DrawSMH
{
	void DrawLine(int x1, int y1, int x2, int y2, ImColor color, int thickness)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), thickness);
	}
}

void ResetAngles() {
	float ViewPitchMin = -89.9999f;
	float ViewPitchMax = 89.9999f;
	float ViewYawMin = 0.0000f;
	float ViewYawMax = 359.9999f;

	write<float>(autod::PlayerCameraManager + 0x3314, ViewPitchMin);
	write<float>(autod::PlayerCameraManager + 0x3318, ViewPitchMax);
	write<float>(autod::PlayerCameraManager + 0x331C, ViewYawMin);
	write<float>(autod::PlayerCameraManager + 0x3320, ViewYawMax);
}

void SetAngles(Vector3 CameraLocation, Vector3 TargetLocation) {
	Vector3 VectorPos = TargetLocation - CameraLocation;

	float distance = (double)(sqrtf(VectorPos.x * VectorPos.x + VectorPos.y * VectorPos.y + VectorPos.z * VectorPos.z));

	float x, y, z;
	x = -((acosf(VectorPos.z / distance) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510)) - 90.f);
	y = atan2f(VectorPos.y, VectorPos.x) * (float)(180.0f / 3.14159265358979323846264338327950288419716939937510);
	z = 0;

	write<float>(autod::PlayerCameraManager + 0x3314, x);
	write<float>(autod::PlayerCameraManager + 0x3318, x);
	write<float>(autod::PlayerCameraManager + 0x331C, y);
	write<float>(autod::PlayerCameraManager + 0x3320, y);
}
double RadianToDegree(double radian) {
	return radian * (180 / M_PI);
}

double DegreeToRadian(double degree) {
	return degree * (M_PI / 180);
}

Vector3 RadianToDegree(Vector3 radians) {
	Vector3 degrees;
	degrees.x = radians.x * (180 / M_PI);
	degrees.y = radians.y * (180 / M_PI);
	degrees.z = radians.z * (180 / M_PI);
	return degrees;
}

Vector3 DegreeToRadian(Vector3 degrees) {
	Vector3 radians;
	radians.x = degrees.x * (M_PI / 180);
	radians.y = degrees.y * (M_PI / 180);
	radians.z = degrees.z * (M_PI / 180);
	return radians;
}


void AngleVectors(const Vector3& angles, Vector3* forward)
{
	float	sp, sy, cp, cy;

	sy = sin(DegreeToRadian(angles.y));
	cy = cos(DegreeToRadian(angles.y));

	sp = sin(DegreeToRadian(angles.x));
	cp = cos(DegreeToRadian(angles.x));

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

void Make3DBox(bool foreground, bool StartIsCenter, float Offset, float Thickness, ImVec4 Clr, Vector3 Start, float Top)
{
	ImDrawList* Renderer;
	if (foreground)
		Renderer = ImGui::GetOverlayDrawList();
	else
		Renderer = ImGui::GetOverlayDrawList();

	if (StartIsCenter) {
		Vector3 BottomOne = Vector3(Start.x + Offset, Start.y + Offset, Start.z - Offset);
		Vector3 BottomOneW2S = CustomFunc::ProjectWorldToScreen(BottomOne, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
		
		Vector3 BottomTwo = Vector3(Start.x - Offset, Start.y - Offset, Start.z - Offset);
		Vector3 BottomTwoW2S = CustomFunc::ProjectWorldToScreen(BottomTwo, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 BottomThree = Vector3(Start.x + Offset, Start.y - Offset, Start.z - Offset);
		Vector3 BottomThreeW2S = CustomFunc::ProjectWorldToScreen(BottomThree, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 BottomFour = Vector3(Start.x - Offset, Start.y + Offset, Start.z - Offset);
		Vector3 BottomFourW2S = CustomFunc::ProjectWorldToScreen(BottomFour, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopOne = Vector3(Start.x + Offset, Start.y + Offset, Start.z + Offset);
		Vector3 TopOneW2S = CustomFunc::ProjectWorldToScreen(TopOne, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopTwo = Vector3(Start.x - Offset, Start.y - Offset, Start.z + Offset);
		Vector3 TopTwoW2S = CustomFunc::ProjectWorldToScreen(TopTwo, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopThree = Vector3(Start.x + Offset, Start.y - Offset, Start.z + Offset);
		Vector3 TopThreeW2S = CustomFunc::ProjectWorldToScreen(TopThree, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopFour = Vector3(Start.x - Offset, Start.y + Offset, Start.z + Offset);
		Vector3 TopFourW2S = CustomFunc::ProjectWorldToScreen(TopFour, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Renderer->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);

		Renderer->AddLine(ImVec2(TopOneW2S.x, TopOneW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopFourW2S.x, TopFourW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);

		Renderer->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
	}
	else {
		Vector3 BottomOne = Vector3(Start.x + Offset, Start.y + Offset, Start.z);
		Vector3 BottomOneW2S = CustomFunc::ProjectWorldToScreen(BottomOne, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 BottomTwo = Vector3(Start.x - Offset, Start.y - Offset, Start.z);
		Vector3 BottomTwoW2S = CustomFunc::ProjectWorldToScreen(BottomTwo, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 BottomThree = Vector3(Start.x + Offset, Start.y - Offset, Start.z);
		Vector3 BottomThreeW2S = CustomFunc::ProjectWorldToScreen(BottomThree, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 BottomFour = Vector3(Start.x - Offset, Start.y + Offset, Start.z);
		Vector3 BottomFourW2S = CustomFunc::ProjectWorldToScreen(BottomFour, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopOne = Vector3(Start.x + Offset, Start.y + Offset, Top);
		Vector3 TopOneW2S = CustomFunc::ProjectWorldToScreen(TopOne, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopTwo = Vector3(Start.x - Offset, Start.y - Offset, Top);
		Vector3 TopTwoW2S = CustomFunc::ProjectWorldToScreen(TopTwo, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopThree = Vector3(Start.x + Offset, Start.y - Offset, Top);
		Vector3 TopThreeW2S = CustomFunc::ProjectWorldToScreen(TopThree, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 TopFour = Vector3(Start.x - Offset, Start.y + Offset, Top);
		Vector3 TopFourW2S = CustomFunc::ProjectWorldToScreen(TopFour, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Renderer->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);

		Renderer->AddLine(ImVec2(TopOneW2S.x, TopOneW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(TopFourW2S.x, TopFourW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);

		Renderer->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
		Renderer->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), ImGui::GetColorU32({ Clr.x, Clr.y, Clr.z, Clr.w }), Thickness);
	}
}

void DrawFilledRectA(int x, int y, int w, int h, ImColor color) {
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
}
float CenterX = GetSystemMetrics(0) / 2 - 1;
float CenterY = GetSystemMetrics(1) / 2 - 1;
void Mouse(float x, float y, float smooth)
{
	float ScreenCenterX = CenterX;
	float ScreenCenterY = CenterY;
	int AimSpeed = Options::Aimbot::AimbotSmooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	mouse_event(MOUSEEVENTF_MOVE, static_cast<int>((float)TargetX), static_cast<int>((float)TargetY), NULL, NULL);
}

//if (Options::Visuals::Distance)
//{
	//char dist[64];
	//sprintf_s(dist, "%.fm | Enemy", FGC_Camera.Location.Distance(vRootBoneOut) / 100.f);

	////outline

	///*ImGui::GetOverlayDrawList()->AddText(ImVec2(w2shead.x - TextSize.x / 2, w2shead.y - 80 - TextSize.y / 2), ImColor(255, 255, 0, 255), dist);*/


	//DrawPlayerBar(w2shead.x, w2shead.y - 35, ImColor(0.85f, 0.85f, 0.2f, 0.45f), ImColor(0, 0, 0, 200), dist);
	//ImGui::GetOverlayDrawList()->AddTriangleFilled(ImVec2(w2shead.x, w2shead.y - 27), ImVec2(w2shead.x - 15, w2shead.y - 40), ImVec2(w2shead.x + 15, w2shead.y - 40), ImColor(0.85f, 0.85f, 0.2f, 0.45f));
//}


/*	static BYTE OldSpread;
	if (!OldSpread)
		OldSpread = read<BYTE>(SDK::ModuleBase + 0x21F2BBC);

	static bool bWroteNoSpread = false;

	if (Options::Misc::NoBloom && (GetAsyncKeyState(VK_RBUTTON) || GetAsyncKeyState(VK_LBUTTON)))
	{
		BYTE Verify = read<BYTE>(SDK::ModuleBase + 0x21F2BBC);
		if (Verify != 0xC3 + 0x10) {
			write<BYTE>(SDK::ModuleBase + 0x21F2BBC, 0xC3 + 0x10);
			bWroteNoSpread = true;
		}
	}
	else {
		if (bWroteNoSpread)
		{
			read<BYTE>(SDK::ModuleBase + 0x21F2BBC, OldSpread);
			bWroteNoSpread = false;
		}
	}*/

	/*	if (Options::Aimbot::MemAimbot)
		{
			Vector3 Mesh = read<Vector3>((uintptr_t)CurrentActorMesh + 0x190 + 0x140);

			Vector3 _Angle = Vector3(Mesh.x, Mesh.y, Mesh.z);

			Vector3 test2, headpos;

			headpos = vHeadBone;

			AngleVectors(_Angle, &test2);
			test2.x *= 160;
			test2.y *= 160;
			test2.z *= 160;

			Vector3 end = headpos + test2;
			Vector3 test1, test3;
			test1 = CustomFunc::ProjectWorldToScreen(headpos, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190));
			test3 = CustomFunc::ProjectWorldToScreen(end, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190));

			ImGui::GetOverlayDrawList()->AddLine(ImVec2(test1.x, test1.y), ImVec2(test3.x, test3.y), ImGui::GetColorU32({1.0f, 0.0f, 0.0f, 1.0f}), 2.0f);
			Make3DBox(false, true, 10.f, 2.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), end, 0.f);
		}
*/


double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

double GetDistance(double x1, double y1, double z1, double x2, double y2) {
	return sqrtf(powf((x2 - x1), 2) + powf((y2 - y1), 2));
}



		//if (Options::Misc::Spinbot)
		//{
		//	uintptr_t Mesh = read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x310);
		//	if (GetAsyncKeyState(VK_RBUTTON))
		//	{

		//		write<Vector3>(Mesh + 0x140, Vector3(0, -rand() % (int)360, 0));
		//	}
		//	else
		//	{
		//		write<Vector3>(Mesh + 0x140, Vector3(0, -90, 0));
		//	}
		//}

		//uintptr_t GetViewPoint = read<uintptr_t>(SDK::ModuleBase + 0x112D4B0);
		//if (!GetViewPoint) continue;

		//_GetViewPoint = (uintptr_t)GetViewPoint;

		//if (autod::LocalPawn)
		//{
		//	gPendingWeapon = read<uint64_t>(autod::LocalPawn + 0x858);
		//	static bool onceaa = 0;
		//	if (!onceaa)
		//	{
		//		std::thread(InstantReloadThread).detach();
		//		onceaa = 1;
		//	}
		//}

bool WasRecentlyRendered(uintptr_t Mesh) {
	float fLastSubmitTime = read<float>(Mesh + 0x330);
	float fLastRenderTimeOnScreen = read<float>(Mesh + 0x338);

	const float fVisionTick = 0.06f;
	return fLastRenderTimeOnScreen + fVisionTick >= fLastSubmitTime;
}

void render() {

	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = nullptr;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static bool once = 0;
	if (!once)
	{
		std::thread(GetEntitiesChache).detach();
		once = 1;
	}

	bool bValidEnemyInArea = false;
	float ClosestActorDistance = FLT_MAX;
	Vector3 ClosestActorMouseAimbotPosition = Vector3(0.0f, 0.0f, 0.0f);
	float distaim;
	float closestDistance = FLT_MAX;
	uintptr_t closestPawn = NULL;
	static bool IsPlayerVisible = 0;

	for (unsigned long i = 0; i < PlayerPawns.size(); ++i) {
		FNlEntity entity = PlayerPawns[i];
		uintptr_t CurrentActor = entity.Actor;

		uint64_t CurrentActorMesh = read<uint64_t>(CurrentActor + 0x310);

	/*	Vector3 GetPlayerViewPoint = read<Vector3>(autod::PlayerController + 0x7B0);

		if (!(double)GetPlayerViewPoint.x & !(double)GetPlayerViewPoint.y & !(double)GetPlayerViewPoint.z)
			std::cout << "No Valid GetPlayerViewPoint! -> " << GetPlayerViewPoint.x << std::hex << std::endl;*/



		Vector3 vHeadBone = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 68);
		Vector3 vRootBone = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 0);

		Vector3 vHeadBoneOut = CustomFunc::ProjectWorldToScreen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 10), read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
		Vector3 vRootBoneOut = CustomFunc::ProjectWorldToScreen(Vector3(vRootBone.x, vRootBone.y, vRootBone.z - 5), read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 w2shead = CustomFunc::ProjectWorldToScreen(vHeadBone, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		Vector3 RootPos = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 68);
		Vector3 RootPosOut = CustomFunc::ProjectWorldToScreen(RootPos, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

		float BoxHeight = abs(vHeadBoneOut.y - vRootBoneOut.y);
		float BoxWidth = BoxHeight * 0.75;

		if (autod::LocalPawn == CurrentActor) continue;

		auto VisibleCheck = WasRecentlyRendered(CurrentActorMesh);

		if (Options::Misc::FovChanger)
		{
			write<float>(autod::PlayerCameraManager + 0x29C + 0x4, Options::Misc::FovSize);
		}
		else
		{
			write<float>(autod::PlayerCameraManager + 0x29C + 0x4, 0);
		}

		

		if (Options::Misc::Spinbot)
		{
			uintptr_t Mesh = read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x310);
			write<Vector3>(Mesh + 0x140, Vector3(0, -rand() % (int)360, 0));
		}
		else
		{
			uintptr_t Mesh = read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x310);
			write<Vector3>(Mesh + 0x140, Vector3(0, -90, 0));
		}

		if (Options::Visuals::Box)
		{
			if (Options::Visuals::VisibleCheck)
			{
				if (VisibleCheck)
				{
					if (Options::Visuals::BoxFilled)
					{
						DrawFilledRectA(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, ImColor(40, 245, 110, 40));
					}
					Draw2DBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1, ImColor(40, 245, 110, 255));
				}
				else
				{

					if (Options::Visuals::BoxFilled)
					{
						DrawFilledRectA(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, ImColor(255, 0, 0, 40));
					}
					Draw2DBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1, ImColor(255, 0, 0, 255));
				}
			}
			else
			{
				if (Options::Visuals::BoxFilled)
				{
					DrawFilledRectA(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, ImColor(255, 255, 255, 25));
				}
				Draw2DBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1, ImColor(255, 255, 255, 255));
			}
		}

		if (Options::Visuals::Skeleton)
		{
			Vector3 vHeadBone = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 66);
			Vector3 vHip = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 2);
			Vector3 vNeck = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 65);
			Vector3 vUpperArmLeft = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 9);
			Vector3 vUpperArmRight = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 62);
			Vector3 vLeftHand = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 10);
			Vector3 vRightHand = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 38);
			Vector3 vLeftHand1 = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 11);
			Vector3 vRightHand1 = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 39);

			Vector3 vRightThigh = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 80);
			Vector3 vLeftThigh = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 73);

			Vector3 vRightCalf = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 74);
			Vector3 vLeftCalf = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 67);

			Vector3 vLeftFoot = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 68);
			Vector3 vRightFoot = CustomFunc::GetBoneWithRotation(CurrentActorMesh, 75);

			Vector3 vHeadBoneOut = CustomFunc::ProjectWorldToScreen(vHeadBone, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vHipOut = CustomFunc::ProjectWorldToScreen(vHip, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vNeckOut = CustomFunc::ProjectWorldToScreen(vNeck, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vUpperArmLeftOut = CustomFunc::ProjectWorldToScreen(vUpperArmLeft, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vUpperArmRightOut = CustomFunc::ProjectWorldToScreen(vUpperArmRight, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vLeftHandOut = CustomFunc::ProjectWorldToScreen(vLeftHand, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vRightHandOut = CustomFunc::ProjectWorldToScreen(vRightHand, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vLeftHandOut1 = CustomFunc::ProjectWorldToScreen(vLeftHand1, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vRightHandOut1 = CustomFunc::ProjectWorldToScreen(vRightHand1, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vRightThighOut = CustomFunc::ProjectWorldToScreen(vRightThigh, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vLeftThighOut = CustomFunc::ProjectWorldToScreen(vLeftThigh, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vRightCalfOut = CustomFunc::ProjectWorldToScreen(vRightCalf, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vLeftCalfOut = CustomFunc::ProjectWorldToScreen(vLeftCalf, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vLeftFootOut = CustomFunc::ProjectWorldToScreen(vLeftFoot, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			Vector3 vRightFootOut = CustomFunc::ProjectWorldToScreen(vRightFoot, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);

			ImColor ESPSkeleton = ImColor(255, 255, 255);

			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vHipOut.x, vHipOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vNeckOut.x, vNeckOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vLeftHandOut1.x, vLeftHandOut1.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vRightHandOut1.x, vRightHandOut1.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vHipOut.x, vHipOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vHipOut.x, vHipOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vLeftFootOut.x, vLeftFootOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImColor(255, 255, 255), 2.0f);
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(vRightFootOut.x, vRightFootOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImColor(255, 255, 255), 2.0f);
		}

		auto dx = w2shead.x - (Width / 2);
		auto dy = w2shead.y - (Height / 2);
		auto dz = w2shead.z - (Depth / 2);
		auto dist = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
		if (dist < Options::Aimbot::FovCircleValue && dist < closestDistance) {
			closestDistance = dist;
			closestPawn = entity.Actor;
			static bool isaimbotting;
			static bool targetlocked = false;
			uint64_t AimbotMesh = read<uint64_t>((uintptr_t)closestPawn + 0x310);
			Vector3 headA = CustomFunc::GetBoneWithRotation(AimbotMesh, 68);
			Vector3 headAAAAA = CustomFunc::GetBoneWithRotation(AimbotMesh, 2);
			Vector3 rootHeadOut = CustomFunc::ProjectWorldToScreen(headA, read<uintptr_t>((uintptr_t)autod::LocalPawn + 0x190), autod::PlayerCameraManager);
			if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0)
			{
				if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= Options::Aimbot::FovCircleValue * 1.5)) {
					if (Options::Aimbot::MemAimbot) {
						if (GetAsyncKeyState(VK_RBUTTON))
						{
							Mouse(rootHeadOut.x, rootHeadOut.y, 1);
							/*write<Vector3>(autod::PlayerCameraManager + 0x290 + 0x140, Vector3(rootHeadOut.x, rootHeadOut.y, 0));*/
						}
					}
				}
				if (Options::Visuals::AimLine)
				{
					if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
						if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= Options::Aimbot::FovCircleValue * 1.5)) {
							ImGui::GetOverlayDrawList()->AddLine({ (float)GetSystemMetrics(0) / 2, (float)GetSystemMetrics(1) / 2 }, { (float)rootHeadOut.x, (float)rootHeadOut.y }, ImColor(255, 255, 255), 1.f);
						}
					}
				}
			}
		}
	}


	if (GetAsyncKeyState(VK_INSERT) & 1) menuopen = !menuopen;

	if (Options::Aimbot::FovCircle)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(Width / 2, Height / 2), Options::Aimbot::FovCircleValue, ImColor(255, 255, 255, 255), 15, 1.0f);
	}

	if (menuopen)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowBorderSize = 0.00f;
		style.ChildBorderSize = 1.00f;
		style.PopupBorderSize = 0.00f;
		style.FrameBorderSize = 0.00f;
		style.TabBorderSize = 0.00f;
		style.WindowRounding = 0.00f;
		style.ChildRounding = 0.00f;
		style.FrameRounding = 0.00f;
		style.PopupRounding = 0.00f;
		style.ScrollbarRounding = 9.00f;
		style.AntiAliasedFill = true;
		style.AntiAliasedLines = true;

		ImGui::SetNextWindowSize({ 400, 288 });
		ImGui::Begin("saftaim", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
		{
			ImGui::Checkbox("Mouse Aimbot", &Options::Aimbot::MemAimbot);
			ImGui::Checkbox("Fov Circle", &Options::Aimbot::FovCircle);
			ImGui::Text("");
			ImGui::SliderFloat("Fov Circle Size", &Options::Aimbot::FovCircleValue, 0, 360, "%.1f", 1);
			ImGui::SliderFloat("Aimbot Smooth", &Options::Aimbot::AimbotSmooth, 0, 50, "%.1f", 1);
			ImGui::Text("");
			ImGui::Checkbox("Box", &Options::Visuals::Box);
			ImGui::Checkbox("Skeleton", &Options::Visuals::Skeleton);
			ImGui::Checkbox("Aimline", &Options::Visuals::AimLine);
			ImGui::Checkbox("Visible Check", &Options::Visuals::VisibleCheck);
			ImGui::Text("");
			ImGui::Checkbox("Player Fly", &Options::Misc::Spinbot);
			//ImGui::Checkbox("Fov Changer", &Options::Misc::FovChanger);
			//ImGui::SliderFloat("Camera Fov Size", &Options::Misc::FovSize, 0, 160, "%.1f", 1);
		}
		ImGui::End();

		ImGui::Render();
	}

	ImGui::EndFrame();
	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}
	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

//if (menuopen)
//{
//	ImGui::SetNextWindowSize(ImVec2(2700, 1200));
//	ImGui::SetNextWindowPos(ImVec2(0, 0));
//	ImGui::Begin("Background", &menuopen, ImVec2(2700, 1200), .45, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove);
//	{
//		auto draw = ImGui::GetWindowDrawList();
//		ImVec2 screenSize = ImGui::GetIO().DisplaySize;

//		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
//		ImGui::PopStyleColor();
//		ImGui::End();
//	}

//	ImGui::GetOverlayDrawList()->AddRectFilled(ImGui::GetIO().MousePos, ImVec2(ImGui::GetIO().MousePos.x + 5.f,
//		ImGui::GetIO().MousePos.y + 5.f), ImColor(255, 255, 255));
//}


//ImGuiStyle& style = ImGui::GetStyle();
//style.WindowBorderSize = 0.00f;
//style.ChildBorderSize = 1.00f;
//style.PopupBorderSize = 0.00f;
//style.FrameBorderSize = 0.00f;
//style.TabBorderSize = 0.00f;
//style.WindowRounding = 0.00f;
//style.ChildRounding = 0.00f;
//style.FrameRounding = 0.00f;
//style.PopupRounding = 0.00f;
//style.ScrollbarRounding = 9.00f;
//style.AntiAliasedFill = true;
//style.AntiAliasedLines = true;

//auto* colors = ImGui::GetStyle().Colors;
//colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
//colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
//colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
//colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
//colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
//colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 1.00f);
//colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
//colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
//colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
//colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
//colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
//colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
//colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
//colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
//colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
//colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
//colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
//colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
//colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.12f, 0.13f, 1.00f);
//colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
//colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
//colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
//colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
//colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
//colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
//colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
//colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
//colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
//colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
//colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
//colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
//colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
//colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
//colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
//colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
//colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
//colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
//colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
//colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
//colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
//colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
//colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

//ImGui::Columns(1);
			//if (ImGui::Button("Aimbot")) { menutab = 0; }
			//ImGui::SameLine();
			//if (ImGui::Button("Visual")) { menutab = 1; }
			//ImGui::SameLine();
			//if (ImGui::Button("Misc")) { menutab = 2; }

			/*if (menutab == 0)
			{
			}*/
			//if (menutab == 1)
			//{
			//	
			//	ImGui::Checkbox("Skeleton", &Options::Visuals::Skeleton);
			//	ImGui::Checkbox("Distance", &Options::Visuals::Distance);
			//	ImGui::Checkbox("Weapon", &Options::Visuals::Weapon);
			//	
			//}
			//if (menutab == 2)
			//{
			//	ImGui::Button("Press Button To Enable Misc (Own Risk!)");
			//	if (ImGui::IsItemClicked())
			//		Options::Misc::RiskOnce::IsOn = true;

			//	if (Options::Misc::RiskOnce::IsOn)
			//	{
			//		//ImGui::Checkbox("Spinbot (360)", &Options::Misc::Spinbot);
			//		//ImGui::Checkbox("No Weapon Bloom", &Options::Misc::NoBloom);
			//		//ImGui::Checkbox("No Weapon Reload", &InstantReload);
			//		//ImGui::Checkbox("No Weapon r", &Options::Misc::NoReload);
			//	}
			//}

void CleanuoD3D();

WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();
		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanuoD3D();
	DestroyWindow(MyWnd);

	return Message.wParam;
}
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		CleanuoD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void CleanuoD3D()
{
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}
	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}

void SetWindowToTarget()
{
	while (true)
	{
		GameWnd = get_process_wnd(SDK::ProcessID);
		if (GameWnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(GameWnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(GameWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}
			ScreenCenterX = Width / 2;
			ScreenCenterY = Height / 2;
			MoveWindow(MyWnd, GameRect.left, GameRect.top, Width, Height, true);
		}
	}
}