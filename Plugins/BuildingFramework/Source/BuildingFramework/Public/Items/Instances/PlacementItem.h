#pragma once

#include "CoreMinimal.h"
#include "ItemInstance.h"
#include "PlacementItem.generated.h"

UCLASS(BlueprintType, EditInlineNew)
class BUILDINGFRAMEWORK_API UPlacementItem : public UItemInstance
{
	GENERATED_BODY()

public:


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Helpers

	UFUNCTION(BlueprintPure, Category = "Placement Framework|Placement Item")
	const UPlacementItemDefinition* GetPlacementDefinition() const;

	UFUNCTION(BlueprintPure, Category = "Placement Framework|Placement Item")
	bool HasDefinition(
		const UPlacementItemDefinition* ItemDefinition) const;
};
