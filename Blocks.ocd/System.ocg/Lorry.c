#appendto Lorry

#include Lib_ConveyorTarget

func Initialize()
{
	Call(Lib_ConveyorTarget.Initialize);
	return _inherited(...);
}

public func AcceptConveyorPayload(object payload)
{
	// TODO: Also consider items in transit.
	return ContentsCount() < MaxContentsCount;
}
