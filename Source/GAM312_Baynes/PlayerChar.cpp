// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// Constructor
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Creates a camera component to PlayerChar and gives it a name
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));
	//Attaches the camera to the head socket on the character mesh. Head socket is already labeled on the default mannequin
	PlayerCamComp->SetupAttachment(GetMesh(), "head");
	//Sets the camera to use the pawn's control rotation, so it moves when the player looks around
	PlayerCamComp->bUsePawnControlRotation = true;


	//Establish resource arrays
	ResourcesArray.SetNum(3); //Builds the array
	//Adds resource names to the array
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));

	//Establish Building Array
	BuildingArray.SetNum(3);
	


}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();
	//Timer to decrease player stats over time
	FTimerHandle StatTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);
	
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isBuilding)
	{
		if (spawnedPart)
		{
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}

}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//Movement setup
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::rotateBuilding);

}

void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}


//Health decreases when the player takes damage from various sources (e.g., enemies, environmental hazards, etc.)
void APlayerChar::SetHealth(float amount)
{
	if (Health + amount <= 100.0f)
	{
		Health = Health + amount;
	}
	else if (Health + amount <= 100.0f)
	{
		Health = 100.0f;
		//Handle player death here (e.g., respawn, game over screen, etc.)
	}

}


//Hunger decreases over time and when the player performs certain actions (e.g., running, jumping, etc.) typically. This will be expanded upon later
void APlayerChar::SetHunger(float amount)
{
	if (Hunger + amount < 100.0f)
	{
		Hunger = Hunger + amount;
	}
	else if (Hunger + amount >= 100.0f)
	{
		Hunger = 100.0f;
		//Handle player starvation here (e.g., health decrease over time, etc.)
	}

}

//Stamina works inversely to Hunger. It decreases when the player is active and increases when they are idle
void APlayerChar::SetStamina(float amount)
{
	if (Stamina + amount <= 100.0f)
	{
		Stamina = Stamina + amount;
	}
	else if (Stamina + amount >= 100.0f)
	{
		Stamina = 100.0f;
		//Maxes out stamina to 100
	}

}


//Decrease stats function. Uses the timer from BeginPlay to decrease Stamina and Hunger over time
void APlayerChar::DecreaseStats()
{
	if (Hunger > 0.0f)
	{
		SetHunger(-1.0f);
	}
	
	SetStamina(10.0f);

	if (Hunger <= 0.0f)
	{
		SetHealth(-3.0f);
	}
}

//Adds reources gathered to coresponding resources array index
void APlayerChar::GiveResource(float amount, FString resourceType)
{
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}
	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}
	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}


void APlayerChar::FindObject()
{
	FHitResult HitResult;
	//Establishes FindObject StartLocation function which is the world location and direction of where a player hits the button
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	//Direction is grabbing the forward vector(exact angle the player is facing) from the camera component and extends the vector by 800 units.
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	//EndLocation is StartLocation plus the 800 units in the direction from the previous called bit.
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);//Don't interact with self/PlayerChar
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;//Returns face normals


	//Disable Trace if building is active
	if (!isBuilding)
	{
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))//Visibility is the trace channel being used. Choices are camera and visibilty channels
		{
			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

			if (Stamina > 5.0f)
			{
				if (HitResource)
				{
					FString HitName = HitResource->resourceName;
					int resourceValue = HitResource->resourceAmount;

					HitResource->totalResource = HitResource->totalResource - resourceValue;

					if (HitResource->totalResource > resourceValue)
					{
						GiveResource(resourceValue, HitName);

						check(GEngine != nullptr);
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Collected"));
						//Calls GameplayStatics library function with the spawn decal bit. Gets the world location, grabs the hitDecal function called in the header, FVector is setting the size of the decal. HitResult is getting the location with the rortator setting -90 to x so it faces the player. 2.0f is the lifespan.
						UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

						SetStamina(-5.0f);
					}
					else
					{
						//Destroys HitResult if no resources are left in the object
						HitResource->Destroy();
						check(GEngine != nullptr);
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Depleted"));
					}
				}
			}
		}
	}

	else
	{
		isBuilding = false; 

	}
	
}

//Update Resources function. First determines if you have enough resources, then subtracts the required amount based on the variables ResourcesArray location.
void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{//Check if player (woodAmount) has enough wood, then if YES, continue to check if player has enough stone. woodAmount and stoneAmount are player related variables. ResourcesArray is the array for resource req's
	if (woodAmount <= ResourcesArray[0])
	{
		if (stoneAmount <= ResourcesArray[1])
		{
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;
			//If resources check is fine, and resources are subtracted, it adds 1 to the array's amount
			if (buildingObject == "Wall")
			{
				BuildingArray[0] = BuildingArray[0] + 1;
			}

			if (buildingObject == "Floor")
			{
				BuildingArray[1] = BuildingArray[1] + 1;
			}

			if (buildingObject == "Ceiling")
			{
				BuildingArray[2] = BuildingArray[2] + 1;
			}
		}
	}
}


//SpawnBuilding. Spawns the building part... This function is called when the player hits the build button.
void APlayerChar::spawnBuilding(int buildingID, bool& isSuccess)
{
	if (!isBuilding)
	{
		if (BuildingArray[buildingID] >= 1)//If the part's button you click is greater than or equal to 1...
		{
			isBuilding = true;//Set bool to true
			FActorSpawnParameters SpawnParams;//UE struct that provides info on how an actor is spawned at runtime(Source: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Engine/FActorSpawnParameters)
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;//400 is the distance from the player the trace exists
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0);

			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;//Subtracts 1 from that part's related array stock.

			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);//Spawns the part based on the variables set above, like trace distance, in relation to player's GetWorld() function.

			isSuccess = true; //Completes validity check.
		}

		isSuccess = false;//Any other state is failure, thus completing a validity check.
	}
}

//Rotates building if player hits the button assigned in UE Action mapping(E)
void APlayerChar::rotateBuilding()
{
	if (isBuilding)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}