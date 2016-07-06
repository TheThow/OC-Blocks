/**
	ConveyorNetwork
	
	Manages targets in a connected network of conveyor tiles.

	@author Luchs
*/

// Root node of the conveyor network.
local root;

// List of targets, keyed on object number for fast manipulation.
local targets; 

public func Create(object root)
{
	if (this != ConveyorNetwork) FatalError("id call only");

	var network = CreateObject(ConveyorNetwork);
	network.root = root;
	network.targets = [];

	return network;
}

// Merges this network with another one (which should be connected).
public func Merge(object other)
{
	if (other == this) return false; // already the same network

	other.root->SetNetwork(this, other);
	other->RemoveObject();

	UpdateTargets();
	return true;
}

// Splits a network at a potentially disconnected node.
public func Split(object node)
{
	// Same network (necessary for 3/4 connection points)? Still connected?
	if (node.network != this || root->FindPath(node)) return false;

	var new_network = ConveyorNetwork->Create(node);
	node->SetNetwork(new_network, this);

	UpdateTargets();

	return true;
}

public func AddTarget(object target)
{
	if (GetIndexOf(targets, target) == -1)
		PushBack(targets, target);
}

public func RemoveTarget(object target)
{
	RemoveArrayValue(targets, target, true /* unstable */);
}

// Makes each registered target re-discover its network.
private func UpdateTargets()
{
	for (var t in targets)
	{
		t->DiscoverConveyor();
	}
}

func SaveScenarioObject(props)
{
	// Don't save - the conveyor tiles automatically create a new one.
	return false;
}
