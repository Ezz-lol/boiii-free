#pragma once

namespace steam
{
	class utils
	{
	public:
		~utils() = default;

		virtual unsigned int GetSecondsSinceAppActive();
		virtual unsigned int GetSecondsSinceComputerActive();
		virtual int GetConnectedUniverse();
		virtual unsigned int GetServerRealTime();
		virtual const char* GetIPCountry();
		virtual bool GetImageSize(int iImage, unsigned int* pnWidth, unsigned int* pnHeight);
		virtual bool GetImageRGBA(int iImage, unsigned char* pubDest, int nDestBufferSize);
		virtual bool GetCSERIPPort(unsigned int* unIP, unsigned short* usPort);
		virtual unsigned char GetCurrentBatteryPower();
		virtual unsigned int GetAppID();
		virtual void SetOverlayNotificationPosition(int eNotificationPosition);
		virtual bool IsAPICallCompleted(unsigned long long hSteamAPICall, bool* pbFailed);
		virtual int GetAPICallFailureReason(unsigned long long hSteamAPICall);
		virtual bool GetAPICallResult(unsigned long long hSteamAPICall, void* pCallback, int cubCallback,
		                              int iCallbackExpected, bool* pbFailed);
		virtual void RunFrame();
		virtual unsigned int GetIPCCallCount();
		virtual void SetWarningMessageHook(void (*pFunction)(int hpipe, const char* message));
		virtual bool IsOverlayEnabled();
		virtual bool BOverlayNeedsPresent();
		virtual unsigned long long CheckFileSignature(const char* szFileName);

		virtual bool ShowGamepadTextInput(int eInputMode, int eInputLineMode, const char* szText,
		                                  unsigned int uMaxLength);
		virtual unsigned int GetEnteredGamepadTextLength();
		virtual bool GetEnteredGamepadTextInput(char* pchValue, unsigned int cchValueMax);

		virtual const char* GetSteamUILanguage();
		virtual bool IsSteamRunningInVR();
		virtual void SetOverlayNotificationInset(int nHorizontalInset, int nVerticalInset);
		virtual bool IsSteamInBigPictureMode();
		virtual void StartVRDashboard();
	};
}
