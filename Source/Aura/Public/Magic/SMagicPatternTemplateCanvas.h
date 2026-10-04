#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

#include "Magic/MagicTypes.h"


class SMagicPatternTemplateCanvas : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SMagicPatternTemplateCanvas){}
	SLATE_ARGUMENT(TSharedPtr<FMagicGesture>,Gesture)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
	
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args,const FGeometry& AllottedGeometry,const FSlateRect& MyCullingRect,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FWidgetStyle& InWidgetStyle,bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent) override;

private:

	TSharedPtr<FMagicGesture> Gesture;
	bool bIsDrawing = false;
	
	FVector2D ScreenToNormalized(const FGeometry& Geometry,const FVector2D& ScreenPosition) const;
	void AddPoint(const FGeometry& Geometry,const FVector2D& ScreenPosition);
};