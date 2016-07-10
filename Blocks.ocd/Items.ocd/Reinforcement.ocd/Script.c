/* Wall Reinforcement Kit */

#include Library_Stackable

func MaxStackCount() { return 10; }

local block_from = nil;
local block_to = nil;
local preview;

public func Destruction()
{
	if (preview) preview->RemoveObject();
}

/* Item usage */

func ControlUseStart(object clonk, int x, int y)
{
	CheckPosition(clonk, x, y);
	return true;
}

func HoldingEnabled() { return true; }

func ControlUseHolding(object clonk, int x, int y)
{
	CheckPosition(clonk, x, y);
	return true;
}

public func ControlUseStop(object clonk, int x, int y)
{
	if (!block_from || !block_to)
	{
		StopPreview(clonk);
		return true;
	}
	var item = TakeObject();
	if (item)
	{
		item->LinkBlocks(block_from, block_to);
	}
	StopPreview(clonk);
	return true;
}

public func ControlUseCancel(object clonk, int x, int y)
{
	StopPreview(clonk);
	return true;
}

private func LinkBlocks(object a, object b)
{
	if (!a || !b) return;
	Exit();
	var fx = a->GetX();
	var fy = a->GetY();
	var tx = b->GetX();
	var ty = b->GetY();
	var offset_x = 0;
	var offset_y = 0;
	
	if ((fx < tx && fy < ty) || (fx > tx && fy > ty))
	{
		offset_x = 1;
		offset_y = -1;
	}
	else
	if ((fx < tx && fy > ty) || (fx > tx && fy < ty))
	{
		offset_x = -1;
		offset_y = -1;
	}
	SetPosition((fx + tx) / 2 + offset_x, (fy + ty) / 2 + offset_y);
	SetSolidMask(0, 0, 9, 15, 0, 0);
	var angle = Angle(fx, fy, tx, ty);
	if (angle > 180)
		angle -= 180;
	SetR(angle);
	SetCategory(C4D_StaticBack);
	this.Plane = b.Plane + 1;
	this.Collectible = false;
}

private func CheckPosition(object clonk, int x, int y)
{
	x = AbsX(x + clonk->GetX());
	y = AbsY(y + clonk->GetY());

	if (!block_from)
	{
		var block = FindObject(Find_AtPoint(x, y), Find_Property("is_constructed"), Find_Func("IsSolidBuildingTile"));
		if (!block) return;
		block_from = block;

		clonk->Sound("Objects::WallKit::Click");
		preview = CreateObject(Dummy, AbsX(block->GetX()), AbsY(block->GetY()), clonk->GetController());
		preview.Visibility = VIS_Owner;
		preview.Plane = block_from.Plane + 1;
		preview->SetGraphics(nil, GetID());
		return;
	}
	
	var fx = block_from->GetX();
	var fy = block_from->GetY();
	var x_rel = (x + GetX()) - fx;
	var y_rel = (y + GetY()) - fy;
	var angle = Angle(0, 0, x_rel, y_rel);
	var distance = Min(3 * block_from.tile_size_x / 2, Distance(0, 0, x_rel, y_rel));
	
	x = fx + Sin(angle, distance) - GetX();
	y = fy - Cos(angle, distance) - GetY();
	CreateParticle("SphereSpark", x, y, 0, 0, 20, Particles_Glimmer());
	var block = FindObject(Find_AtPoint(x, y), Find_Property("is_constructed"), Find_Func("IsSolidBuildingTile"), Find_Exclude(block_from));
	if (block && block != block_to)
	{
		clonk->Sound("Objects::WallKit::Click", {pitch = 50});
		block_to = block;
		preview->SetPosition((block_to->GetX() + block_from->GetX()) / 2, (block_to->GetY() + block_from->GetY()) / 2);
		angle = Angle(block_from->GetX(), block_from->GetY(), block_to->GetX(), block_to->GetY());
		var fsin = Sin(angle, 1000);
		var fcos = Cos(angle, 1000);
		preview->SetObjDrawTransform(+fcos, +fsin, 0, -fsin, +fcos, 0);
	}
}

func StopPreview(object clonk)
{
	if (preview) preview->RemoveObject();
	block_from = nil;
	block_to = nil;
	return true;
}


/* Impact sound */

func Hit()
{
	Sound("Hits::GeneralHit?");
	return true;
}


/* Status */

local Collectible = 1;
local Name = "$Name$";
local Description = "$Description$";
local Components = {Metal = 2};