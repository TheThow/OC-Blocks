public func SaveScenarioObject() { return false; }

local home_block, target; // start and destination
local hook;
local line;

public func Construction()
{
	var particles = Particles_Dust();
	CreateParticle("Smoke", 0, 0, PV_Random(-10, 10), PV_Random(-10, 10), PV_Random(0, 20), particles, 40);
	
	hook = CreateObject(Tile_ConveyorArmHook, 0, 0, NO_OWNER);
	hook.base = this;
	
	line = CreateObject(ElevatorRope, 0, 0, NO_OWNER);
	line->SetAction("Be", hook);
	line.Plane = 9;
}

public func Destruction()
{
	if (line) line->RemoveObject();
	if (hook) hook->RemoveObject();
	
	var particles = Particles_Dust();
	CreateParticle("Smoke", 0, 0, PV_Random(-5, 5), PV_Random(-5, 5), PV_Random(0, 10), particles, 40);
}

local MoveEffect = new Effect
{
	Construction = func()
	{
		this.pos = 0;
	},

	Timer = func()
	{
		if (!this.final_target || !this.final_target.conveyor_tile) return FX_Execute_Kill;
		if (!this.current_block) return FX_Execute_Kill;
		var current_x = this.Target->GetX();
		var current_y = this.Target->GetY();
		
		if (!this.current_target)
		{
			// Move on in the pre-calculated path.
			this.current_target = this.path && this.path[this.pos++];
			if (!this.current_target || !this.current_target->IsNeighbour(this.current_block))
			{
				// Recalculate a new path to the final target.
				this.path = this.current_block->FindPath(this.final_target.conveyor_tile);
				if (this.path)
				{
					this.current_target = this.path[1];
					this.pos = 2;
				}
				else
					return FX_Execute_Kill;
			}
		}
		var target_x = this.current_target->GetX();
		var target_y = this.current_target->GetY();
		var move_x = BoundBy(target_x - current_x, -2, 2);
		var move_y = BoundBy(target_y - current_y, -2, 2);
		var new_x = this.Target->GetX() + move_x;
		var new_y = this.Target->GetY() + move_y;
		this.Target->SetPosition(new_x, new_y);
		this.Target.hook->SetPosition(new_x, new_y);
		
		if (move_x == 0 && move_y == 0)
		{
			if (this.current_target == this.final_target.conveyor_tile)
			{
				this.Target->RemoveObject();
				return FX_Execute_Kill;
			}
			this.last_target = this.current_block;
			this.current_block = this.current_target;
			this.current_target = nil;
		}
		
		return FX_OK;
	},

	Destruction = func()
	{
		Target.home_block = this.current_block;
		ScheduleCall(Target, "StartMoving", 30, nil, this.payload);
	}
};

public func StartMoving(object payload)
{
	if (!payload || !home_block) return;

	// Check the predefined target or find a new one.
	var target = this.target;
	if (!target || target.conveyor_network != home_block.network)
		target = home_block.network->DiscoverTarget(home_block, payload);
	
	if (!target)
	{
		ScheduleCall(this, "StartMoving", 30, nil, payload);
		return;
	}
	
	if (this.line)
		this.line->RemoveObject();
	
	var fx = CreateEffect(MoveEffect, 1, 1);
	fx.final_target = target;
	fx.current_block = home_block;
	fx.payload = payload;
}
