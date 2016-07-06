/**
	Lib_BuildingElement
	

	@author KKenny
*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Plane = 5;

local previous_mat;

func IsWallBuildingTile() { return true; }
local tile_mode = TILE_MODE_LINE;

func Constructed()
{
	/*var x = GetX();
	var y = GetY();
	var w = tile_size_x/2;
	var h = tile_size_y/2;
	
	previous_mat = GetMaterial();
	
	DrawMaterialQuad("Tunnel-tunnel", 
		x-w, y-h, x+w, y-h, x+w, y+h, x-w, y+h, DMQ_Sub);*/
	
	//DrawMap(0,0,LandscapeWidth(), LandscapeHeight()/2, "map Tunnel{overlay{mat = Tunnel;};};");
	
	return _inherited();
}

func Redraw()
{
	var x = GetX();
	var y = GetY();
	var w = tile_size_x/2;
	var h = tile_size_y/2;
	
	var name = MaterialName(previous_mat);
	if (previous_mat == -1)
		ClearFreeRect(x-w, y-h, w*2, h*2);
	else
		DrawMaterialQuad(name, 
		x-w, y-h, x+w, y-h, x+w, y+h, x-w, y+h, DMQ_Sub);
}

func Destruct()
{
	//Redraw();
	return _inherited();
}

func BuildingCondition(bool crucial)
{
	if (VerticesStuckSemi() == GetVertexNum()+1)
		return false;

	if(!crucial)
	{
		if (FindObject(Find_AtPoint(), Find_NoContainer(), Find_Func("IsWallBuildingTile"), Find_Not(Find_Func("IsPreview")), Find_Exclude(this)))
			return false;
	
		if (FindObject(Find_Exclude(this), Find_NoContainer(), Find_Func("IsBuildingTile"), Find_Not(Find_Func("IsPreview")),
			Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2))))
			return true;
		
		return false;
	}
	
	return true;
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
			PushBack(neighbour_tiles, neighbour);
			
			if (neighbour->FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)),
									 Find_Property("is_constructed"), Find_Func("IsSolidBuildingTile")))
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

private func GetNeighbours(bool ignore_cycles, bool find_wall_previews)
{
	var blocks = [];
	var x_pos = [-1, +1, 0, 0];
	var y_pos = [0, 0, -1, 1];
	for (var i = 0; i < 4; ++i)
	{
		var block = FindObject(Find_AtPoint(x_pos[i] * build_grid_x, y_pos[i] * build_grid_y), Find_Property("is_constructed"), Find_Func("IsWallBuildingTile"));
		if (!block) continue;
		if (block.already_found && ignore_cycles) continue;
		PushBack(blocks, block);
	}
	return blocks;
}

private func Destroy()
{
	SetCategory(C4D_None);
	
	var color = PV_Random(50, 100, nil, 1);
	var particles = 
	{
		Size = PV_KeyFrames(0, 0, 0, 100, PV_Random(5, 8), 1000, 3),
		R = color, G = color, B = color,
		Alpha = PV_Linear(255, 0),
		ForceY = PV_Gravity(100),
		CollisionVertex = 0,
		Rotation = PV_Random(360)
	};
	CreateParticle("WoodChip", PV_Random(-build_grid_x/2, +build_grid_x/2), PV_Random(-build_grid_y/2, +build_grid_y/2), PV_Random(-2, 2), PV_Random(-2, 2), PV_Random(10, 60), particles, 20);
	RemoveObject();
} 