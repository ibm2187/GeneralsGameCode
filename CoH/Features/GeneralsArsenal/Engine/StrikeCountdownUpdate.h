// CoH GeneralsArsenal: the module on a super weapon's target marker. A super weapon power drops the
// marker on its target (an OCL); from then on every player sees the target and a countdown (UI), and
// when it runs out the weapon fires. The weapon comes from a host: a hidden copy of the old super
// weapon building, made at the map edge as the countdown starts so it can charge, which then fires
// its own power at the marker. So the beam, missile or Scud Storm look and hit as before.
//
//     Behavior = StrikeCountdownUpdate ModuleTag_CoHCountdown
//       Delay        = 20000          ; ms from targeting to firing
//       Host         = CoHHost_AmericaParticleCannonUplink
//       HostPower    = CoHHost_ParticleUplinkCannon
//       HostLifetime = 30000          ; ms the host and marker stay after firing
//       Label        = CONTROLBAR:FireParticleUplinkCannon
//       Radius       = 200            ; of the ring drawn on the ground
//       WarnEveryone = Yes            ; No: only the owner and allies see it
//     End

#pragma once

#include "GameLogic/Module/UpdateModule.h"

#include "Features/GeneralsArsenal/Domain/Warning.h"

class SpecialPowerTemplate;

class StrikeCountdownUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_delayFrames;
	AsciiString m_hostName;
	const SpecialPowerTemplate *m_hostPower;
	UnsignedInt m_hostLifetimeFrames;
	AsciiString m_label;
	Real m_radius;
	Bool m_warnEveryone;

	StrikeCountdownUpdateModuleData();
	static void buildFieldParse( MultiIniFieldParse &p );
};

class StrikeCountdownUpdate : public UpdateModule
{
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StrikeCountdownUpdate, StrikeCountdownUpdateModuleData );
	MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE( StrikeCountdownUpdate, "StrikeCountdownUpdate", 16, 16 )	// not in the engine's pool table

public:
	StrikeCountdownUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update() override;

	CoH::GeneralsArsenal::WarningTimer timer() const;
	Bool hasFired() const { return m_fired; }
	Real getRadius() const { return getStrikeCountdownUpdateModuleData()->m_radius; }
	const AsciiString &getLabel() const { return getStrikeCountdownUpdateModuleData()->m_label; }
	Bool warnsEveryone() const { return getStrikeCountdownUpdateModuleData()->m_warnEveryone; }

	Object *ownerObject() { return getObject(); }	///< for the UI

	static StrikeCountdownUpdate *find( Object *object );

private:
	void createHost();
	void fire();
	void finish();

	UnsignedInt m_startFrame;
	ObjectID m_hostID;
	Bool m_fired;
	UnsignedInt m_doneFrame;
};
