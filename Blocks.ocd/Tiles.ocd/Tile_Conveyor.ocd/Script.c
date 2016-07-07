/**
	Tile_Conveyor
	

	@author 
*/

#include Lib_PipeBuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local Plane = 15;

local IsConveyorBuildingTile = true;
local TileKindPropertyName = "IsConveyorBuildingTile"; 

local network;

protected func Hit(x, y)
{
	StonyObjectHit(x,y);
	return true;
}

func Constructed()
{
	inherited(...);

	if (neighbour_count == 0)
		network = ConveyorNetwork->Create(this);
	else
		// Connect to existing networks.
		for (var neighbour in neighbours_as_list)
			if (neighbour)
			{
				if (!network)
					network = neighbour.network;
				else
					network->Merge(neighbour.network);
			}
	
	AddTimer("CheckObjects", 60 + Random(20));
}

public func Destruct()
{
	inherited(...);

	// Network management
	if (neighbour_count == 0)
		// We were the last node of the network.
		network->RemoveObject();
	else if (neighbour_count > 1)
		// We potentially have disconnected two or more networks.
		for (var neighbour in neighbours_as_list)
			if (neighbour)
				network->Split(neighbour);

	RemoveTimer("CheckObjects");
}

// Recursively updates the network.
public func SetNetwork(object new_network, object previous_network)
{
	if (network != previous_network) return;
	network = new_network;
	for (var neighbour in neighbours_as_list)
		if (neighbour)
			neighbour->SetNetwork(new_network, previous_network);
}

static const ConveyorPathOps = new AStarOps
{
	distance = ObjectDistance,

	successors = func(object node)
	{
		return node->GetNeighbours();
	}
};

public func FindPath(object other)
{
	return AStar(this, other, ConveyorPathOps);
}

private func CheckObjects()
{
	var rect = Find_AtRect(-build_grid_x / 2, -build_grid_y / 2, build_grid_x, build_grid_y * 4);
	var obj = FindObject(rect, Find_Category(C4D_Object), 
		Find_Not(Find_OCF(OCF_HitSpeed1)), Find_NoContainer(), Find_Not(Find_Property("is_being_moved_by_conveyor")),
		Find_PathFree(this)
		);
	if (!obj) return;
	if (FindObject(rect, Find_ID(Tile_ConveyorArmHook))) return;
	
	BeginTransport(obj);
}

private func BeginTransport(object obj)
{
	var base = CreateObject(Tile_ConveyorArmBase, BoundBy(obj->GetX() - GetX(), -build_grid_x / 2, +build_grid_x / 2), 0, NO_OWNER);
	base.hook->MoveToGrab(obj);
	base.home_block = this;
}

local Components = {Wood = 1};
public func IsToolProduct() { return true; }
