#pragma once
#include "Monster.h"
#include "Geometric.h"
#include "Bone.h"
#include <memory>

class Belatos final: public Monster 
{
public:
	using MyType = Belatos;
	using Super = Monster;
public:
	struct StateDuringSpeed
	{
		float Run = 25.f;
	};
	enum class State : uint8
	{
		Wait = 0u,
		Run,
		RunEnd,
		Skill1st,
		Skill2nd,
		RTStand,

		Air,

		DownBack,//Down_B_Belatos_Twohandedsword
		DownFront,//Down_F_Belatos_Twohandedsword
		Respawn,//Respawn_Belatos_Twohandedsword

		RTChaseBigBack,//RTChase_Big_B_Belatos_Twohandedsword
		RTChaseBigFront,//RTChase_Big_F_Belatos_Twohandedsword
		RTChaseBigLeft,//RTChase_Big_L_Belatos_Twohandedsword
		RTChaseBigRight,//RTChase_Big_R_Belatos_Twohandedsword
		
		RTChaseSmallBack,//RTChase_Small_B_Belatos_Twohandedsword

		RTDownAirFallBack,//RTDown_Air_Fall_B_Belatos_Twohandedsword
		RTDownAirFallFront,//RTDown_Air_Fall_F_Belatos_Twohandedsword
		RTDownAirLandingBack,//RTDown_Air_Landing_B_Belatos_Twohandedsword
		RTDownAirLandingBackLeft,//RTDown_Air_Landing_B_L_Belatos_Twohandedsword

		RTDownBack,//RTDown_B_Belatos_Twohandedsword
		RTDownFront,//RTDown_F_Belatos_Twohandedsword
		RTStandAirBigFront,//RTStand_Air_Big_F_Belatos_Twohandedsword
		RTStandAirBigBack,//RTStand_Air_Big_B_Belatos_Twohandedsword
		RTStandAirFall,//RTStand_Air_Fall_Belatos_Twohandedsword
		RTStandAirLanding,//RTStand_Air_Landing_Belatos_Twohandedsword
		RTStandAirSmall,//RTStand_Air_Small_Belatos_Twohandedsword

		StandUpBack,//StandUp_B_Belatos_Twohadnedsword
		StandUpFront,//StandUp_F_Belatos_Twohadnedsword 
	};
	virtual void FSM(const float DeltaTime)& override;
	virtual void Edit()& override;
	virtual void HitNotify(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitBegin(Object* const Target, const Vector3 PushDir,
		const float CrossAreaScale) & override;
	virtual void HitEnd(Object* const Target) & override;
	virtual void Event() & override;
	virtual void Update(const float DeltaTime)&;
	virtual void LateUpdate(const float DeltaTime)&;
	virtual std::function< SpawnReturnValue(const SpawnParam&)>
				PrototypeEdit() & override;
	void PrototypeInitialize(IDirect3DDevice9* const Device) & ;

	void Initialize(const std::optional<Vector3>& Scale, const std::optional<Vector3>& Rotation, const Vector3& SpawnLocation)&;

	virtual std::shared_ptr<Engine::Object> GetCopyShared() & override;
	virtual std::optional<Engine::Object::SpawnReturnValue> InitializeFromEditSpawnParam(const SpawnParam& _SpawnParam) & override;
private:
	void WaitTransition(const FSMControlInformation& FSMControlInfo)&;
	void WaitState(const FSMControlInformation& FSMControlInfo)&;
	void RunState(const FSMControlInformation& FSMControlInfo)&;
	void RunTransition(const FSMControlInformation& FSMControlInfo)&;
	void RunEndState(const FSMControlInformation& FSMControlInfo)&;
	void RunEndTransition(const FSMControlInformation& FSMControlInfo)&;

	void Skill1stState(const FSMControlInformation& FSMControlInfo)&;
	void Skill1stTransition(const FSMControlInformation& FSMControlInfo)&;
	void Skill2ndState(const FSMControlInformation& FSMControlInfo)&;
	void Skill2ndTransition(const FSMControlInformation& FSMControlInfo)&;

	void RespawnState(const FSMControlInformation& FSMControlInfo)&;
	void RespawnTransition(const FSMControlInformation& FSMControlInfo)&;
	void RTStandState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandTransition(const FSMControlInformation& FSMControlInfo)&;

	void AirState(const FSMControlInformation& FSMControlInfo)&;
	void AirTransition(const FSMControlInformation& FSMControlInfo)&;

	// Ω√¿€
	void DownBackState(const FSMControlInformation& FSMControlInfo)&;
	void DownBackTransition(const FSMControlInformation& FSMControlInfo)&;

	void DownFrontState(const FSMControlInformation& FSMControlInfo)&;
	void DownFrontTransition(const FSMControlInformation& FSMControlInfo)&;

	void RTChaseSmallBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTChaseSmallBackTransition(const FSMControlInformation& FSMControlInfo)&;
	
	void RTChaseBigBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTChaseBigBackTransition(const FSMControlInformation& FSMControlInfo)&;

	
	void RTChaseBigFrontState(const FSMControlInformation& FSMControlInfo)&;
	void RTChaseBigFrontTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void RTChaseBigLeftState(const FSMControlInformation& FSMControlInfo)&;
	void RTChaseBigLeftTransition(const FSMControlInformation& FSMControlInfo)&;

	
	//
	void RTChaseBigRightState(const FSMControlInformation& FSMControlInfo)&;
	void RTChaseBigRightTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void RTDownAirFallBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownAirFallBackTransition(const FSMControlInformation& FSMControlInfo)&;

	////
	void RTDownAirFallFrontState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownAirFallFrontTransition(const FSMControlInformation& FSMControlInfo)&;

	////
	void RTDownAirLandingBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownAirLandingBackTransition(const FSMControlInformation& FSMControlInfo)&;

	////
	void RTDownAirLandingBackLeftState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownAirLandingBackLeftTransition(const FSMControlInformation& FSMControlInfo)&;

	////
	void RTDownBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownBackTransition(const FSMControlInformation& FSMControlInfo)&;

	////
	void RTDownFrontState(const FSMControlInformation& FSMControlInfo)&;
	void RTDownFrontTransition(const FSMControlInformation& FSMControlInfo)&;
	
	//
	void RTStandAirBigFrontState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandAirBigFrontTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void RTStandAirBigBackState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandAirBigBackTransition(const FSMControlInformation& FSMControlInfo)&;


	//
	void RTStandAirFallState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandAirFallTransition(const FSMControlInformation& FSMControlInfo)&;


	//
	void RTStandAirLandingState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandAirLandingTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void RTStandAirSmallState(const FSMControlInformation& FSMControlInfo)&;
	void RTStandAirSmallTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void StandUpBackState(const FSMControlInformation& FSMControlInfo)&;
	void StandUpBackTransition(const FSMControlInformation& FSMControlInfo)&;

	//
	void StandUpFrontState(const FSMControlInformation& FSMControlInfo)&;
	void StandUpFrontTransition(const FSMControlInformation& FSMControlInfo)&;
private:
	void WeaponAttackCollisionSweep(const FSMControlInformation& FSMControlInfo)&;
	void DissolveStart(const float SliceAmountSpeed, const float SliceAmoutStart)&;
private:
	
	float SliceAmountSpeed = 1.f;
	Vector3 WeaponLocalMin{};
	Vector3 WeaponLocalMax{};
	Engine::Bone* WeaponHandleBone{ nullptr };
	StateDuringSpeed StateableSpeed{};
	State   CurrentState{ Belatos::State::Wait };
};