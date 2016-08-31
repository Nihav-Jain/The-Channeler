#pragma once 

#include "GameFramework/HUD.h"
#include "Private/EyeXPluginPrivatePCH.h"
#include "TheChannelerHUD.generated.h"


/**
* Class to provide functions for drawing on the HUD .
* The Mouse Emulation treated as the centre of screen and displays a crosshair in the centre of the screen.
*/
UCLASS()
class ATheChannelerHUD : public AHUD
{
	GENERATED_BODY()

public:

	/**
	* Constructor
	*/
	ATheChannelerHUD();

	/**
	* DrawHUD - Primary draw call for the HUD
	*/
	virtual void DrawHUD() override;

	/**
	* SetMousePosition - Sets the mouse position at center of the screen.
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Mouse")
		void SetMousePosition();

	/**
	* DrawSectionLines - Draws the line divider between the sections.
	* @param Sections	Number of sections to divide the circular menu.
	* @param InnerRadius	Inner radius of the lines.
	* @param OuterRadius	Outer radius of the lines.
	* @param ScreenSize	The current screen size.
	* @param Offset	The offset from 0 degrees <- experimental
	* @param lineColor The color of dividing line.
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		void DrawSectionLines(int32 Sections, float InnerRadius, float OuterRadius, FVector2D ScreenSize, float Offset = 0.0, FLinearColor LineColor = FLinearColor::White);

	/**
	* DrawSections - Draws the materials and the textures of the sections.
	* @param PreviousSelectedMaterial	Material for previously selected section.
	* @param CurrentSelectedMaterial	Material for curretnly selected section.
	* @param Background	Material for background of circular menu.
	* @param Sections	Number of sections to divide the circular menu.
	* @param ScreenSize	The current screen size.
	* @param Offset	The offset from 0 degrees <- experimental
	* @param LastSection	Retention of the last/previosly selected section.
	* @param WheelScale	The scale of the wheel. Default value: 600.0
	* @Return Returns the section on which the player is curretnly gazing.
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		int32 DrawSections(UMaterialInterface* PreviousSelectedMaterial, UMaterialInterface* CurrentSelectedMaterial, UTexture* Background, int32 Sections, FVector2D ScreenSize, float Offset = 0.0, int32 LastSection = 0, float WheelScale = 600.0);


	/**
	* DrawSections - Draws the materials and the textures of the sections.
	* @param PreviousSelectedMaterial	Material for previously selected section.
	* @param CurrentSelectedMaterial	Material for curretnly selected section.
	* @param Background	Material for background of circular menu.
	* @param Sections	Number of sections to divide the circular menu.
	* @param ScreenSize	The current screen size.
	* @param Offset	The offset from 0 degrees <- experimental
	* @param LastSection	Retention of the last/previosly selected section.
	* @param WheelScale	The scale of the wheel. Default value: 600.0
	* @Return Returns the section on which the player is curretnly gazing.
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		void DrawIcons(TArray<UTexture*> Icons, UTexture *OverLayImage, int32 Sections, int32 SelectedSection, float IconRadius, float Offset, FVector2D ScreenSize, float IconScale);


	/**
	* IsPointInQuad - Checks wether the point is in rectangle/(any four coordinates) or not.
	* @param Point The point to check if present or not in rectangle.
	* @param RectA Coordinate a of polygon/rectangle.
	* @param RectB Coordinate b of polygon/rectangle.
	* @param RectC Coordinate c of polygon/rectangle.
	* @param RectD Coordinate d of polygon/rectangle.
	* @Return Returns true if the point is in rectangle/(any four coordinates)
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		bool IsPointInQuad(FVector2D Point, FVector2D RectA, FVector2D RectB, FVector2D RectC, FVector2D RectD);


	/**
	* IsWithinCircle - Checks wether the point is in circle or not.
	* @param Point The point to check if present or not in rectangle.
	* @param Centre	Center of the circle
	* @param Radius Range/Radius
	* @Return Returns true if the point is incircle
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		bool IsWithinCircle(FVector2D Point, FVector2D Centre, float Radius);


	/**
	* TurnOffGaze - Turns off the Eye Gaze Circle
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		void TurnOffGaze();



	/**
	* TurnOffGaze - Turns on the Eye Gaze Circle
	*/
	UFUNCTION(BlueprintCallable, Category = "Channeler|Hud|Draw")
		void TurnOnGaze();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Channeler|Hud|Draw")
		bool DisplayEyeGaze = true;

private:
	/** Crosshair asset pointer */
	class UTexture2D* EyeGazeTex;

	/** Eyex used for getting the Gaze data */
	IEyeXPlugin* EyeX;


};

