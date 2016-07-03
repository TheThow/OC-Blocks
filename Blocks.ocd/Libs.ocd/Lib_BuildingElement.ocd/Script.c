/**
	Lib_BuildingElement
	

	@author KKenny
*/

local Name = "$Name$";
local Description = "$Description$";

local can_be_colored = true;

local build_offset_x = 0;
local build_offset_y = 0;

local build_grid_x = 10;
local build_grid_y = 10;

local starting_point_x;
local starting_point_y;

local preview_objects;
local preview_width;
local preview_height;

// Whether the element is locked in place (and NOT a preview and NOT loose).
local is_constructed = false;

static const TILE_MODE_SINGLE = 1;
static const TILE_MODE_LINE = 2;
static const TILE_MODE_VERTICAL_LINE = 3;
static const TILE_MODE_HORIZONTAL_LINE = 4;
static const TILE_MODE_RECTANGLE = 5;
local tile_mode = TILE_MODE_SINGLE;

func IsBuildingElement() { return true; }
func HoldingEnabled() { return true; }
func IsPreview() { return this.isPreview; }

public func Initialize()
{
	if(!GetEffect("Preview"))
		AddEffect("Preview", nil, 1, 5);
}

public func Construction()
{
	preview_objects = [];
}

func PreviewMode()
{
	SetCategory(C4D_StaticBack);
	SetClrModulation(RGBa(255, 255, 255, 128));
	this.Visibility = VIS_Owner;
	this.isPreview = true;
	//AddEffect("Preview", this, 1, 5, this);
}

public func ControlUseStart(object clonk, int x, int y)
{
	starting_point_x = GetRightX(clonk->GetX() + x);
	starting_point_y = GetRightY(clonk->GetY() + y);
	
	SetController(clonk->GetOwner());
	var obj = CreatePreviewObject();
	PushBack(preview_objects, obj);
	ControlUseHolding(clonk, x, y);
	
	return true;
}

public func ControlUseHolding(object clonk, int x, int y)
{
	var tx = GetRightX(clonk->GetX() + x);
	var ty = GetRightY(clonk->GetY() + y);
	
	if (tile_mode == TILE_MODE_SINGLE)
		preview_objects[0]->SetPosition(tx, ty);
	else if (tile_mode == TILE_MODE_LINE)
		DrawPreviewLine(starting_point_x, starting_point_y, clonk->GetX() + x, clonk->GetY() + y);
	else if (tile_mode == TILE_MODE_VERTICAL_LINE)
		DrawPreviewLine(starting_point_x, starting_point_y, starting_point_x, clonk->GetY() + y);
	else if (tile_mode == TILE_MODE_HORIZONTAL_LINE)
		DrawPreviewLine(starting_point_x, starting_point_y, clonk->GetX() + x, starting_point_y);
	else if (tile_mode == TILE_MODE_RECTANGLE)
		DrawPreviewRect(starting_point_x, starting_point_y, clonk->GetX() + x, clonk->GetY() + y);
	return true;
}

public func ControlUseStop(object clonk, int x, int y)
{
	while (true)
	{
		var flag = false;
		var len = GetLength(preview_objects);
		for (var i = 0; i < len; ++i)
		{
			var preview_object = preview_objects[i];
			if (!preview_object) continue;
			if (!preview_object->BuildingCondition()) continue;
			var obj = this->~TakeObject();
			
			if(!obj)
				obj = this;
			
			obj->SetPosition(preview_object->GetX(), preview_object->GetY());
			obj->Constructed();
			flag = true;
			preview_objects[i] = nil;
			preview_object->RemoveObject();
			if (obj == this)
			{
				flag = false;
				break;
			}
		}
		
		if (!flag)
			break;
	}
	
	ClearPreviewObjects();
	return true;
}

public func ControlUseCancel(object clonk, int x, int y)
{
	/*if(preview_object)
		preview_object->RemoveObject();*/
	AddEffect("RemovePreview", this, 1, 1, this);
	return true;
}

private func DrawPreviewRect(int from_x, int from_y, int to_x, int to_y)
{
	var len_x = to_x - from_x;
	var len_y = to_y - from_y;
	var xdir = 0;
	var ydir = 0;

	xdir = BoundBy(len_x, -1, 1);
	ydir = BoundBy(len_y, -1, 1);
	
	// Round up.
	var blocksX = Abs(len_x) / build_grid_x + 1;
	var blocksY = Abs(len_y) / build_grid_y + 1;
	
	blocksX = BoundBy(blocksX, -7, 7);
	blocksY = BoundBy(blocksY, -7, 7);
	
	var sizey = nil;
	var sizex = nil;
	
	if (preview_width != nil && GetLength(preview_objects))
	{
		sizey = GetLength(preview_objects)/preview_width;
		sizex = GetLength(preview_objects)/preview_height;
	}
	
	var new_previews = [];
	SetLength(new_previews, blocksX*blocksY);
	
	var x = starting_point_x;
	var y = starting_point_y;
	for (var i = 0; i < blocksX; ++i)
	{
		var block;
		if (sizex == nil || i >= sizex)
		{
			block = CreatePreviewObject();
		}
		else 
		{
			block = preview_objects[i * sizey];
			preview_objects[i * sizey] = nil;
		}
		new_previews[i * sizey] = block;
		//block = CreatePreviewObject();
		if (!block) continue;
		
		block->SetPosition(x, y);
		
		block = nil;
		y = starting_point_y + ydir * build_grid_y;
		for (var u = 1; u < blocksY; ++u)
		{
			//block = CreatePreviewObject();
			if (sizex == nil || u >= sizey)
				block = CreatePreviewObject();
			else 
			{
				block = preview_objects[i * sizey + u];
				preview_objects[i * sizey + u] = nil;
			}
			
			new_previews[i * sizey + u] = block;	
			
			if (!block) continue;
			
			block->SetPosition(x, y);
			y += ydir * build_grid_y;
		}
		y = starting_point_y;
		x += xdir * build_grid_x;
	}
	
	/*if (preview_width != nil)
	{
		var mx = blocksX;
		var my = blocksY;
		//var sizey = GetLength(preview_objects)/preview_width;
		//var sizex = GetLength(preview_objects)/preview_height;
		
		while (my < sizey)
		{
			for (var i = 0; i < sizex; i++)
			{
				if (preview_objects[my + i * sizey])
					preview_objects[my + i * sizey]->RemoveObject();
			}
			my++;
		}
		
		while (mx < sizex)
		{
			for (var i = 0; i < sizey; i++)
			{
				if (preview_objects[mx*sizey + i])
					preview_objects[mx*sizey + i]->RemoveObject();
			}
			mx++;
		}
	}*/
	ClearPreviewObjects();
	preview_width = blocksX;
	preview_height = blocksY;
	//SetLength(preview_objects, blocksX*blocksY);
	preview_objects = new_previews;
}

private func DrawPreviewLine(int from_x, int from_y, int to_x, int to_y)
{
	var len_x = to_x - from_x;
	var len_y = to_y - from_y;
	var len = Max(Abs(len_x), Abs(len_y));
	var xdir = 0;
	var ydir = 0;
	
	if (Abs(len_x) > Abs(len_y)) xdir = BoundBy(len_x, -1, 1);
	else ydir = BoundBy(len_y, -1, 1);
	
	// Round up.
	var blocks = Min(len / build_grid_x + 1, 25);
	
	var x = starting_point_x;
	var y = starting_point_y;
	for (var i = 0; i < blocks; ++i)
	{
		var block;
		if (i >= GetLength(preview_objects))
		{
			block = CreatePreviewObject();
			PushBack(preview_objects, block);
		}
		else block = preview_objects[i];
		if (!block) continue;
		
		block->SetPosition(x, y);
				
		x += xdir * build_grid_x;
		y += ydir * build_grid_y;
	}
	
	var original_blocks = blocks;
	while (blocks < GetLength(preview_objects))
	{
		if (preview_objects[blocks])
			preview_objects[blocks]->RemoveObject();
		blocks++;
	}
	SetLength(preview_objects, original_blocks);
}


private func CreatePreviewObject()
{
	var preview_object = CreateObject(GetID(), 0, 0, GetController());
	preview_object.Collectible = 0;
	preview_object->PreviewMode();
	return preview_object;
}

private func ClearPreviewObjects()
{
	for (var obj in preview_objects)
		if (obj) obj->RemoveObject();
	preview_objects = [];
	preview_width = nil;
	preview_height = nil;
}

//Workaround
func FxRemovePreviewStop()
{
	ClearPreviewObjects();
}

func Constructed()
{
	is_constructed = true;
	this.Collectible = false;
	return true;
}

func Destruct()
{
	is_constructed = false;
	return true;
}

func Destroy()
{
	is_constructed = false;
	return true;
}

func BuildingCondition()
{
	return true;
}

func PreviewBuildingCondition()
{
	return BuildingCondition();
}

func GetRightX(int x)
{
	var off = x % build_grid_x;
	x = x - off;
	
	if (off > build_grid_x/2)
		off = build_grid_x;
	else
		off = 0;
	
	return build_offset_x + x + off;
}

func GetRightY(int y)
{
	var off = y % build_grid_y;
	y = y - off;
	
	if (off > build_grid_y/2)
		off = build_grid_y;
	else
		off = 0;
	
	return build_offset_y + y + off;
}

func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	if (is_constructed) props->AddCall("Constructed", this, "Constructed");
	return true;
}
