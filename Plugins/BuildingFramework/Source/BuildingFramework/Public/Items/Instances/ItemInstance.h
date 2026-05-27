#pragma once

#include "CoreMinimal.h"
#include "ItemInstance.generated.h"

class UItemDefinition;

UCLASS(Abstract, BlueprintType, EditInlineNew)
class BUILDINGFRAMEWORK_API UItemInstance : public UObject
{
    GENERATED_BODY()

public:

    virtual bool IsSupportedForNetworking() const override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory Framework|Item Instance")
	TObjectPtr<UItemDefinition> Definition = nullptr; // The definition of the item, using TObjectPtr for safe memory management and garbage collection

    template<typename T>
	const T* GetDefinitionAs() const // Template function to get the item definition as a specific class
    {
        return Cast<T>(Definition);
    }

    // Helpers

    // Get Item Definition
    UFUNCTION(BlueprintPure, Category = "Inventory Framework|Item Instance")
    UItemDefinition* GetItemDefinition() const
    {
        return Definition;
    }
};
