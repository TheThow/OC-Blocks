#appendto Library_Producer

#include Lib_ConveyorTarget

public func RequestObject(id item_id, int amount)
{
	if (!conveyor_network) return;

	for (var i = 0; i < amount; i++)
		conveyor_network->RequestObject(this, item_id);
}
