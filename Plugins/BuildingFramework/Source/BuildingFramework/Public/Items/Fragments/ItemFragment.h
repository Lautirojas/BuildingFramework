
#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class BUILDINGFRAMEWORK_API UItemFragment : public UObject
{
	GENERATED_BODY()

public:

	virtual void OnItemAcquired(AActor* Buyer) const {} // Virtual function that can be overridden by child classes to define specific behavior when an item is acquired, takes the buyer as a parameter to allow for interactions with the buyer's inventory or other systems
};