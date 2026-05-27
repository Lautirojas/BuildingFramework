

#include "Components/PlacementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "../../Public/Items/Definitions/PlacementItemDefinition.h"

#pragma region InitialOverrideFunctions

	UPlacementComponent::UPlacementComponent()
	{
		SetIsReplicatedByDefault(true);
		PrimaryComponentTick.bCanEverTick = true;
		PrimaryComponentTick.bStartWithTickEnabled = false;

		// Tags default for "grounds", if tag aren't added the system is gonna save the first "ground" mesh got by the trace
		GroundTags.Add(FName("Ground"));
		GroundTags.Add(FName("Floor"));
	}

	void UPlacementComponent::BeginPlay()
	{
		Super::BeginPlay();

		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			OwnerController = Cast<APlayerController>(OwnerPawn->GetController());
			bIsPlacing = false;
		}

	}

	void UPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
	{
		CancelPlacement();
		Super::EndPlay(EndPlayReason);
	}

	void UPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		if (bIsPlacing && IsValid(PreviewActor))
		{
			UpdatePreviewLocation();
		}
	}

#pragma endregion InitialOverrideFunctions

#pragma region PlacementFunctions

	void UPlacementComponent::StartPlacement(UPlacementItemDefinition* PlacementDefinition)
	{
		if (!PlacementDefinition || !PlacementDefinition->PreviewMesh || !PlacementDefinition->ActorToPlace)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlacementComponent: ActorToPlace or PreviewMesh is null"));
			return;
		}

		if (bIsPlacing)
		{
			CancelPlacement();
		}

		CurrentPlacementDefinition = PlacementDefinition;

		// Validate Type
		const bool bIsValidMeshType = CurrentPlacementDefinition->PreviewMesh->IsA<UStaticMesh>() || CurrentPlacementDefinition->PreviewMesh->IsA<USkeletalMesh>();
		if (!ensureMsgf(bIsValidMeshType, TEXT("PlacementComponent: PreviewMesh must be StaticMesh or SkeletalMesh")))
		{
			return;
		}

		CurrentPreviewRotation = FRotator::ZeroRotator;
		bIsPlacing = true;
		OverlappingBlockingActors.Empty();

		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(CurrentPlacementDefinition->PreviewMesh))
		{
			CreatePreviewActorFromStaticMesh(StaticMesh);
		}
		else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(CurrentPlacementDefinition->PreviewMesh))
		{
			CreatePreviewActorFromSkeletalMesh(SkeletalMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlacementComponent: PreviewMesh is not a StaticMesh or SkeletalMesh"));
			return;
		}

		SetComponentTickEnabled(true);
	}

	bool UPlacementComponent::ConfirmPlacement()
	{
		if (!bIsPlacing ||
			!bCanPlace ||
			!CurrentPlacementDefinition)
		{
			return false;
		}

		ServerConfirmPlacement(
			CurrentPreviewLocation,
			CurrentPreviewRotation,
			CurrentPlacementDefinition);

		return true;
	}

	void UPlacementComponent::CancelPlacement()
	{
		if (!bIsPlacing)
		{
			return;
		}

		DestroyPreviewActor();

		CurrentPlacementDefinition = nullptr;
		bIsPlacing = false;
		bCanPlace = false;
		CurrentGroundActor = nullptr;
		MeshPivotOffset = FVector::ZeroVector;
		OverlappingBlockingActors.Empty();

		SetComponentTickEnabled(false);
	}

	void UPlacementComponent::UpdateCanPlace()
	{
		// Can be placed if we don't have overlapping actors
		bCanPlace = OverlappingBlockingActors.Num() == 0;
	}

#pragma endregion PlacementFunctions

#pragma region MultiplayerFunctions

	void UPlacementComponent::ServerConfirmPlacement_Implementation(
		FVector PlacementLocation,
		FRotator PlacementRotation,
		UPlacementItemDefinition* PlacementDefinition)
	{
		if (!PlacementDefinition ||
			!PlacementDefinition->ActorToPlace)
		{
			return;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = World->SpawnActor<AActor>(
			PlacementDefinition->ActorToPlace,
			PlacementLocation,
			PlacementRotation,
			SpawnParams
		);

		if (SpawnedActor)
		{
			SpawnedActor->SetReplicates(true);

			OnObjectPlaced.Broadcast(SpawnedActor);
		}
	}

#pragma endregion MultiplayerFunctions

#pragma region PreviewFunctions

	void UPlacementComponent::CreatePreviewActorFromStaticMesh(UStaticMesh* Mesh)
	{
		UWorld* World = GetWorld();
		if (!World || !Mesh)
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PreviewActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (PreviewActor)
		{
			UStaticMeshComponent* StaticMeshComp = NewObject<UStaticMeshComponent>(PreviewActor);
			StaticMeshComp->SetStaticMesh(Mesh);

			StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Overlap);
			StaticMeshComp->SetGenerateOverlapEvents(true);
			StaticMeshComp->SetCastShadow(false);
			StaticMeshComp->RegisterComponent();

			PreviewActor->SetRootComponent(StaticMeshComp);
			PreviewMeshComponent = StaticMeshComp;

			// Offset
			FBox LocalBox = Mesh->GetBoundingBox();
			MeshPivotOffset = FVector(-LocalBox.GetCenter().X, -LocalBox.GetCenter().Y, -LocalBox.Min.Z);

			SetupPreviewMaterial();
			BindOverlapEvents();
		}
	}

	void UPlacementComponent::CreatePreviewActorFromSkeletalMesh(USkeletalMesh* Mesh)
	{
		UWorld* World = GetWorld();
		if (!World || !Mesh)
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PreviewActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (PreviewActor)
		{
			USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(PreviewActor);
			SkelMeshComp->SetSkeletalMesh(Mesh);

			SkelMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SkelMeshComp->SetCollisionResponseToAllChannels(ECR_Overlap);
			SkelMeshComp->SetGenerateOverlapEvents(true);
			SkelMeshComp->SetCastShadow(false);
			SkelMeshComp->RegisterComponent();

			PreviewActor->SetRootComponent(SkelMeshComp);
			PreviewMeshComponent = SkelMeshComp;

			// Offset using bounds of skeletal mesh
			FBoxSphereBounds MeshBounds = Mesh->GetBounds();
			MeshPivotOffset = FVector(-MeshBounds.Origin.X, -MeshBounds.Origin.Y, MeshBounds.BoxExtent.Z - MeshBounds.Origin.Z);

			SetupPreviewMaterial();
			BindOverlapEvents();
		}
	}

	void UPlacementComponent::DestroyPreviewActor()
	{
		if (PreviewMeshComponent)
		{
			PreviewMeshComponent->OnComponentBeginOverlap.RemoveAll(this);
			PreviewMeshComponent->OnComponentEndOverlap.RemoveAll(this);
		}

		if (IsValid(PreviewActor))
		{
			PreviewActor->Destroy();
			PreviewActor = nullptr;
			PreviewMeshComponent = nullptr;
		}
		PreviewMaterialInstance = nullptr;
	}

	void UPlacementComponent::SetupPreviewMaterial()
	{
		if (PreviewMaterialBase)
		{
			PreviewMaterialInstance = UMaterialInstanceDynamic::Create(PreviewMaterialBase, this);
		}
		else
		{
			UMaterial* DefaultMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial"));
			if (DefaultMaterial)
			{
				PreviewMaterialInstance = UMaterialInstanceDynamic::Create(DefaultMaterial, this);
			}
		}

		if (PreviewMaterialInstance && PreviewMeshComponent)
		{
			int32 NumMaterials = PreviewMeshComponent->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				PreviewMeshComponent->SetMaterial(i, PreviewMaterialInstance);
			}
		}

		UpdatePreviewMaterial();
	}

	void UPlacementComponent::UpdatePreviewLocation()
	{
		if (!IsValid(PreviewActor) || !IsValid(OwnerController))
		{
			return;
		}

		FHitResult HitResult;
		bool bHitGround = TraceToGround(HitResult);

		if (bHitGround)
		{
			CurrentGroundActor = HitResult.GetActor();
			CurrentImpactPoint = HitResult.ImpactPoint;

			// Rotate mesh offset
			FVector RotatedOffset =
				CurrentPreviewRotation.RotateVector(MeshPivotOffset);

			// Base placement location
			CurrentPreviewLocation =
				CurrentImpactPoint + RotatedOffset;

			// Apply grid snapping
			CurrentPreviewLocation =
				SnapLocationToGrid(CurrentPreviewLocation);

			float DistanceToOwner =
				FVector::Dist(
					GetOwner()->GetActorLocation(),
					CurrentPreviewLocation
				);

			PreviewActor->SetActorLocationAndRotation(
				CurrentPreviewLocation,
				CurrentPreviewRotation
			);

			PreviewActor->SetActorHiddenInGame(false);

			if (DistanceToOwner > MaxPlacementDistance ||
				!IsValidSurface(HitResult))
			{
				bCanPlace = false;
			}
			else
			{
				CheckCurrentOverlaps();
				UpdateCanPlace();
			}
		}
		else
		{
			bCanPlace = false;
			CurrentGroundActor = nullptr;

			PreviewActor->SetActorHiddenInGame(true);
		}

		UpdatePreviewMaterial();
	}

	void UPlacementComponent::RotatePreview(float YawDelta)
	{
		if (bIsPlacing)
		{
			CurrentPreviewRotation.Yaw += YawDelta;
			CurrentPreviewRotation.Yaw = FMath::Fmod(CurrentPreviewRotation.Yaw, 360.0f);
		}
	}

	void UPlacementComponent::RotateLeft()
	{
		if (!bIsPlacing) return;

		CurrentPreviewRotation.Yaw -= CurrentPlacementDefinition->RotationStep;
		CurrentPreviewRotation.Yaw = FMath::Fmod(CurrentPreviewRotation.Yaw, 360.0f);
	}

	void UPlacementComponent::RotateRight()
	{
		if (!bIsPlacing) return;

		CurrentPreviewRotation.Yaw += CurrentPlacementDefinition->RotationStep;
		CurrentPreviewRotation.Yaw = FMath::Fmod(CurrentPreviewRotation.Yaw, 360.0f);
	}

	void UPlacementComponent::UpdatePreviewMaterial()
	{
		if (!PreviewMaterialInstance)
		{
			return;
		}

		const FLinearColor ColorToUse =
			bCanPlace ? ValidPlacementColor : InvalidPlacementColor;

		PreviewMaterialInstance->SetVectorParameterValue(
			PreviewColorParameter,
			ColorToUse
		);
	}

	bool UPlacementComponent::TraceToGround(FHitResult& OutHit)
	{
		if (!IsValid(OwnerController))
		{
			return false;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			return false;
		}

		FVector WorldLocation;
		FVector WorldDirection;

		switch (TraceMode)
		{
		case EPlacementTraceMode::MousePosition:
		{
			if (!OwnerController->DeprojectMousePositionToWorld(
				WorldLocation,
				WorldDirection))
			{
				return false;
			}
			break;
		}

		case EPlacementTraceMode::ScreenCenter:
		{
			int32 ViewportSizeX, ViewportSizeY;
			OwnerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

			FVector2D ScreenCenter(
				ViewportSizeX * 0.5f,
				ViewportSizeY * 0.5f
			);

			if (!UGameplayStatics::DeprojectScreenToWorld(
				OwnerController,
				ScreenCenter,
				WorldLocation,
				WorldDirection))
			{
				return false;
			}
			break;
		}

		default:
			return false;
		}

		FVector TraceStart = WorldLocation;
		FVector TraceEnd = WorldLocation + (WorldDirection * GroundTraceDistance);

#if !UE_BUILD_SHIPPING
		if (bDrawDebugTrace)
		{
			DrawDebugLine(World, TraceStart, TraceEnd, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}
#endif

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(PreviewActor);
		QueryParams.bTraceComplex = true;

		bool bHit = World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			TraceEnd,
			GroundChannel,
			QueryParams
		);

#if !UE_BUILD_SHIPPING
		if (bHit && bDrawDebugTrace)
		{
			DrawDebugSphere(World, OutHit.ImpactPoint, 15.0f, 8, FColor::Magenta, false, 0.0f);
		}
#endif

		return bHit;
	}

	bool UPlacementComponent::IsValidSurface(const FHitResult& HitResult) const
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor)
		{
			return false;
		}

		// Check tag if required
		if (CurrentPlacementDefinition->bRequireGroundTag)
		{
			bool bHasGroundTag = false;
			for (const FName& Tag : GroundTags)
			{
				if (HitActor->ActorHasTag(Tag))
				{
					bHasGroundTag = true;
					break;
				}
			}
			if (!bHasGroundTag)
			{
				return false;
			}
		}

		// Check surface angle
		// ImpactNormal.Z = 1 means perfectly flat (pointing up)
		// ImpactNormal.Z = 0 means vertical wall
		float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(HitResult.ImpactNormal.Z));
		if (SurfaceAngle > CurrentPlacementDefinition->MaxSurfaceAngle)
		{
			return false;
		}

		return true;
	}

#pragma endregion PreviewFunctions

#pragma region GridFunctions

	FVector UPlacementComponent::SnapLocationToGrid(const FVector& Location) const
	{
		// Grid disabled?
		if (!CurrentPlacementDefinition->bUseGridPlacement)
		{
			return Location;
		}

		if (CurrentPlacementDefinition->GridSize <= 0.0f)
		{
			return Location;
		}

		FVector SnappedLocation = Location;

		// Snap X/Y to grid
		SnappedLocation.X =
			FMath::RoundToFloat(Location.X / CurrentPlacementDefinition->GridSize) * CurrentPlacementDefinition->GridSize;

		SnappedLocation.Y =
			FMath::RoundToFloat(Location.Y / CurrentPlacementDefinition->GridSize) * CurrentPlacementDefinition->GridSize;

		// Keep original Z
		// and usually we still want surface height
		SnappedLocation.Z = Location.Z;

		return SnappedLocation;
	}

#pragma endregion GridFunctions

#pragma region OverlapFunctions

	void UPlacementComponent::OnPreviewBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
	{
		if (!OtherActor || OtherActor == GetOwner() || OtherActor == PreviewActor)
		{
			return;
		}

		// Ignore current ground
		if (OtherActor == CurrentGroundActor)
		{
			return;
		}

		// Ignore actors with ground/floor tag
		for (const FName& Tag : GroundTags)
		{
			if (OtherActor->ActorHasTag(Tag))
			{
				return;
			}
		}

		// Is an obstacle - add to the list
		if (!OverlappingBlockingActors.Contains(OtherActor))
		{
			OverlappingBlockingActors.Add(OtherActor);
			UE_LOG(LogTemp, Log, TEXT("PlacementComponent: Overlap BEGIN with %s"), *OtherActor->GetName());
		}

		UpdateCanPlace();
		UpdatePreviewMaterial();
	}

	void UPlacementComponent::OnPreviewEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
	{
		if (!OtherActor)
		{
			return;
		}

		if (OverlappingBlockingActors.Contains(OtherActor))
		{
			OverlappingBlockingActors.Remove(OtherActor);
			UE_LOG(LogTemp, Log, TEXT("PlacementComponent: Overlap END with %s"), *OtherActor->GetName());
		}

		UpdateCanPlace();
		UpdatePreviewMaterial();
	}

	void UPlacementComponent::BindOverlapEvents()
	{
		if (PreviewMeshComponent)
		{
			PreviewMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &UPlacementComponent::OnPreviewBeginOverlap);
			PreviewMeshComponent->OnComponentEndOverlap.AddDynamic(this, &UPlacementComponent::OnPreviewEndOverlap);
		}
	}

	void UPlacementComponent::CheckCurrentOverlaps()
	{
		if (!PreviewMeshComponent)
		{
			return;
		}

		// Clean list and reconstruct current overlaps
		OverlappingBlockingActors.Empty();

		TArray<AActor*> OverlappingActors;
		PreviewActor->GetOverlappingActors(OverlappingActors);

		for (AActor* OtherActor : OverlappingActors)
		{
			if (!OtherActor || OtherActor == GetOwner() || OtherActor == PreviewActor)
			{
				continue;
			}

			// Ignore current ground
			if (OtherActor == CurrentGroundActor)
			{
				continue;
			}

			// Ignore actors with ground/floor tag
			bool bIsGround = false;
			for (const FName& Tag : GroundTags)
			{
				if (OtherActor->ActorHasTag(Tag))
				{
					bIsGround = true;
					break;
				}
			}
			if (bIsGround)
			{
				continue;
			}

			// Is an obstacle
			OverlappingBlockingActors.Add(OtherActor);
		}
	}

#pragma endregion OverlapFunctions