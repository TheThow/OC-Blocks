/**
	Tile_SteamPipe
	

	@author 
*/

#include Lib_PipeBuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local Plane = 14;

local IsSteamPipeBuildingTile = true;
local TileKindPropertyName = "IsSteamPipeBuildingTile";

local steam_level = 0;

local open_offset_x;
local open_offset_y;

local steam_particles;

local pressure_valve;

public func Definition()
{
	this.steam_particles = new Particles_Smoke()
	{
		Size = PV_Linear(0, PV_Random(20, 50)),
		ForceX = PV_Random(-5, 5, 10),
		ForceY = PV_Random(-5, 1, 10),
		R = 255, G = 255, B = 255, 
		Alpha = PV_Linear(16, 0),
		Rotation = PV_Random(360)
	};
	return _inherited();
}

public func Construction()
{
	return inherited();
}

protected func Hit(x, y)
{
	StonyObjectHit(x,y);
	return true;
}

public func Constructed()
{
	_inherited();

	if (neighbour_count != 1)
	{
		open_offset_x = nil;
		open_offset_y = nil;
	}
	
	AddTimer("CheckSteam", 10 + Random(3));
	AddTimer("CheckBurningObjects", 20 + Random(10));
}

public func CheckOpenOffset()
{
	var directions = ["left", "right", "up", "bottom"];

	open_offset_x = 0;
	open_offset_y = 0;

	for (var i = 0; i < 4; ++i)
	{
		var neighbour = neighbours_as_list[i];
		if (!neighbour) continue;
		var direction = directions[i];
		
		if (direction == "left")
			open_offset_x = build_grid_x/2;
		else if (direction == "right")
			open_offset_x = -build_grid_x/2;
		else if (direction == "up")
			open_offset_y = +build_grid_y/2;
		else if (direction == "bottom")
			open_offset_y = -build_grid_y/2;
	}
	
	if (neighbour_count != 1)
	{
		open_offset_x = nil;
		open_offset_y = nil;
	}
}

public func Destruct()
{
	RemoveTimer("CheckSteam");
	RemoveTimer("CheckBurningObjects");
	if (pressure_valve)
		pressure_valve->Destruct();
	return inherited(...);
}

public func Destroy()
{
	if (pressure_valve)
		pressure_valve->RemoveObject();
	return inherited();
}

private func CheckBurningObjects()
{
	var total_fire_value = 0;
	for (var obj in FindObjects(Find_AtRect(-build_grid_x, -build_grid_y, 2 * build_grid_x, 5 * build_grid_y), Find_Func("OnFire")))
	{
		var fuel_value = Min(10, obj->GetMass() / 5) + obj->~GetFuelAmount() / 10;
		var fire_amount = obj->OnFire();
		total_fire_value += fire_amount * fuel_value / 100;
	}
	
	if (GetMaterialVal("Incendiary", "Material", GetMaterial(0, RandomX(build_grid_y, 5 * build_grid_y))))
		total_fire_value += 10;
	
	DoSteam(total_fire_value);
}

private func CheckSteam()
{
	if (steam_level <= 0) return;
	
	if (pressure_valve)
	{
		if (!pressure_valve.is_open)
		{
			if (steam_level > 150)
				pressure_valve->Animate();
			return;
		}
	}
	
	var min_steam_neighbour = nil;
	var min_steam_level = 0;
	for (var neighbour in neighbours_as_list)
	{
		if (!neighbour) continue;
		var neighbour_steam_level = neighbour.steam_level;
		if (!min_steam_neighbour || neighbour_steam_level < min_steam_level)
		{
			min_steam_neighbour = neighbour;
			min_steam_level = neighbour_steam_level;
		}
	}
	
	var change = 0;
	if (min_steam_neighbour)
		change = 3 * (steam_level - min_steam_level) / 4;
		
	if (!(change > 0))
	{
		if (open_offset_x != nil)
		{
			var loss = Max(5, steam_level / 4);
			CreateParticle("Dust", open_offset_x, open_offset_y, 0, 0, PV_Random(10, 200), this.steam_particles, loss);
			DoSteam(-loss);
			if (!Random(3))
				Sound("Liquids::Sizzle", {pitch = 5 + Random(5), volume = 5});
		}
		else
		{
			var rnd = Random(steam_level);
			var limit = 100 * (5 - neighbour_count);
			if (rnd > limit)
			{
				CreateParticle("Dust", PV_Random(-4, 4), PV_Random(-4, 4), PV_Random(-10, 10), PV_Random(-10, 10), PV_Random(5, 20), this.steam_particles, 5 * steam_level);
				Destroy();
			}
			else
			if (rnd > limit / 2)
			{
				if (!Random(3))
					SoundAt("Liquids::Boiling");
				CreateParticle("Dust", PV_Random(-4, 4), PV_Random(-4, 4), PV_Random(-10, 10), PV_Random(-10, 10), PV_Random(5, 20), this.steam_particles, 20);
			}
			
		}
		return;
	}
	
	min_steam_neighbour->DoSteam(change);
	DoSteam(-change);
}

public func DoSteam(int change)
{
	steam_level = Max(0, steam_level + change);
	var discolored = 255 - Min(steam_level, 200);
	SetClrModulation(RGB(255, discolored, discolored));
}

public func OnAddedNeighbour(string direction, object neighbour)
{
	CheckOpenOffset();
	if (neighbour)
		neighbour->CheckOpenOffset();
}

local Components = {Wood = 1};
public func IsToolProduct() { return true; }
