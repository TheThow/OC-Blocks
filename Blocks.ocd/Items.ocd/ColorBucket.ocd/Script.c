/**
	Bucket
	Transport earth from one spot to another to form the landscape.
	Replaces the old earth chunks in their behaviour.

	@author Clonkonaut
*/


// Maximum distance at which material is collected / spilled
local maxreach = 15;

local color;

local starting_point_x;
local starting_point_y;
local particle;
local dummy;

public func GetCarryMode() { return CARRY_HandBack; }
public func GetCarryBone() { return "main"; }
func HoldingEnabled() { return true; }


public func ControlUseStart(object clonk, int x, int y)
{
	SetController(clonk->GetOwner());
	dummy = CreateObject(Dummy, 0, 0, GetController());
	dummy->SetPosition(0,0);
	dummy.Visibility = VIS_Owner;
	dummy.Plane = 700;
	
	starting_point_x = clonk->GetX() + x;
	starting_point_y = clonk->GetY() + y;
	ControlUseHolding(clonk, x, y);
	
	return true;
}

public func ControlUseHolding(object clonk, int x, int y)
{
	dummy->ClearParticles();
	var tx = clonk->GetX() + x;
	var ty = clonk->GetY() + y;
	dummy->DrawParticleLine("Flash", starting_point_x, starting_point_y, tx + (tx-starting_point_x), ty + (ty - starting_point_y), 1, 0, 0, 1, particle);
	//DrawParticleLine("Flash", starting_point_x - GetX(), starting_point_y - GetY(), clonk->GetX() + x - GetX(), clonk->GetY() + y - GetY(), 1, 0, 0, 1, particle);
	return true;
}

public func Construction()
{
	SetDrawingColor([nil, RGB(255,255,255)]);
}

public func GetCarryTransform()
{
	return Trans_Mul(Trans_Rotate(-90, 0, 1, 0), Trans_Translate(3500, 0, -4000));
}

public func RejectUse(object clonk)
{
	return !clonk->HasHandAction(false, false, true);
}

public func ControlThrow(clonk, x, y)
{
	var clr = RGB(175, 0, 175);

	var menu = 
	{
		Player = clonk->GetOwner(),
		Decoration = GUI_MenuDeco,
		Margin = ["2em", "2em"],
		BackgroundColor = RGBa(0, 0, 0, 150),
		Style = GUI_FitChildren,
		
		titlebar = 
		{
			Bottom = "1.25em",
			Symbol = 0,
			text = 
			{
				Left = "1em",
				Text = Format("<c %x>%s</c>", clr, "Pick a color")
			},
			bottomline = 
			{
				Top = "100% - 0.1em",
				BackgroundColor = clr
			}
		},
		
		items = {
			Style = GUI_GridLayout | GUI_FitChildren,
			Top = "1.5em", Left = "1.5em", Bottom = "100% - 1.5em", Right = "100% - 1.5em",
		}
	};
	
	
	var clrs = [];
	
	var cnt = 100;
	var none = {
		Symbol = Icon_Cancel,
		Priority=cnt++,
		BackgroundColor = RGBa(255,255,255,0),
		OnClick = GuiAction_Call(this, "SetDrawingColor", [clonk, RGB(255,255,255)]),
		Right = "2.5em", Bottom = "2.5em", Margin = "0.025em",
	};
	PushBack(clrs, none);
	for (var i = 0; i < 255; i+=5)
	{
		var c = {
			Symbol = Dummy,
			Priority=cnt++,
			BackgroundColor = HSL(i, 255, 128),
			OnClick = GuiAction_Call(this, "SetDrawingColor", [clonk, HSL(i, 255, 128)]),
			Right = "2.5em", Bottom = "2.5em", Margin = "0.025em",
		};
		PushBack(clrs, c);
	}
	
	for (var i = 30; i <= 255; i+=15)
	{
		var c = {
			Symbol = Dummy,
			Priority=cnt++,
			BackgroundColor = RGB(i, i, i),
			OnClick = GuiAction_Call(this, "SetDrawingColor", [clonk, RGB(i, i, i)]),
			Right = "2.5em", Bottom = "2.5em", Margin = "0.025em",
		};
		PushBack(clrs, c);
	}
	
	/*var black = {
		Symbol = Dummy,
		Priority=cnt++,
		BackgroundColor = RGB(0, 0, 0),
		OnClick = GuiAction_Call(this, "SetDrawingColor", [clonk, RGB(50, 50, 50)]),
		Right = "2.5em", Bottom = "2.5em", Margin = "0.025em",
	};
	PushBack(clrs, black);
	var white = {
		Symbol = Dummy,
		Priority=cnt++,
		BackgroundColor = RGB(255, 255, 255),
		OnClick = GuiAction_Call(this, "SetDrawingColor", [clonk, RGB(255, 255, 255)]),
		Right = "2.5em", Bottom = "2.5em", Margin = "0.025em",
	};
	PushBack(clrs, white);*/
	
	for(var c in clrs)
		GuiAddSubwindow(c, menu.items);
	
	if(!clonk.color_bucket_menu_id)
	{
		var mid = GuiOpen(menu);
		clonk.color_bucket_menu_id = mid;
	}
	return true;
}

public func SetDrawingColor(array props)
{
	if(props[0])
	{
		GuiClose(props[0].color_bucket_menu_id);
		props[0].color_bucket_menu_id = nil;
	}
	
	color = props[1];
	SetClrModulation(color);
	
	var rgba = SplitRGBaValue(color);
	
	particle = {
		Size = 1,
		R = rgba.R,
		G = rgba.G,
		B = rgba.B,
		Attach=ATTACH_Front,
	};
}

public func ControlUseStop(object clonk, int x, int y)
{
	if(!color)
		return;
	
	for (var o in FindObjects(Find_OnLine(starting_point_x-GetX(), starting_point_y-GetY(), clonk->GetX() + x - GetX(), clonk->GetY() + y - GetY()), 
	Find_Or(Find_Func("IsSolidBuildingTile"), Find_Func("IsWallBuildingTile")), Find_Property("can_be_colored"), Find_Property("is_constructed")))
	{
		o->SetClrModulation(color);
	}
	
	if (dummy)
		dummy->RemoveObject();
	
	return true;
}

public func ControlUseCancel(object clonk, int x, int y)
{
	if (dummy)
		dummy->RemoveObject();
	return true;
}


protected func Hit()
{
	Sound("Hits::BucketHit?");
}

public func IsTool() { return true; }
public func IsToolProduct() { return true; }


protected func Definition(def)
{
	SetProperty("PictureTransformation", Trans_Mul(Trans_Translate(500,400,0), Trans_Rotate(-10,1,0,0), Trans_Rotate(30,0,1,0), Trans_Rotate(+25,0,0,1), Trans_Scale(1100)),def);
}

local Name = "$Name$";
local Description = "$Description$";
local Collectible = true;
local ForceFreeHands = true;
local Components = {Wood = 1, Metal = 1};
