public func SaveScenarioObject() { return false; }

local base;

local target;
local length;
local start_y;

local retract_fx;

local MovingEffect = new Effect
{
	Timer = func (int time)
	{
		var phase = time;
		this.Target->SetPosition(this.Target->GetX(), this.Target.start_y + Sin(phase, this.Target.length));
		
		if (phase >= 90)
		{
			this.Target->OnMoveEnd();
			return FX_Execute_Kill;
		}
	}
};

local RetractMarker = new Effect
{
	Start = func() { this.Target.is_being_moved_by_conveyor = true; },
	Stop = func() { this.Target.is_being_moved_by_conveyor = false; },
};

local RetractEffect = new Effect
{
	SetPayload = func(payload)
	{
		if (!payload) return;
		this.payload = payload;
		this.payload_fx = payload->CreateEffect(Tile_ConveyorArmHook.RetractMarker, 1, 0);
		payload->SetRDir();
		// The hook also needs to be able to grab stuff from containers such as lorries.
		payload->Exit();
		this.was_collectible = payload.Collectible;
		payload.Collectible = false;
	},
	
	Stop = func()
	{
		if (this.payload_fx)
			RemoveEffect(nil, nil, this.payload_fx);
		if (this.was_collectible != nil)
			this.payload.Collectible = this.was_collectible;
	},
	
	Timer = func (int time)
	{
		var this_y = this.Target->GetY();
		var target_y = this.Target.base->GetY();  
		if (this_y != target_y)
			this.Target->SetPosition(this.Target.base->GetX(), this_y + BoundBy(target_y - this_y, -2, 2));
		else
		{
			if (this.was_collectible != nil)
			{
				this.Target.Collectible = this.was_collectible;
				this.was_collectible = nil;
			}

			if (!this.payload || this.payload->Contained())
			{
				this.Target.base->RemoveObject();
				return FX_Execute_Kill;
			}
			else if (!this.is_moving)
			{
				this.Target.base->StartMoving(this.payload);
				this.is_moving = true;
			}
		}
		
		if (this.payload)
		{
			this.payload->SetPosition(this.Target->GetX(), this.Target->GetY());
			this.payload->SetSpeed(0, 0);
		}
	}
};

// Grab a free object in the landscape or deliver a grabbed object.
public func MoveTo(object target)
{
	this.target = target;
	start_y = GetY();
	length = target->GetY() - start_y;
	this.Plane = target.Plane + 1;
	CreateEffect(MovingEffect, 1, 1);
	Sound("Structures::Elevator::Start", {pitch = 200});
}

public func OnMoveEnd()
{
	Sound("Structures::Elevator::Stop", {pitch = 200});
	Sound("Hits::Materials::Metal::DullMetalHit*", {pitch = 150});
	var still_target = FindObject(Find_AtPoint(), Find_InArray([target]));
		
	if (retract_fx)
	{
		if (still_target)
			target->Collect(retract_fx.payload, true);
		base->RemoveObject();
	}
	else
	{
		retract_fx = CreateEffect(RetractEffect, 1, 1);
		retract_fx->SetPayload(still_target);
	}
}
