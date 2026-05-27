
#include "Items/Instances/ItemInstance.h"
#include "Net/UnrealNetwork.h"

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UItemInstance, Definition);
}

bool UItemInstance::IsSupportedForNetworking() const
{
	return true;
}