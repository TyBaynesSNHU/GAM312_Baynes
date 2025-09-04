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
	PlayerInputComponent->BindAction("FindObject", IE_Pressed, this, &APlayerChar::FindObject);

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
	if (Health + amount > 100.0f)
	{
		Health = Health + amount;
	}
	else if (Health + amount <= 0.0f)
	{
		Health = 0.0f;
		//Handle player death here (e.g., respawn, game over screen, etc.)
	}
	else
	{
		Health += amount;
	}
}


//Hunger decreases over time and when the player performs certain actions (e.g., running, jumping, etc.) typically. This will be expanded upon later
void APlayerChar::SetHunger(float amount)
{
	if (Hunger + amount < 100.0f)
	{
		Hunger = Hunger + amount;
	}
	else if (Hunger + amount <= 0.0f)
	{
		Hunger = 0.0f;
		//Handle player starvation here (e.g., health decrease over time, etc.)
	}
	else
	{
		Hunger += amount;
	}
}

//Stamina works inversely to Hunger. It decreases when the player is active and increases when they are idle
void APlayerChar::SetStamina(float amount)
{
	if (Stamina + amount <= 100.0f)
	{
		Stamina = Stamina + amount;
	}
	else if (Stamina + amount <= 0.0f)
	{
		Stamina = 0.0f;
		//Handle player exhaustion here (e.g., slow movement, inability to sprint, etc.)
	}
	else
	{
		Stamina += amount;
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


void APlayerChar::FindObject()
{
	//Line trace (raycast) to find an object in front of the player
	FHitResult Hit;
	FVector Start = PlayerCamComp->GetComponentLocation();
	FVector End = Start + (PlayerCamComp->GetForwardVector() * 500.0f);
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams))
	{
		if (Hit.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *Hit.GetActor()->GetName());
		}
	}
}
