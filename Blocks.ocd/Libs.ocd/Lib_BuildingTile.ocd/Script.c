/**
	Lib_BuildingElement
	

	@author KKenny
*/

#include Lib_BuildingElement
#include Library_Stackable

func MaxStackCount() { return 48; }
func InitialStackCount() { return 4; }

local Name = "$Name$";
local Description = "$Description$";

local tile_size_x = 10;
local tile_size_y = 10;

func IsBuildingTile() { return true; }
func CanBeHitByPickaxe() { if (GetCategory() == C4D_StaticBack) return true; return false; }

func Construction()
{
	SetClrModulation(RGBa(180, 180, 180, 255));
	return _inherited();
}

func Constructed()
{
	SetCategory(C4D_StaticBack);
	SetClrModulation(RGBa(255,255,255,255));
	this.Collectible = false;
	return _inherited();
}

func Destruct()
{
	SetCategory(C4D_Object);
	SetClrModulation(RGBa(180, 180, 180, 255));
	this.Collectible = true;
	return _inherited();
}

func OnHitByPickaxe()
{
	Destruct();
}

func BuildingCondition(bool crucial)
{
	return _inherited();
}

func PreviewBuildingCondition()
{
	this.already_found = true;
	var objects = [this];
	var flag = false;
	
	for (var i = 0; i < GetLength(objects); ++i)
	{
		var current = objects[i];
		if (!current) continue;
		
		var cneigh = current->GetNeighbourPreviews(true);
		
		if (GetLength(cneigh)==0)
		{
			flag = BuildingCondition();
		}
		
		for (var neighbour in cneigh)
		{
			if (neighbour.preview_buffer != nil)
				flag = neighbour.preview_buffer;
			else
			{
				neighbour.already_found = true;
				PushBack(objects, neighbour);
				flag = neighbour->BuildingCondition();
			}
				
			if (flag)
				break;
		}
		
		if (flag)
			break;
	}
	
	for (var o in objects)
	{
		if (o) o.already_found = nil;
	}
	
	if (!BuildingCondition(true))
		flag = false;
	
	this.preview_buffer = flag;
	AddEffect("ClearBuffer", this, 1, 1, this);
	
	return flag;
}

func FxClearBufferStop()
{
	this.preview_buffer = nil;
}

func SpecialPreviewCondition()
{
	return true;
}


func AdjustSurroundingMaterial(up, down, left, right)
{
	for (var x = -tile_size_x/2; x < tile_size_x/2; x++)
	{
		var mat = nil;
		if(up)
		{
			for(var y = tile_size_y; y > tile_size_y/2-1; y--)
			{
				if (!GBackSolid(x, y) && mat == nil)
					break;
				
				if (!GBackSolid(x, y) && mat != Material("HalfVehicle"))
					DrawMaterialQuad(MaterialName(mat), GetX() + x, GetY() + y, GetX() + x + 1, GetY() + y, GetX() + x + 1, GetY() + y + 1, GetX() + x, GetY() + y + 1, DMQ_Sub);
				
				mat = GetMaterial(x,y);
			}
		}
		
		if(down)
		{
			mat = nil;
			for(var y = -tile_size_y; y < -tile_size_y/2; y++)
			{
				if (!GBackSolid(x, y) && mat == nil)
					break;
				
				if (!GBackSolid(x, y) && mat != Material("HalfVehicle"))
					DrawMaterialQuad(MaterialName(mat), GetX() + x, GetY() + y, GetX() + x + 1, GetY() + y, GetX() + x + 1, GetY() + y + 1, GetX() + x, GetY() + y + 1, DMQ_Sub);
				
				mat = GetMaterial(x,y);
			}
		}
	}
	
	
	for (var y = -tile_size_y/2; y < tile_size_y/2; y++)
	{
		var mat = nil;
		if(left)
		{
			for(var x = -tile_size_x; x < -tile_size_x/2+1; x++)
			{
				if (!GBackSolid(x, y) && mat == nil)
					break;
				
				if (!GBackSolid(x, y) && mat != Material("HalfVehicle"))
					DrawMaterialQuad(MaterialName(mat), GetX() + x, GetY() + y, GetX() + x + 1, GetY() + y, GetX() + x + 1, GetY() + y + 1, GetX() + x, GetY() + y + 1, DMQ_Sub);
				
				mat = GetMaterial(x,y);
			}
		}
		
		if(right)
		{
			mat = nil;
			for(var x = tile_size_x; x > tile_size_x/2-1; x--)
			{
				if (!GBackSolid(x, y) && mat == nil)
					break;
				
				if (!GBackSolid(x, y) && mat != Material("HalfVehicle"))
					DrawMaterialQuad(MaterialName(mat), GetX() + x, GetY() + y, GetX() + x + 1, GetY() + y, GetX() + x + 1, GetY() + y + 1, GetX() + x, GetY() + y + 1, DMQ_Sub);
				
				mat = GetMaterial(x,y);
			}
		}
	}
}

private func GetNeighbourPreviews(bool ignore_cycles)
{
	var blocks = [];
	var x_pos = [-1, +1, 0, 0];
	var y_pos = [0, 0, -1, 1];
	for (var i = 0; i < 4; ++i)
	{
		var block = FindObject(Find_AtPoint(x_pos[i] * build_grid_x, y_pos[i] * build_grid_y), Find_ID(GetID()), Find_Controller(GetController()), Find_Func("IsPreview"), Find_Category(C4D_StaticBack));
		if (!block) continue;
		if (block.already_found && ignore_cycles) continue;
		PushBack(blocks, block);
	}
	return blocks;
}