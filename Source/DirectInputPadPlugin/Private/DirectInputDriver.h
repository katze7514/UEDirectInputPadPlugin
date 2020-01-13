#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"

class FDirectInputJoystick;

//! DirectInputドライバークラス
class FDirectInputDriver
{
public:
	FDirectInputDriver() :pDriver_(nullptr) {}
	~FDirectInputDriver() { Fin(); }

	bool Init();
	void Fin();

	LPDIRECTINPUT8 driver()const { return pDriver_; }

private:
	LPDIRECTINPUT8 pDriver_;
};

/*! @brief 使用可能なジョイスティックを管理するクラス
 *
 *	使用可能なジョイスティックの情報を集めておき、必要に応じて
 *	その情報を返す
 */
class FDirectInputJoystickEnum
{
public:
	FDirectInputJoystickEnum(){  vecJoyStickInfo_.Reserve(8); }

	bool Init(FDirectInputDriver& adapter);

	//! 検出したXInputデバイスの数
	uint32 GetXInputDeviceNum()const{ return nXInputDeviceNum_; }

	//! 使用可能なジョイスティックの数
	uint32 EnabledJoystickNum()const{ return vecJoyStickInfo_.Num(); }

	//! @brief 使用可能なジョイスティックの情報を取得する
	/*! @param[in] nJoyNo 情報を取得するジョイスティック番号。0以上enable_joystick_num()以下 
	 *  @return ジョイスティック情報のoptioanl。ジョイスティックが存在していれば該当の値が入っている */
	const DIDEVICEINSTANCE* GetJoystickInfo(uint32_t nJoyNo)const;

private:
	//! DirectInput::EnumDeviceに渡すコールバック関数
	static BOOL CALLBACK OnEnumDevice(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

private:
	//! ジョイスティック情報配列
	TArray<DIDEVICEINSTANCE> vecJoyStickInfo_;

	uint32 nXInputDeviceNum_ = 0;
};


/*! @brief FDirectInputJoystickを作成管理するクラス
 *
 *  FDirectInputJoystick_enum/FDirectInputJoystickをまとめる。
 *  Joystickを取得する時はこれを使うと良い
 */
class FDirectInputJoystickFactory
{
public:
	typedef TMap<uint32, TSharedPtr<FDirectInputJoystick>> joy_map;

public:
	FDirectInputJoystickFactory();
	~FDirectInputJoystickFactory(){ Fin(); }

	bool Init(HWND hWnd, const TSharedPtr<FDirectInputDriver>& pDriver, bool bBackGround=false);
	void Fin();

	//! 検出したXInputデバイスの数
	uint32 GetXInputDeviceNum()const{ return joyEnum_.GetXInputDeviceNum(); }

	//! 使用可能なジョイスティックの数
	uint32 EnabledJoystickNum()const{ return joyEnum_.EnabledJoystickNum(); }

	//! @brief 指定の番号のFDirectInputJoystickを取得する
	/*! @return すでに存在したらそれを返す。存在しないjoystickだったら生成する。生成が失敗したらnullptrが返る */
	TSharedPtr<FDirectInputJoystick> GetJoystick(uint32 nNo);

private:
	TSharedPtr<FDirectInputDriver>	pAdapter_;
	HWND							hWnd_;
	bool							bBackGround_;

	FDirectInputJoystickEnum	joyEnum_;
	joy_map						mapJoy_;
};

#include "Windows/HideWindowsPlatformTypes.h"
