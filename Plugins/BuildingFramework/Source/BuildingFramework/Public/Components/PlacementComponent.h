

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlacementComponent.generated.h"

// Forward Declarations
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPlacementItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectPlaced, AActor*, PlacedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlacementCancelled);


UENUM(BlueprintType)
enum class EPlacementTraceMode : uint8
{
	ScreenCenter UMETA(DisplayName = "Screen Center"),
	MousePosition UMETA(DisplayName = "Mouse Position")
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUILDINGFRAMEWORK_API UPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlacementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

#pragma region PlacementFunctions

	// Placement functions

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void StartPlacement(UPlacementItemDefinition* PlacementDefinition);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	bool ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	bool IsPlacing() const { return bIsPlacing; }

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void RotateLeft();
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void RotateRight();

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnObjectPlaced OnObjectPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FOnPlacementCancelled OnPlacementCancelled;

#pragma endregion PlacementFunctions


#pragma region MultiplayerFunctions

	UFUNCTION(Server, Reliable)
	void ServerConfirmPlacement(
		FVector PlacementLocation,
		FRotator PlacementRotation,
		UPlacementItemDefinition* PlacementDefinition);

#pragma endregion MultiplayerFunctions

protected:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

#pragma region InternalHelpers

	// Internal helper functions
	void UpdatePreviewLocation();
	void RotatePreview(float YawDelta);
	bool TraceToGround(FHitResult& OutHit);
	void UpdatePreviewMaterial();
	void CheckCurrentOverlaps();
	bool IsValidSurface(const FHitResult& HitResult) const;

	// Preview Actor functions
	void CreatePreviewActor(UStaticMesh* Mesh);
	void CreatePreviewActorFromStaticMesh(UStaticMesh* Mesh);
	void CreatePreviewActorFromSkeletalMesh(USkeletalMesh* Mesh);
	void DestroyPreviewActor();
	void SetupPreviewMaterial();

	// Grid Snapping
	FVector SnapLocationToGrid(const FVector& Location) const;

#pragma endregion InternalHelpers

#pragma region OverlapCallbacks

	// Overlap Callbacks
	UFUNCTION()
	void OnPreviewBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPreviewEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void BindOverlapEvents();

	void UpdateCanPlace();

#pragma endregion OverlapCallbacks

#pragma region DebugConfig

	// Configuration variables

	UPROPERTY(EditAnywhere, Category = "Placement|Config")
	float MaxPlacementDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Placement|Config")
	float GroundTraceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Placement|Config")
	EPlacementTraceMode TraceMode = EPlacementTraceMode::ScreenCenter;

	UPROPERTY(EditAnywhere, Category = "Placement|Config")
	TEnumAsByte<ECollisionChannel> GroundChannel = ECC_Visibility;

	// Whitelist tags
	UPROPERTY(EditAnywhere, Category = "Placement|Config")
	TArray<FName> GroundTags;

	// Preview mesh colors

	UPROPERTY(EditAnywhere, Category = "Placement|Materials")
	FName PreviewColorParameter = TEXT("EmissiveColor");

	UPROPERTY(EditAnywhere, Category = "Placement|Materials")
	FLinearColor ValidPlacementColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Placement|Materials")
	FLinearColor InvalidPlacementColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Placement|Materials")
	UMaterialInterface* PreviewMaterialBase;

	#if WITH_EDITORONLY_DATA
		// Debug
		UPROPERTY(EditAnywhere, Category = "Placement|Debug")
		bool bDrawDebugTrace = false;
	#endif

#pragma endregion DebugConfig

#pragma region InternalVariables

	UPROPERTY()
	TObjectPtr<UPlacementItemDefinition> CurrentPlacementDefinition;

	UPROPERTY()
	bool bIsPlacing = false;

	UPROPERTY()
	bool bCanPlace = false;

	UPROPERTY()
	AActor* PreviewActor;

	UPROPERTY()
	UMeshComponent* PreviewMeshComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterialInstance;

	UPROPERTY()
	FRotator CurrentPreviewRotation;

	UPROPERTY()
	FVector CurrentPreviewLocation;

	UPROPERTY()
	APlayerController* OwnerController;

	UPROPERTY()
	AActor* CurrentGroundActor;

	// Current Overlapping Blocking actors which we are collision with
	UPROPERTY()
	TArray<AActor*> OverlappingBlockingActors;

	// Offset calculated to center mesh and keep it at ground
	UPROPERTY()
	FVector MeshPivotOffset;

	UPROPERTY()
	FVector CurrentImpactPoint;

#pragma endregion InternalVariables
		
};
