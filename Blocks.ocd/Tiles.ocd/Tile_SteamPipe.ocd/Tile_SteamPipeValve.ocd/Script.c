/**
	Tile_SteamPipeValve
	

	@author 
*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local Plane = 16;
local pipe;
local is_open = false;

protected func Hit(x, y)
{
	StonyObjectHit(x,y);
	return true;
}

public func BuildingCondition(bool crucial)
{
	if (FindObject(Find_AtPoint(), Find_ID(GetID()), Find_Property("is_constructed"), Find_Exclude(this)))
		return false;
	if (!FindObject(Find_AtPoint(), Find_Property("IsSteamPipeBuildingTile"), Find_Property("is_constructed"), Find_Exclude(this)))
		return false;
	return true;
}

public func Constructed()
{
	pipe = FindObject(Find_AtPoint(), Find_Property("is_constructed"), Find_Property("IsSteamPipeBuildingTile"));
	if (!pipe) return RemoveObject();
	pipe.pressure_valve = this;
	return _inherited();
}


public func Destruct()
{
	return inherited(...);
}

local AnimationEffect = new Effect 
{
	Start = func()
	{
		this.Target.is_open = true;
	},
	Stop = func()
	{
		this.Target.is_open = false;
	},
	Timer = func(int time)
	{
		var angle = time / 4;
		this.Target->SetR(Sin(angle, 360));
		if (angle == 90) return FX_Execute_Kill;
	}
};

public func Animate()
{
	CreateEffect(AnimationEffect, 1, 1);
}

local Components = {Wood = 1};
public func IsToolProduct() { return true; }
