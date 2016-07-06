/**
	Lib_PipeBuildingTile
	

	@author 
*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";


local tile_mode = TILE_MODE_LINE;
local Plane = 15;

local IsPipeBuildingTile = true;

// Needs to be overwritten.
local TileKindPropertyName = "IsPipeBuildingTile";
// Neighbours as a proplist (left/right/up/bottom).
local neighbours;
// Neighbours in a list (always up to date, can contain nil).
local neighbours_as_list;
// Neighbours in 3x3 array. MIGHT not be up to date at all times.
local last_neighbours_as_matrix;
// Amount of current neighbours.
local neighbour_count = 0;

public func Construction()
{
	return inherited();
}

public func Constructed()
{
	_inherited();
	
	neighbour_count = 0;
	neighbours = {left = nil, right = nil, up = nil, bottom = nil};
	neighbours_as_list = [nil, nil, nil, nil];
		
	var current_neighbours = GetNeighboursAsMatrix();
	AddNeighbour("left", current_neighbours[0][1]);
	AddNeighbour("right", current_neighbours[2][1]);
	AddNeighbour("up", current_neighbours[1][0]);
	AddNeighbour("bottom", current_neighbours[1][2]);
	this->UpdateGraphics(current_neighbours);
	
	for (var neighbour in [neighbours.left, neighbours.right, neighbours.up, neighbours.bottom])
	{
		if (!neighbour) continue;
		neighbour->UpdateGraphics(neighbour->GetNeighboursAsMatrix());
	}
}

private func IsOtherPipeType(string pipe_type)
{
	return is_constructed && IsPipeBuildingTile && TileKindPropertyName != pipe_type;
}

private func IsSamePipeType(string pipe_type)
{
	return is_constructed && IsPipeBuildingTile && TileKindPropertyName == pipe_type;
}

private func IsOverlayedByOtherPipe()
{
	return FindObject(Find_AtPoint(), Find_Func("IsOtherPipeType", TileKindPropertyName)) != nil;
}

private func CanSupportOtherPipeTile()
{
	return !IsOverlayedByOtherPipe();
}

public func BuildingCondition(bool crucial)
{
	if (FindObject(Find_AtPoint(), Find_Property("is_constructed"),
		Find_Not(Find_Or(Find_Func("IsWallBuildingTile"), Find_Func("IsPillarBuildingTile"), Find_Func("IsOtherPipeType", TileKindPropertyName))),
		Find_Func("IsBuildingTile"), Find_Exclude(this)))
		return false;

	if (VerticesStuckSemi() == GetVertexNum()+1)
		return false;
	
	var is_other_pipe_there = IsOverlayedByOtherPipe();
	
	// If trying to build an overlay, require at least one non-overlayed tile around.
	if (is_other_pipe_there)
	{
		if (!FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)),
						Find_Exclude(this), Find_Func("IsSamePipeType", TileKindPropertyName), Find_Not(Find_Func("IsOverlayedByOtherPipe"))))
			return false;
		return true;
	}
	else // Otherwise, just require a tile.
	{
		if (FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)),
								Find_Exclude(this), Find_Func("IsSamePipeType", TileKindPropertyName)))
			return true;
		if (FindObject(Find_AtPoint(), Find_Property("is_constructed"), Find_Func("IsPillarBuildingTile"), Find_Exclude(this)))
			return true;
		return false;
	}
}

func Destruct()
{
	_inherited();
	OnDetached();
	OnBecomeUnstable();
}

private func Destroy()
{
	SetCategory(C4D_None);
	OnDetached();
	OnBecomeUnstable();
	
	var particles = 
	{
		Size = PV_KeyFrames(0, 0, 0, 100, PV_Random(2, 5), 1000, 3),
		Alpha = PV_Linear(255, 0),
		ForceY = PV_Gravity(200),
		CollisionVertex = 0,
		Rotation = PV_Random(360)
	};
	particles = Particles_Colored(particles, GetClrModulation());
	CreateParticle("Straw", PV_Random(-build_grid_x/2, +build_grid_x/2), PV_Random(-build_grid_y/2, +build_grid_y/2), PV_Random(-5, 5), PV_Random(-5, 5), PV_Random(10, 30), particles, 50);
	SoundAt("Hits::Materials::Rock::Rockfall*", 0, 0, {pitch = 300, volume = 20});
	if (!Random(2))
		SoundAt("Hits::Materials::Metal::DullMetalHit*", 0, 0, {pitch = 30, volume = 20});
	RemoveObject();
} 

private func AddNeighbour(string direction, object neighbour, bool no_recursion)
{
	if (!neighbour && neighbours[direction])
		--neighbour_count;
	else if (neighbour && !neighbours[direction])
		++neighbour_count;
	var idx_map = {"left" = 0, "right" = 1, "up" = 2, "bottom" = 3};
	neighbours_as_list[idx_map[direction]] = neighbour; 
	neighbours[direction] = neighbour;
	
	this->~OnAddedNeighbour(direction, neighbour);
	if (!neighbour) return;

	if (!no_recursion)
	{
		var mapping = {left = "right", right = "left", up = "bottom", bottom = "up"};
		neighbour->AddNeighbour(mapping[direction], this, true);
	}
}

private func GetNeighboursAsMatrix(bool ignore_cycles)
{
	var blocks = [[nil, nil, nil], [nil, nil, nil], [nil, nil, nil]];
	var x_pos = [-1, +1, 0, 0];
	var y_pos = [0, 0, -1, +1];
	for (var i = 0; i < 4; ++i)
	{
		var block = FindObject(Find_AtPoint(x_pos[i] * build_grid_x, y_pos[i] * build_grid_y), Find_Exclude(this), Find_Property("is_constructed"), Find_Property(TileKindPropertyName));
		if (!block) continue;
		if (ignore_cycles && block.already_found) continue;
		blocks[1 + x_pos[i]][1 + y_pos[i]] = block;
	}
	last_neighbours_as_matrix = blocks;
	return blocks;
}

private func GetNeighbours(bool ignore_cycles)
{
	var blocks = [];
	for (var neighbour in [neighbours.left, neighbours.right, neighbours.up, neighbours.bottom])
	{
		if (!neighbour || (ignore_cycles && neighbour.already_found)) continue;
		PushBack(blocks, neighbour);
	}
	return blocks;
}

private func IsNeighbour(object other)
{
	return GetIndexOf([neighbours.left, neighbours.right, neighbours.up, neighbours.bottom], other) != -1;
}

private func OnDetached()
{
	is_constructed = false;
	
	var mirrored_directions = ["right", "left", "bottom", "up"];
	var neighbour_list = [neighbours.left, neighbours.right, neighbours.up, neighbours.bottom];
	for (var i = 0; i < 4; ++i)
	{
		var neighbour = neighbour_list[i];
		if (!neighbour) continue;
		neighbour->AddNeighbour(mirrored_directions[i], nil);
		neighbour->UpdateGraphics(neighbour->GetNeighboursAsMatrix());
	}
}

private func UpdateGraphics(array neighbours)
{
	neighbours = neighbours || GetNeighboursAsMatrix();
	
	var count = 0;
	for (var axis in neighbours)
		for (var obj in axis)
			if (obj) ++count;
	if (count >= 3 || count == 1)
	{
		SetGraphics("");
	}
	else
	if (count == 2)
	{
		var horizontal = (neighbours[0][1] && neighbours[2][1]);
		var vertical = (neighbours[1][0] && neighbours[1][2]);
		if (vertical)
		{
			SetGraphics("Straight");
			SetR(0);
		}
		else if (horizontal)
		{
			SetGraphics("Straight");
			SetR(90);
		}
		else
		{
			SetGraphics("Edge");
			if (neighbours[1][0] && neighbours[2][1])
				SetR(0);
			else if (neighbours[2][1] && neighbours[1][2])
				SetR(90);
			else if (neighbours[1][2] && neighbours[0][1])
				SetR(180);
			else
				SetR(270);
		}
	}
}

public func OnBecomeUnstable()
{
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
			
			PushBack(neighbour_tiles, neighbour);
			
			if (neighbour->FindObject(Find_AtPoint(0, 0), Find_Property("is_constructed"), Find_Func("IsPillarBuildingTile")))
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