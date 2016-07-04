/**
	ConveyorNetwork
	
	Manages targets in a connected network of conveyor tiles.

	@author Luchs
*/

// Root node of the conveyor network.
local root;

public func Create(object root)
{
	if (this != ConveyorNetwork) FatalError("id call only");

	var network = CreateObject(ConveyorNetwork);
	network.root = root;

	return network;
}

// Merges this network with another one (which should be connected).
public func Merge(object other)
{
	if (other == this) return false; // already the same network

	other.root->SetNetwork(this, other);

	other->RemoveObject();
	return true;
}

// Splits a network at a potentially disconnected node.
public func Split(object node)
{
	// Same network (necessary for 3/4 connection points)? Still connected?
	if (node.network != this || root->FindPath(node)) return false;

	var new_network = ConveyorNetwork->Create(node);
	node->SetNetwork(new_network, this);

	return true;
}

func SaveScenarioObject(props)
{
	// Don't save - the conveyor tiles automatically create a new one.
	return false;
}
