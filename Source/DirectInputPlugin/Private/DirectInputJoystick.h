#pragma once

#include "AllowWindowsPlatformTypes.h"

#define DIRECTINPUT_VERSION 0x0800 
#define _CRT_SECURE_NO_DEPRECATE

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <windows.h>

#pragma warning(push) 
#pragma warning(disable:6000 28251) 
#include <dinput.h> 
#pragma warning(pop) 

#include <dinputd.h>

#include "Core.h"

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
};


//! PVOと軸の入力方向を示す列挙子
enum EDirectInputArrow : uint8
{
	// 通常のボタンの次からの番号を振る
	POV_NONE = 32,	//!< POV何も押されていない
	POV_UP,			//!< POV↑
	POV_RIGHT,		//!< POV→
	POV_DOWN,		//!< POV↓
	POV_LEFT,		//!< POV←
	AXIS_NONE,		//!< XY軸何も押されていない
	AXIS_UP,		//!< 軸Y↑
	AXIS_RIGHT,		//!< 軸X→
	AXIS_DOWN,		//!< 軸Y↓
	AXIS_LEFT,		//!< 軸X←
	ARROW_END,
};


/*! @brief ジョイスティック一つを表すクラス
 *
 *	FDirectInputJoystick_enumから、取得した情報を元にデバイスを作成し
 *	実際に入力処理を行う
 *  軸・回転の範囲は、-1.0～1.0 を取る。あそびの範囲にある時は0.0が返る
 *
 *  AD変換をONにすると、XY軸デジタルがPOVとして、POVがXY軸デジタル入力としても扱われる
 *  XY軸がis_pov系メソッドで、POV系がis_axis系メソッドで取得できるようになる
 */
class FDirectInputJoystick
{
public:
	static const int32 MAX_AXIS_VALUE;

public:
	FDirectInputJoystick():pDevice_(nullptr),nCurIndex_(0),bAcquire_(false),
				  bADConv_(true),
				  nX_Threshold_(300),nY_Threshold_(300),nZ_Threshold_(300),
				  nXrot_Threshold_(300),nYrot_Threshold_(300),nZrot_Threshold_(300),
				  bGuard_(false){ ClearBuf(); }

	~FDirectInputJoystick(){ Fin(); }

	//! @brief 初期化
	/*! @param[in] joyins デバイスを作成するデバイスインスタンス
		@param[in] adaptar DirectInputアダプターインスタンス
	    @param[in] hWnd キーボード入力を受け付けるウインドウハンドル
		@param[in] bBackGraound trueだと非アクティブでもキー入力を受け付けるようになる */
	bool Init(const DIDEVICEINSTANCE& joyins, FDirectInputDriver& adapter, HWND hWnd, bool bBackGround=false);

	//! 終了処理
	void Fin();

	//! ジョイスティック入力を取得
	bool Input();

	//! AD変換が有効か
	bool IsAdConvFlag()const{ return bADConv_; }
	//! AD変換フラグ設定
	bool SetAdConvFlag(bool bAD){ bADConv_ = bAD; }

	//! アナログ軸のあそびの閾値設定。この値以下の入力は無効扱い
	void SetAxisThreshold(uint32 nX, uint32 nY, uint32 nZ){ nX_Threshold_=nX; nY_Threshold_=nY; nZ_Threshold_=nZ; }
	//! アナログ回転のあそびの閾値設定。この値以下の入力は無効扱い
	void SetRotThreshold(uint32 nX, uint32 nY, uint32 nZ){ nXrot_Threshold_=nX; nYrot_Threshold_=nY; nZrot_Threshold_=nZ; }

	//! x軸の値
	float	X()const;
	//! y軸の値
	float	Y()const;
	//! z軸の値
	float	Z()const;

	//! x軸の1つ前の値
	float	PrevX()const;
	//! y軸の1つ前の値
	float	PrevY()const;
	//! z軸の1つ前の値
	float	PrevZ()const;

	//! x回転の値
	float	RotX()const;
	//! y回転の値
	float	RotY()const;
	//! z回転の値
	float	RotZ()const;

	//! x回転の1つ前の値
	float	RotPrevX()const;
	//! y回転の1つ前の値
	float	RotPrevY()const;
	//! z回転の1つ前の値
	float	RotPrevZ()const;

	//! pov値
	int32 Pov()const;
	//! povの1つ前の値
	int32 PrevPov()const;

	//! ボタンが押されている。0～31はボタン。32～はPOVと軸
	bool IsPress(uint32 nBtn)const;
	//! ボタンが押された瞬間。0～31はボタン。32～はPOVと軸
	bool IsPush(uint32 nBtn)const;
	//! ボタンが離れた瞬間。0～31はボタン。32～はPOVと軸
	bool IsRelease(uint32 nBtn)const;

	// 入力ガード
	void SetGuard(bool bGuard);
	bool IsGuard()const{ return bGuard_; }

	//! バッファをクリアする
	void ClearBuf(){ nCurIndex_=0; ::ZeroMemory(joyBuf_, sizeof(joyBuf_)); }

	//! 現在のバッファをクリアする
	void ClearCurBuf(){ ::ZeroMemory(&joyBuf_[nCurIndex_], sizeof(DIJOYSTATE)); }

private:
	//! POVが押されている
	bool IsPovPress(enum EDirectInputArrow eArrow)const;
	//! POVが押された瞬間
	bool IsPovPush(enum EDirectInputArrow eArrow)const;
	//! POVが離れた瞬間
	bool IsPovRelease(enum EDirectInputArrow eArrow)const;


	//! POVが押されているかどうかのチェック
	/*! @param[in] eArrow チェックする方向
		@param[in] index チェックするバッファインデックス */
	bool IsPovPressInner(enum EDirectInputArrow eArrow, uint32 nIndex)const;

	//! XY軸がデジタル的に押されている
	bool IsAxisPress(enum EDirectInputArrow eArrow)const;
	//! XY軸がデジタル的に押された瞬間
	bool IsAxisPush(enum EDirectInputArrow eArrow)const;
	//! XY軸がデジタル的に離れた瞬間
	bool IsAxisRelease(enum EDirectInputArrow eArrow)const;

	//! XY軸がデジタル的に押されているかどうかのチェック
	/*! @param[in] eArrow チェックする方向
		@param[in] index チェックするバッファインデックス */
	bool IsAxisPressInner(enum EDirectInputArrow eArrow, uint32 nIndex)const;

	//! POV_*とAXIS_* を相互変換する
	enum EDirectInputArrow SwapPovAxis(enum EDirectInputArrow eArrow)const;

private:
	//! DirectInputDevice::EnumObjectsに渡すコールバック関数
	static BOOL CALLBACK OnEnumAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

private:
	//! デバイス
	LPDIRECTINPUTDEVICE8	pDevice_;

	//! ジョイスティック入力バッファ
	DIJOYSTATE	joyBuf_[2];
	//! ジョイスティックバッファ位置
	uint32		nCurIndex_;

	//! デバイスを獲得しているかどうか
	bool		bAcquire_;

	//! アナログX,YをPOV(十字キー)として見なすかどうか 
	bool		bADConv_;

	// joystickのアナログの遊びの値　-threshold～threshold の範囲になる
	// これ以下の値の時は入力なし扱い

	uint32		nX_Threshold_;
	uint32		nY_Threshold_;
	uint32		nZ_Threshold_;

	uint32		nXrot_Threshold_;
	uint32		nYrot_Threshold_;
	uint32		nZrot_Threshold_;

	//! @brief 入力ガードフラグ
	/*! trueになすると入力バッファがクリアされ、inputを呼んでも更新されない
	 *  つまり、入力されてない扱いになる */
	bool bGuard_;
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

	FDirectInputJoystickFactory(){}
	~FDirectInputJoystickFactory(){ Fin(); }

	bool Init(HWND hWnd, const TSharedPtr<FDirectInputDriver>& pDriver, bool bBackGround=false);
	void Fin();

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
