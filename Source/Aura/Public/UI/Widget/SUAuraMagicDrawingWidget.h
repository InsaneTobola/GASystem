#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SUAuraMagicDrawingWidget.generated.h"

class UAuraMagicComponent;
class UButton;

UCLASS()
class AURA_API UAuraMagicDrawingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeDrawing(UAuraMagicComponent* InMagicComponent);

	UFUNCTION(BlueprintCallable)
	void CancelMagicDrawing();
	
	UFUNCTION(BlueprintCallable)
	void ResetMagicDrawing();
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry,const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry,const FPointerEvent& InMouseEvent) override;

	virtual int32 NativePaint(const FPaintArgs& Args,const FGeometry& AllottedGeometry,const FSlateRect& MyCullingRect,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FWidgetStyle& InWidgetStyle,bool bParentEnabled) const override;

private:
	UPROPERTY()
	TObjectPtr<UAuraMagicComponent> MagicComponent;
	
	bool bIsDrawing = false;
	FVector2D ScreenToNormalized(const FGeometry& Geometry,const FVector2D& ScreenPosition) const;
	FReply ConfirmMagicDrawing();
	bool IsInsideDrawingSquare(const FGeometry& Geometry,const FVector2D& ScreenPosition) const;
};