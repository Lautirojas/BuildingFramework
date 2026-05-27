

#pragma once

#include "CoreMinimal.h"
#include "Items/Definitions/ItemDefinition.h"
#include "PlacementItemDefinition.generated.h"

UCLASS(BlueprintType)
class BUILDINGFRAMEWORK_API UPlacementItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:

	UPlacementItemDefinition();

	// Actor spawned when placed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement")
	TSubclassOf<AActor> ActorToPlace;

	// Preview Mesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement",
		meta = (AllowedClasses = "/Script/Engine.StaticMesh,/Script/Engine.SkeletalMesh"))
	TObjectPtr<UStreamableRenderAsset> PreviewMesh;

	// Grid
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement")
	bool bUseGridPlacement = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement",
		meta = (ClampMin = "1.0"))
	float GridSize = 100.0f;

	// Rotation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement")
	float RotationStep = 45.0f;

	// Surface Validation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement")
	bool bRequireGroundTag = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Placement Framework|Placement",
		meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSurfaceAngle = 45.0f;
};
