#appendto Clonk

#include Lib_ConveyorTarget

// No pickup from Clonks.
public func HasConveyorPayload() { return nil; }
public func GetConveyorPayloads() { return []; }

public func OnInteractionMenuOpen(object menu)
{
	_inherited(menu, ...);

	if (!Contained() && IsConnectedToConveyor())
	{
		var helper = CreateObject(Conveyor_PickupHelper);
		helper->InitFor(this, menu);
	}
}
