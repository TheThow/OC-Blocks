/**
	Lib_ConveyorTarget
	
	Manages the connection to the conveyor network. A target can only be
	connected to one network at once.

	@author Luchs
*/

local conveyor_tile, conveyor_network;

local IsConveyorTarget = true;
public func IsConnectedToConveyor() { return conveyor_network != nil; }

/* Function to override */

// Returns whether an object is accepted for storage.
public func AcceptConveyorPayload(object payload) { return false; }

// Returns an object to transport with the given id.
// The default implementation looks at all contained objects.
public func HasConveyorPayload(id payload)
{
	return FindObject(Find_Container(this), Find_ID(payload));
}

/* END functions to override */

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
