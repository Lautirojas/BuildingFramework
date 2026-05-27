
#include "Items/Instances/PlacementItem.h"

#include "Net/UnrealNetwork.h"
#include "Items/Definitions/PlacementItemDefinition.h"

void UPlacementItem::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

const UPlacementItemDefinition* UPlacementItem::GetPlacementDefinition() const
{
	return Cast<UPlacementItemDefinition>(Definition);
}

bool UPlacementItem::HasDefinition(
	const UPlacementItemDefinition* ItemDefinition) const
{
	return Definition == ItemDefinition;
}