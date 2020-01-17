#pragma once

#include "DirectInputPadState.h"
#include <functional>

#include "Windows/AllowWindowsPlatformTypes.h"

class FDirectInputDriver;
class FXInputJoystickEmu;
class UDirectInputPadJoystick;

//! PVOと軸の入力方向を示す列挙子
enum EDirectInputArrow : uint8
{
	// 通常のボタンの次からの番号を振る
	POV_UP	  =32,	//!< POV↑
	POV_RIGHT,		//!< POV→
	POV_DOWN,		//!< POV↓
	POV_LEFT,		//!< POV←
	AXIS_UP,		//!< 軸Y↑
	AXIS_RIGHT,		//!< 軸X→
	AXIS_DOWN,		//!< 軸Y↓
	AXIS_LEFT,		//!< 軸X←
	POV_NONE,		//!< POV何も押されていない
	AXIS_NONE,		//!< XY軸何も押されていない
	ARROW_END,
};

/*! @brief ジョイスティック一つを表すクラス
 *
 *	FDirectInputJoystick_enumから、取得した情報を元にデバイスを作成し
 *	実際に入力処理を行う
 *  軸・回転の範囲は、-1.0～1.0 を取る。あそびの範囲にある時は0.0が返る
 *
 *  AD変換をONにすると、LeftAnalogXY軸がPOVとして、POVがLeftAnalogXY軸入力としても扱われる
 *  どの軸がLeftAnalog扱いなのかは、SetUEKeyで設定する
 */
class FDirectInputJoystick
{
public:
	static const int32 MAX_AXIS_VALUE;

	friend class FXInputJoystickEmu;
	friend class UDirectInputPadJoystick;

public:
	FDirectInputJoystick():pDevice_(nullptr),nPlayerIndex_(-1),nCurIndex_(0),bAcquire_(false),
				  bADConv_(false),
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

	//! 現在の入力状態に合わせてイベントを飛ばす
	void Event(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler);

public:
	//! AD変換が有効か
	bool IsAdConvFlag()const{ return bADConv_; }
	//! AD変換フラグ設定
	bool SetAdConvFlag(bool bAD){ bADConv_ = bAD; }

	//! アナログ軸のあそびの閾値設定。この値以下の入力は無効扱い
	void SetAxisThreshold(uint32 nX, uint32 nY, uint32 nZ){ nX_Threshold_=nX; nY_Threshold_=nY; nZ_Threshold_=nZ; }
	//! アナログ回転のあそびの閾値設定。この値以下の入力は無効扱い
	void SetRotThreshold(uint32 nX, uint32 nY, uint32 nZ){ nXrot_Threshold_=nX; nYrot_Threshold_=nY; nZrot_Threshold_=nZ; }

public:
	int32	GetPlayerIndex()const{ return nPlayerIndex_; }
	void	SetPlayerIndex(int32 nPlayerIndex){ nPlayerIndex_ = nPlayerIndex; }

	bool	IsAxisReverse(EDirectInputPadKeyNames ePadAxis)const;
	void	SetAxisReverse(EDirectInputPadKeyNames ePadAxis, bool bReverse);

	//! 入力に変化があったらtrue、なかったらfalse
	bool	IsChangedKeyState()const;

public:
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

	// 初期値の取得
	float InitX()const;
	float InitY()const;
	float InitZ()const;
	float InitRotX()const;
	float InitRotY()const;
	float InitRotZ()const;

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

	//! ボタンが押されているかの一つ前の状態。0～31はボタン。32～はPOVと軸
	bool IsPrevPress(uint32 nBtn)const;

public:
	// 入力ガード
	void SetGuard(bool bGuard);
	bool IsGuard()const{ return bGuard_; }

	//! バッファをクリアする
	void ClearBuf(){ nCurIndex_=0; ::ZeroMemory(joyBuf_, sizeof(joyBuf_)); }

	//! 現在のバッファをクリアする
	void ClearCurBuf(){ ::ZeroMemory(&joyBuf_[nCurIndex_], sizeof(DIJOYSTATE)); }

private:
	// アナログイベントを発生させる
	void EventAnalog(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, float Analog, EDirectInputPadKeyNames ePadName, FKey DIKey);
	// ボタンイベントを発生させる
	void EventButton(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey);
	// POVイベント発生させる
	void EventPov(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler);

	// ボタン押したイベントを発生させる
	void EventButtonPressed(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey);
	// ボタン離したイベントを発生させる
	void EventButtonReleased(const TSharedPtr<FGenericApplicationMessageHandler>& MessageHandler, EDirectInputPadKeyNames ePadName, FKey DIKey);
	

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

	//! 左スティックがデジタル的に押されている
	bool IsAxisPress(enum EDirectInputArrow eArrow)const;
	//! 左スティックがデジタル的に押された瞬間
	bool IsAxisPush(enum EDirectInputArrow eArrow)const;
	//! 左スティックがデジタル的に離れた瞬間
	bool IsAxisRelease(enum EDirectInputArrow eArrow)const;

	//! 左スティックがデジタル的に押されているかどうかのチェック
	/*! @param[in] eArrow チェックする方向
		@param[in] index チェックするバッファインデックス */
	bool IsAxisPressInner(enum EDirectInputArrow eArrow, uint32 nIndex)const;


	//! 右スティックがデジタル的に押されている
	bool IsAxisRightPress(enum EDirectInputArrow eArrow)const;
	//! 右スティックがデジタル的に押された瞬間
	bool IsAxisRightPush(enum EDirectInputArrow eArrow)const;
	//! 右スティックがデジタル的に離れた瞬間
	bool IsAxisRightRelease(enum EDirectInputArrow eArrow)const;

	//! 右スティックがデジテル的に押されているかのチェック
	bool IsAxisRightPressInner(enum EDirectInputArrow eArrow, uint32_t nIndex)const;

	//! POV_*とAXIS_* を相互変換する
	enum EDirectInputArrow SwapPovAxis(enum EDirectInputArrow eArrow)const;

private:
	//! DirectInputDevice::EnumObjectsに渡すコールバック関数
	static BOOL CALLBACK OnEnumAxis(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

private:
	//! デバイス
	LPDIRECTINPUTDEVICE8	pDevice_;

	// 対応するPlayerIndex
	int32		nPlayerIndex_;

	//! ジョイスティックの初期値
	DIJOYSTATE	InitialJoyBuf_;

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
	bool		bGuard_;

private:
	// 軸の反転フラグ
	TArray<bool>	AxisReverseFlagMap_;
};

#include "Windows/HideWindowsPlatformTypes.h"
