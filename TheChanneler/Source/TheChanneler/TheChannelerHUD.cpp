// Fill out your copyright notice in the Description page of Project Settings.


#include "TheChanneler.h"
#include "Engine.h"
#include "TheChannelerHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

ATheChannelerHUD::ATheChannelerHUD()
{
	EyeX = &IEyeXPlugin::Get();

	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> EyeGazeTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	EyeGazeTex = EyeGazeTexObj.Object;
}


void ATheChannelerHUD::DrawHUD()
{
	Super::DrawHUD();
}

void ATheChannelerHUD::SetMousePosition()
{
	UPlayer*  Player = GetWorld()->GetFirstPlayerController()->Player;
	FViewport* v = CastChecked<ULocalPlayer>(Player)->ViewportClient->Viewport;

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);


	v->SetMouse((int32)Center.X, (int32)Center.Y);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X - (EyeGazeTex->GetSurfaceWidth() * 0.5)),
		(Center.Y - (EyeGazeTex->GetSurfaceHeight() * 0.5f)));

	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, EyeGazeTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);

	/*
	//Leftover code: Did not delete because this code might be necessary/useful in future changes.
	if (DetectionMode == EEyeXEmulationMode::Enabled)
	{
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	FViewport* v = CastChecked<ULocalPlayer>(this->Player)->ViewportClient->Viewport;
	int intX = (int)ViewportSize.X / 2;
	int intY = (int)ViewportSize.Y / 2;
	v->SetMouse(intX, intY);
	}
	*/
}

void ATheChannelerHUD::DrawSectionLines(int32 Sections, float InnerRadius, float OuterRadius, FVector2D ScreenSize, float Offset, FLinearColor LineColor)
{
	float SectorAngle = ((float)360 / Sections);


	float StartScreenX = 0.0f;
	float StartScreenY = 0.0f;
	float EndScreenX = 0.0f;
	float EndScreenY = 0.0f;
	float SinValue = 0.0f;
	float CosValue = 0.0f;

	for (int32 i = 1; i <= Sections; i++)
	{
		SinValue = sin(FMath::DegreesToRadians((SectorAngle*i) - 90 + Offset));
		CosValue = cos(FMath::DegreesToRadians((SectorAngle*i) - 90 + Offset));
		EndScreenX = OuterRadius*CosValue + ScreenSize.X;
		EndScreenY = OuterRadius*SinValue + ScreenSize.Y;
		StartScreenX = InnerRadius*CosValue + ScreenSize.X;
		StartScreenY = InnerRadius*SinValue + ScreenSize.Y;
		DrawLine(StartScreenX, StartScreenY, EndScreenX, EndScreenY, LineColor);
	}
}

int32 ATheChannelerHUD::DrawSections(UMaterialInterface *PreviousSelectedMaterial, UMaterialInterface *CurrentSelectedMaterial, UTexture *Background, int32 Sections, FVector2D ScreenSize, float Offset, int32 LastSection, float WheelScale)
{
	FVector2D Gaze = EyeX->GetGazePoint(EEyeXGazePointDataMode::LightlyFiltered).Value;

	//FVector2D Gaze = GazePoint;
	FVector2D Pivot(0.5, 0.5);
	float EyeGazeAngle = FMath::RadiansToDegrees(FMath::Atan2((ScreenSize.X - Gaze.X), ((ScreenSize.Y - Gaze.Y))));

	//EyeGazeAngle -= Offset;
	if (EyeGazeAngle <= 0.0)
	{
		EyeGazeAngle += 360;
	}

	int32 CurrentSection = 1 + ((int32)EyeGazeAngle / ((float)360 / Sections));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("EyeGazeAngle =  %f sections: %f final val: %f"), EyeGazeAngle, ((float)360 / Sections), (EyeGazeAngle / ((float)360 / Sections))));//debug
	float RotationLast = ((float)360 / Sections) - (((float)360 / Sections) * LastSection);// +Offset;;
	float RotationCurrent = ((float)360 / Sections) - (((float)360 / Sections) * CurrentSection);// +Offset;;

	DrawMaterial(PreviousSelectedMaterial, (ScreenSize.X - WheelScale / 2), (ScreenSize.Y - WheelScale / 2), WheelScale, WheelScale, 0, 0, 1, 1, 1.0, false, RotationLast, Pivot);
	DrawTexture(Background, (ScreenSize.X - WheelScale / 2), (ScreenSize.Y - WheelScale / 2), WheelScale, WheelScale, 1, 1, 1, 1, FLinearColor::White, EBlendMode::BLEND_Translucent, 1, true);
	DrawMaterial(CurrentSelectedMaterial, (ScreenSize.X - WheelScale / 2), (ScreenSize.Y - WheelScale / 2), WheelScale, WheelScale, 0, 0, 1, 1, 1.0, false, RotationCurrent, Pivot);
	return CurrentSection;
}


void ATheChannelerHUD::DrawIcons(TArray<UTexture*> Icons, UTexture *OverLayImage, int32 Sections, int32 SelectedSection, float IconRadius, float Offset, FVector2D ScreenSize, float IconScale)
{
	float SectorAngle = ((float)360 / Sections);
	float IconCenterAngle = ((float)SectorAngle / 2);

	float IconX = 0.0f;
	float IconY = 0.0f;
	float SinValue = 0.0f;
	float CosValue = 0.0f;

	SelectedSection = Sections - SelectedSection;

	float SelectionScaleIncreaseBy = 10;
	float OverLayImageScale = IconScale;
	IconScale *= (0.5);


	for (int32 iconIndex = 0; iconIndex < (Sections); iconIndex++)
	{

		SinValue = sin(FMath::DegreesToRadians((SectorAngle*iconIndex) + IconCenterAngle - 90 + Offset));
		CosValue = cos(FMath::DegreesToRadians((SectorAngle*iconIndex) + IconCenterAngle - 90 + Offset));
		IconX = IconRadius * CosValue + ScreenSize.X;
		IconY = IconRadius * SinValue + ScreenSize.Y;
		if (iconIndex == SelectedSection)
		{
			DrawTexture(Icons[iconIndex], (IconX - (IconScale + SelectionScaleIncreaseBy)/ 2), (IconY - (IconScale + SelectionScaleIncreaseBy)/ 2), IconScale + SelectionScaleIncreaseBy, IconScale + SelectionScaleIncreaseBy, 1, 1, 1, 1, FLinearColor::White, EBlendMode::BLEND_Translucent, 1, true);
			DrawTexture(OverLayImage, (IconX - (OverLayImageScale + SelectionScaleIncreaseBy)/ 2), (IconY - (OverLayImageScale + SelectionScaleIncreaseBy)/ 2), OverLayImageScale + SelectionScaleIncreaseBy, OverLayImageScale + SelectionScaleIncreaseBy, 1, 1, 1, 1, FLinearColor::White, EBlendMode::BLEND_Translucent, 1, true);
		}
		else
		{
			DrawTexture(Icons[iconIndex], (IconX - IconScale / 2), (IconY - IconScale / 2), IconScale, IconScale, 1, 1, 1, 1, FLinearColor::White, EBlendMode::BLEND_Translucent, 1, true);
			DrawTexture(OverLayImage, (IconX - OverLayImageScale /2), (IconY - OverLayImageScale /2), OverLayImageScale, OverLayImageScale, 1, 1, 1, 1, FLinearColor::White, EBlendMode::BLEND_Translucent, 1, true);
		}
	}
}



bool ATheChannelerHUD::IsPointInQuad(FVector2D Point, FVector2D RectA, FVector2D RectB, FVector2D RectC, FVector2D RectD)
{

	FVector2D RectangleCoordinates[5] = { RectA, RectB, RectC, RectD, RectA };

	double AreaOfQuad = 0;
	float AreaSumOfTriangles = 0;

	for (int32 vertex = 0; vertex < (4/*QUAD_SIDES*/); vertex++)
	{
		AreaSumOfTriangles += FMath::Abs(
			(0.5f)*(
				((Point.X - RectangleCoordinates[vertex + 1].X)*
					(RectangleCoordinates[vertex].Y - Point.Y)) -
				((Point.X - RectangleCoordinates[vertex].X)*
					(RectangleCoordinates[vertex + 1].Y - Point.Y))
				));
		//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("AreaSumOfTriangles =  %f vertex: %d "), AreaSumOfTriangles, vertex));//debug
	}


	AreaOfQuad = FMath::Abs(
		(0.5f)*(
			(RectangleCoordinates[0].X - RectangleCoordinates[2].X)*
			(RectangleCoordinates[1].Y - RectangleCoordinates[3].Y) -
			(RectangleCoordinates[1].X - RectangleCoordinates[3].X)*
			(RectangleCoordinates[0].Y - RectangleCoordinates[2].Y)
			));

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("AreaSumOfTriangles =  %f AreaOfQuad: %f "), AreaSumOfTriangles, AreaOfQuad));//debug
	return ((int32)AreaSumOfTriangles == (int32)AreaOfQuad);
}

bool ATheChannelerHUD::IsWithinCircle(FVector2D Point, FVector2D Centre, float Radius)
{
	//float EyeGazeLengthFromCentre =  FMath::Abs((Point.X * Point.X - Centre.X * Centre.X) + (Point.Y * Point.Y - Centre.Y * Centre.Y));
	return ((FMath::Abs(FMath::Square(Point.X - Centre.X) + FMath::Square(Point.Y - Centre.Y))) < Radius*Radius);
}


void ATheChannelerHUD::TurnOffGaze()
{
	DisplayEyeGaze = false;
}

void ATheChannelerHUD::TurnOnGaze()
{
	DisplayEyeGaze = true;
}
