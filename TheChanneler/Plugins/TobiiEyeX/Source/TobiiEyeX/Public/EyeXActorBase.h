// Copyright 2014 Tobii Technology AB. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "IEyeXPlugin.h"
#include "EyeXActorBase.generated.h"

/** 
 * The EyeXActorBase class is a base class for actors which can be interacted with using eye-gaze.
 * The GotGazeFocus event is triggered when the user is looking at the object
 * and LostGazeFocus is triggered when the user stops looking. Additionally
 * the status of the actor can be checked using the HasGazeFocus function.
 *
 * The events can be overriden when using C++ or called in Blueprint. In order
 * to use this class the scene needs to contain an EyeXPlayerController which
 * determines which actor has the attention of the user.
 */
UCLASS()
class TOBIIEYEX_API AEyeXActorBase : public AActor
{
	GENERATED_UCLASS_BODY()
	friend class AEyeXPlayerController;

public:

	/** The minimum distance at which the actor can be activated by gaze. Set to 0 to ignore distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float MinDistance;

	/** The maximum distance at which the actor can be activated by gaze. Set to 0 to ignore distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float MaxDistance;

	/** Set to true to ignore height when calculating distances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	bool bIgnoreHeightInDistanceCalculation;

	/** Set to true to visualize the minimum and maximum distance. Disabled for shipping builds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	bool bVisualizeDistance;

	/** The time it takes until this actors behavior is activated once the gaze has fallen upon it. Using some dwell time
	can be required in order to ignore false positives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float DwellTimeActivate;

	/** The time it takes until this actors behavior is deactivated once the gaze has stopped falling upon it. Using some dwell time
	can be required in order to ignore false positives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	float DwellTimeDeactivate;

	/** The customization for Channeler, if the flag is true, the module will reset the gaze timer immediately when the ray doesn't hit the actor.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeX")
	bool bResetGazeTimeImmediately;

public:

	/**
	 * Fires when this actor has received the focus of the users gaze. Requires no super call when overriding since it doesn't
	 * do anything.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "EyeX")
	void GotGazeFocusBP();

	/**
	 * Fires when this actor has lost the focus of the users gaze. Requires no super call when overriding since it doesn't
	 * do anything.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "EyeX")
	void LostGazeFocusBP();

	/**
	 * Check to see if this actor is focused by the users gaze.
	 * @return true if this actor is focused by the users gaze.
	 */
	UFUNCTION(BlueprintCallable, Category = "EyeX")
	bool HasGazeFocus() const;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void LostGazeFocus();
	virtual	void GotGazeFocus();

private:

	void VisualizeDistance() const;
	void DrawShape(float Distance, FColor Color) const;
	bool IsWithinDistance() const;
	void UpdateDwellTime(float DeltaSeconds);
	bool ClampTowardsTarget(float& CurrentTime, const float DeltaSeconds, const float DwellTime);

private:

	bool bTraceHit;
	bool bHasFocus;
	float CurrentTimeIn;
	float CurrentTimeOut;

};
