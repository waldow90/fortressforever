//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TRIGGERS_H
#define TRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "basetoggle.h"
#include "entityoutput.h"

//
// Spawnflags
//

enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects

	// New FF stuff.  Be careful not to add too many or we'll run out of bits
	// Also, there's already a #define SF_VPHYSICS_MOTION_MOVEABLE	0x1000 ... and also 0x800 is skipped; possibly reserved?
	SF_TRIGGER_ALLOW_FF_GRENADES			= 0x2000,	// Allow grenades
	SF_TRIGGER_ALLOW_FF_BUILDABLES			= 0x4000,	// Allow buildables
	//SF_TRIGGER_ALLOW_FF_INFOSCRIPTS			= 0x8000,	// info_ff_scripts
};

// DVS TODO: get rid of CBaseToggle
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBaseTrigger : public CBaseToggle
{
	DECLARE_CLASS( CBaseTrigger, CBaseToggle );
public:
	CBaseTrigger();
	
	void Activate( void );
	virtual void PostClientActive( void );
	void InitTrigger( void );

	void Enable( void );
	void Disable( void );
	void Spawn( void );
	bool CreateVPhysics( void );
	void UpdateOnRemove( void );

	// Input handlers
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggle( inputdata_t &inputdata );

	virtual void InputStartTouch( inputdata_t &inputdata );
	virtual void InputEndTouch( inputdata_t &inputdata );

	virtual bool PassesTriggerFilters(CBaseEntity *pOther);
	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);
	bool IsTouching( CBaseEntity *pOther );

	CBaseEntity *GetTouchedEntityOfType( const char *sClassName );

	int	 DrawDebugTextOverlays(void);

	// by default, triggers don't deal with TraceAttack
	void TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType) {}

	bool		m_bDisabled;
	string_t	m_iFilterName;
	CHandle<class CBaseFilter>	m_hFilter;

protected:

	// Outputs
	COutputEvent m_OnStartTouch;
	COutputEvent m_OnEndTouch;
	COutputEvent m_OnEndTouchAll;

	// Entities currently being touched by this trigger
	CUtlVector< EHANDLE >	m_hTouchingEntities;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: Variable sized repeatable trigger.  Must be targeted at one or more entities.
//			If "delay" is set, the trigger waits some time after activating before firing.
//			"wait" : Seconds between triggerings. (.2 default/minimum)
//-----------------------------------------------------------------------------
class CTriggerMultiple : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerMultiple, CBaseTrigger );
public:
	void Spawn( void );
	void MultiTouch( CBaseEntity *pOther );
	void MultiWaitOver( void );
	void ActivateMultiTrigger(CBaseEntity *pActivator);

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;
};

// Global list of triggers that care about weapon fire
extern CUtlVector< CHandle<CTriggerMultiple> >	g_hWeaponFireTriggers;


//------------------------------------------------------------------------------
// Base VPhysics trigger implementation
// NOTE: This uses vphysics to compute touch events.  It doesn't do a per-frame Touch call, so the 
// Entity I/O is different from a regular trigger
//------------------------------------------------------------------------------
#define SF_VPHYSICS_MOTION_MOVEABLE	0x1000

class CBaseVPhysicsTrigger : public CBaseEntity
{
	DECLARE_CLASS( CBaseVPhysicsTrigger , CBaseEntity );

public:
	DECLARE_DATADESC();

	virtual void Spawn();
	virtual void UpdateOnRemove();
	virtual bool CreateVPhysics();
	virtual void Activate( void );
	virtual bool PassesTriggerFilters(CBaseEntity *pOther);

	// UNDONE: Pass trigger event in or change Start/EndTouch.  Add ITriggerVPhysics perhaps?
	// BUGBUG: If a player touches two of these, his movement will screw up.
	// BUGBUG: If a player uses crouch/uncrouch it will generate touch events and clear the motioncontroller flag
	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );

	void InputToggle( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

protected:
	bool						m_bDisabled;
	string_t					m_iFilterName;
	CHandle<class CBaseFilter>	m_hFilter;
};

// ##################################################################################
//	>> func_ff_script
// ##################################################################################
class CFuncFFScript : public CTriggerMultiple
{
	DECLARE_CLASS( CFuncFFScript, CTriggerMultiple );

	// Goal States:
	enum { GS_INACTIVE = 0, GS_ACTIVE = 1, GS_REMOVED = -1 };

public:
	CFuncFFScript() ;

	virtual bool	IsActive( void ) const		{ return m_iGoalState == GS_ACTIVE; }
	virtual bool	IsInactive( void ) const	{ return m_iGoalState == GS_INACTIVE; }
	virtual bool	IsRemoved( void ) const		{ return m_iGoalState == GS_REMOVED; }

	virtual void	Spawn( void );
	virtual int		UpdateTransmitState( void );

	void SetBotGoalInfo(int _type, int _team);

	virtual void	LuaRestore( void )			{ SetRestored(); SetInactive(); Enable(); }
	virtual void	LuaRemove( void )			{ SetRemoved(); Disable(); }
	virtual void	LuaSetLocation();

	virtual Class_T Classify( void )			{ return CLASS_TRIGGERSCRIPT; }

	virtual void	SetActive( void );
	virtual void	SetInactive( void );
	virtual void	SetRemoved( void );
	virtual void	SetRestored( void );

	// bot info accessors
	int GetBotTeamFlags() const { return m_BotTeamFlags; }
	int GetBotGoalType() const { return m_BotGoalType; }
protected:
	int	m_iGoalState;
	int m_iClipMask;

	// cached information for bot use
	int		m_BotTeamFlags;
	int		m_BotGoalType;
};

#endif // TRIGGERS_H
