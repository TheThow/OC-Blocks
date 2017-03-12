/**
	Lib_BuildingElement
	

	@author KKenny
*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Plane = 200;

local tile_mode = TILE_MODE_LINE;

local HitPoints = 50;

func IsSolidBuildingTile() { return true; }

local AutoFillWallDefinition = nil;

func Constructed()
{
	is_constructed = true;
	SetSolidMask(0,0,tile_size_x,tile_size_y);
	AdjustSurroundingMaterial(true, true, true, true);
	CheckAutoFillWall();
	CheckPlatforms();
	return _inherited();
}

func CheckPlatforms()
{
	for (var o in FindObjects(Find_Or(Find_AtPoint(-tile_size_x, 0), Find_AtPoint(tile_size_x, 0)), Find_Func("IsPlatformBuildingTile"), Find_NoContainer(), Find_Not(Find_Func("IsPreview"))))
		o->AdjustSupport();
}


func BuildingCondition(bool crucial)
{
	if (VerticesStuckSemi() == GetVertexNum()+1)
		return false;
	
	if (FindObject(Find_AtRect(-tile_size_x/2, -tile_size_y/2, tile_size_x, tile_size_y), 
		Find_Exclude(this), 
		Find_NoContainer(),
		Find_Not(Find_Category(C4D_Object)),
		Find_Not(Find_Func("IsPreview")), 
		Find_Not(Find_Func("IsWallBuildingTile"))))
		return false;

	if(!crucial)
	{
		if ((GBackSolid(-(tile_size_x), 0) || GBackSolid(tile_size_x, 0) || GBackSolid(0, (tile_size_y)) || GBackSolid(0,-(tile_size_y))))
			return true;
		
		if (FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)), 
			Find_Exclude(this), Find_NoContainer(), Find_Not(Find_Func("IsPreview")), Find_Func("IsSolidBuildingTile")))
			return true;
		
		if (FindObject(Find_AtPoint(0, tile_size_y), Find_NoContainer(), Find_Not(Find_Func("IsPreview")), Find_Func("IsPillarBuildingTile")))
			return true;
	
		return false;
	}
	
	return true;
}

func Destruct()
{
	is_constructed = false;
	SetSolidMask();
	_inherited();
	OnBecomeUnstable();
	CheckPlatforms();
}

private func Destroy()
{
	is_constructed = false;
	SetCategory(C4D_None);
	SetSolidMask();
	OnBecomeUnstable();
	CheckPlatforms();
	
	var particles = 
	{
		Size = PV_KeyFrames(0, 0, 0, 100, PV_Random(3, 5), 1000, 3),
		R = PV_Random(100, 255, nil, 1), G = PV_Random(100, 255, nil, 1), B = PV_Random(100, 255, nil, 1),
		Alpha = PV_Linear(255, 0),
		ForceY = PV_Gravity(100),
		CollisionVertex = 0
	};
	CreateParticle("SmokeDirty", PV_Random(-build_grid_x/2, +build_grid_x/2), PV_Random(-build_grid_y/2, +build_grid_y/2), PV_Random(-10, 10), PV_Random(-10, 10), PV_Random(10, 60), particles, 50);
	Sound("Hits::Materials::Rock::Rockfall*");
	RemoveObject();
} 

public func OnBecomeUnstable()
{
	// Possibly kill walls.
	for (var neighbour in GetNeighbours(nil, true))
		if (neighbour) neighbour->CheckSupport();
	// Remove a supported pillar.
	var pillar = FindObject(Find_AtPoint(0, -build_grid_y), Find_Category(C4D_StaticBack), Find_Func("IsPillarBuildingTile"));
	if (pillar) pillar->Destroy();
	// And then make the neighbours check their stability.
	if (this && this.no_propagation) return;
	if (!this) return;
	for (var neighbour in GetNeighbours())
		if (neighbour) neighbour->CheckSupport();
}

public func CheckSupport()
{
	this.already_found = true;
	var has_support = false;
	var neighbour_tiles = [this];
	for (var i = 0; i < GetLength(neighbour_tiles); ++i)
	{
		var current = neighbour_tiles[i];
		if (!current) continue;
		
		for (var neighbour in current->GetNeighbours(true))
		{
			neighbour.already_found = true;
			//if (ObjectDistance(this, neighbour) < 100)
			PushBack(neighbour_tiles, neighbour);
			
			if ((neighbour->GBackSolid(0, build_grid_y) && (neighbour->GetMaterial(0, build_grid_y) != Material("Vehicle")))
			|| (neighbour->FindObject(Find_AtPoint(0, build_grid_y), Find_Category(C4D_StaticBack), Find_Func("IsPillarBuildingTile"))))
			{
				has_support = true;
				break;
			} 
		}
		
		if (has_support) break;
	}
	
	for (var neighbour in neighbour_tiles)
	{
		if (!neighbour) continue;
		
		if (!has_support)
		{
			neighbour.no_propagation = true;
			neighbour->Destroy();
		}
		else
		{
			neighbour.already_found = nil;
		}
	}
}

/*-- Damage Handling --*/

public func GetHitPoints()
{
	return this.HitPoints;
}

public func GetRemainingHitPoints()
{
	return this.HitPoints - GetDamage();
}

public func Damage(int change, int cause, int cause_plr)
{
	if (!this.take_direct_damage)
	{
		var deminishing_damage = change / 2;
		if (GetDamage() > this.HitPoints)
			deminishing_damage += (GetDamage() - this.HitPoints) * 2;
		this.already_found = true;
		var neighbour_tiles = [this];
		for (var i = 0; i < GetLength(neighbour_tiles); ++i)
		{
			if (deminishing_damage <= 0) break;
			
			var current = neighbour_tiles[i];
			if (!current) continue;
			
			for (var neighbour in current->GetNeighbours(true))
			{
				neighbour.already_found = true;;
				PushBack(neighbour_tiles, neighbour);
			}
			
			if (i == 0) continue;
			
			current.take_direct_damage = true;
			current->DoDamage(deminishing_damage, cause, cause_plr);
			if (current)
				current.take_direct_damage = false;
			
			deminishing_damage -= 4;
			
			// A whole column might have been collapsed (including myself).
			if (!this) break;
		}
		
		for (var neighbour in neighbour_tiles)
		{
			if (neighbour) neighbour.already_found = nil;
		}
	}
	
	if (!this) return;
	if (GetDamage() > this.HitPoints)
	{
		Destroy();
	}
	if (this)
		UpdateDamageDisplay();
}

private func GetNeighbours(bool ignore_cycles, bool find_wall_previews)
{
	var check_func = "IsSolidBuildingTile";
	if (find_wall_previews)
		check_func = "IsWallBuildingTile";
	var blocks = [];
	var x_pos = [-1, +1, 0, 0];
	var y_pos = [0, 0, -1, 1];
	for (var i = 0; i < 4; ++i)
	{
		var block = FindObject(Find_AtPoint(x_pos[i] * build_grid_x, y_pos[i] * build_grid_y), Find_Property("is_constructed"), Find_Func(check_func));
		if (!block) continue;
		if (block.already_found && ignore_cycles) continue;
		PushBack(blocks, block);
	}
	return blocks;
}

private func UpdateDamageDisplay()
{
	ClearParticles();
	var particles = 
	{
		R = PV_Random(0, 50), G = PV_Random(0, 50), B = PV_Random(0, 50),
		Rotation = PV_Random(0, 360),
		Stretch = PV_Random(0, 4000),
		Size = PV_Random(0, 3),
		Attach = ATTACH_Front | ATTACH_MoveRelative
	};
	CreateParticle("Fire", PV_Random(-4, 4), PV_Random(-4, 4), 0, 0, 0, particles, 3 * GetDamage() / 2);
}

private func CheckAutoFillWall()
{
	if (!AutoFillWallDefinition) return;
	
	for (var direction = -1; direction <= +1; direction += 2)
	{
		var offset_y = 0;
		var valid = false;
		do
		{
			offset_y += direction * build_grid_y;
			var absolute_y = GetY() + offset_y;
			if (absolute_y < 0 || absolute_y >= LandscapeHeight()) break;
			
			if (FindObject(Find_AtPoint(0, offset_y), Find_ID(GetID()), Find_Property("is_constructed")))
			{
				valid = true;
				break;
			}
			
			if (GBackSolid(0, offset_y)) break;
		} while (true);
		
		if (!valid) continue;
		
		var y = 0;
		while (y != offset_y)
		{
			y += direction * build_grid_y;
			var wall = CreateObject(AutoFillWallDefinition, 0, y, GetOwner());
			wall->PreviewMode();
			if (!wall->BuildingCondition())
			{
				wall->RemoveObject();
				continue;
			}
			wall->RemoveObject();
			wall = CreateObject(AutoFillWallDefinition, 0, y, GetOwner());
			wall->Constructed();
		}
	}
}
