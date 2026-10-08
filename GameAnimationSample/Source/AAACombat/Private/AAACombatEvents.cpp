// Copyright (c) AAA Multiplayer RPG. All Rights Reserved.

#include "AAACombatEvents.h"

FAAAOnHitConfirmedNative& FAAACombatEvents::OnHitConfirmed()
{
	static FAAAOnHitConfirmedNative Delegate;
	return Delegate;
}

FAAAOnBlockedNative& FAAACombatEvents::OnBlocked()
{
	static FAAAOnBlockedNative Delegate;
	return Delegate;
}

FAAAOnParriedNative& FAAACombatEvents::OnParried()
{
	static FAAAOnParriedNative Delegate;
	return Delegate;
}

FAAAOnStaggeredNative& FAAACombatEvents::OnStaggered()
{
	static FAAAOnStaggeredNative Delegate;
	return Delegate;
}

FAAAOnDeathNative& FAAACombatEvents::OnDeath()
{
	static FAAAOnDeathNative Delegate;
	return Delegate;
}
