/*--- Amethyst ---*/

#include Lib_BuildingTile

local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local IsMovingBlockBuildingTile = true;

local root = nil;
local children;
local moving_fx;

public func IsRoot()
{
	return root == this;
}

public func BuildingCondition()
{
	if (FindObject(Find_AtPoint(), Find_Property("is_constructed"),
		Find_Not(Find_Or(Find_Func("IsWallBuildingTile"), Find_Func("IsPillarBuildingTile"), Find_Property("IsPipeBuildingTile"))),
		Find_Func("IsBuildingTile"), Find_Exclude(this)))
		return false;

	if (VerticesStuckSemi() == GetVertexNum()+1)
		return false;
	
	var steam_pipe = FindObject(Find_AtPoint(), Find_Property("is_constructed"), Find_Property("IsSteamPipeBuildingTile"), Find_Exclude(this));
	if (steam_pipe) return true;
	
	if (FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)),
						Find_Exclude(this), Find_Property("IsMovingBlockBuildingTile")))
		return true;
	return false;
}

protected func Hit(x, y)
{
	StonyObjectHit(x,y);
	return true;
}

public func Constructed()
{
	inherited(...);
	
	SetSolidMask(0,0, build_grid_x, build_grid_y);
	
	var other = FindObject(Find_Or(Find_OnLine(-tile_size_x/2-1, 0, tile_size_x/2+1, 0), Find_OnLine(0, -tile_size_y/2-1, 0, tile_size_y/2+2)),
						Find_Exclude(this), Find_Property("is_constructed"), Find_Property("IsMovingBlockBuildingTile"));
	if (other)
	{
		root = other;
		root->AddChild(this);
	}
	else
	{
		MakeRoot();
	}
}

public func Destruct()
{
	SetSolidMask();
	if (IsRoot())
		for (var obj in children)
			if (obj) obj->Destruct();
	root = nil;
	children = nil;
	SetGraphics(nil, nil, 1);
	return inherited(...);
}

public func Destroy()
{
	SetSolidMask();
	if (IsRoot())
		for (var obj in children)
			if (obj) obj->Destroy();
	
	var particles = new Particles_Glimmer()
	{
		R = PV_Random(150, 255), G = PV_Random(150, 255), B = 50
	};
	CreateParticle("StarSpark", PV_Random(-build_grid_x/2, build_grid_x/2), PV_Random(-build_grid_y/2, build_grid_y/2), PV_Random(-15, 15), PV_Random(-15, 15), PV_Random(10, 30), particles, 40);
	RemoveObject();
}

local MovingWheelEffect = new Effect
{
	Construction = func()
	{
		this.Target->SetGraphics("Wheel", this.Target->GetID(), 1, GFXOV_MODE_Base);
		this.rotation = 0;
	},
	Timer = func(int time)
	{
		this.rotation += 5;
		var fsin=Sin(this.rotation, 1000), fcos=Cos(this.rotation, 1000);
		this.Target->SetObjDrawTransform(
			+fcos, +fsin, 0,
			-fsin, +fcos, 0, 1);
	}
};

private func MakeRoot()
{
	children = [];
	root = this;
	moving_fx = CreateEffect(MovingWheelEffect, 1, 3);
}

public func AddChild(object child)
{
	if (root == this)
		PushBack(children, child);
	else root->AddChild(child);
}

local Components = {Rock = 1};
public func IsToolProduct() { return true; }
