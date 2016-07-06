/**
	Lib_BuildingElement
	

	@author KKenny
*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Plane = 55;

func MaxStackCount() { return 48; }
func InitialStackCount() { return 4; }

func IsPlatformBuildingTile() { return true; }
local tile_mode = TILE_MODE_HORIZONTAL_LINE;

local support;
local supportID = nil;

func Initialize()
{
	SetHalfVehicleSolidMask(true);
	return _inherited();
}

func Construction()
{
	support = CreateArray(2);
	
	return _inherited();
}


func Constructed()
{
	SetSolidMask(0,0,10,10);
	SetHalfVehicleSolidMask(true);
	AdjustSupport();
	return _inherited();
}

func Destruct()
{
	SetSolidMask();
	RemoveSupport();
	return _inherited();
}

func AdjustSupport()
{
	if (!supportID)
		return;

	RemoveSupport();

	if (FindObject(Find_AtPoint(-tile_size_x, 0), Find_Func("IsSolidBuildingTile"), Find_Property("is_constructed")))
	{
		if(!support[0])
			support[0] = CreateObject(supportID, 0, 0, GetController());
	}

	if (FindObject(Find_AtPoint(tile_size_x, 0), Find_Func("IsSolidBuildingTile"), Find_Property("is_constructed")))
	{
		if(!support[1])
		{
			support[1] = CreateObject(supportID, 0, 0, GetController());
			support[1]->SetObjDrawTransform(-1000, 0, 0, 0, 1000);
		}
	}
}

func RemoveSupport()
{
	for (var s in support)
	{
		if(s)
			s->RemoveObject();
	}
}


func BuildingCondition(bool crucial)
{
	if (VerticesStuckSemi() == GetVertexNum()+1)
		return false;

	if(!crucial)
	{
		if (FindObject(Find_AtPoint(), Find_NoContainer(), Find_Func("IsPlatformBuildingTile"), Find_Not(Find_Func("IsPreview")), Find_Exclude(this)))
			return false;
	
		if (FindObject(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0),
			Find_Exclude(this), Find_Property("is_constructed"), Find_Or(Find_Func("IsSolidBuildingTile"), Find_Func("IsPlatformBuildingTile"))))
			return true;
		
		return false;
	}
	
	return true;
}

private func Destroy()
{
	SetCategory(C4D_None);
	RemoveSupport();
	
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
