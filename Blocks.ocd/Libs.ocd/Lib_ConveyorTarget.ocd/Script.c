/**
	Lib_ConveyorTarget
	
	Manages the connection to the conveyor network. A target can only be
	connected to one network at once.

	@author Luchs
*/

local conveyor_tile, conveyor_network;

local IsConveyorTarget = true;
public func IsConnectedToConveyor() { return conveyor_network != nil; }

// Override me!
public func AcceptConveyorPayload(object payload) { return false; }

private func FindConveyor(int range)
{
	return FindObject(Find_ID(Tile_Conveyor), Find_OnLine(0, 0, 0, -range));
}

// Discover and updates a connected conveyor network.
protected func DiscoverConveyor()
{
	// TODO: Proper constant for the collection range
	conveyor_tile = FindConveyor(40);
	var network = conveyor_tile && conveyor_tile.network;
	if (network != conveyor_network)
	{
		if (conveyor_network)
			conveyor_network->RemoveTarget(this);
		if (network)
			network->AddTarget(this);
		conveyor_network = network;
	}
}

static const FxConveyorTarget = new Effect
{
	Timer = func()
	{
		Target->DiscoverConveyor();
	},

	Destruction = func()
	{
		if (Target.conveyor_network)
			Target.conveyor_network->RemoveTarget(Target);
	},
};

func Initialize()
{
	CreateEffect(FxConveyorTarget, 1, 30);
	return _inherited(...);
}
