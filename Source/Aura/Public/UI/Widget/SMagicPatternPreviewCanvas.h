#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class UMagicPatternDefinition;

class SMagicPatternPreviewCanvas : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SMagicPatternPreviewCanvas)
	{}

	SLATE_ARGUMENT(
		UMagicPatternDefinition*,
		PatternDefinition)

SLATE_END_ARGS()


void Construct(
	const FArguments& InArgs);


	static void Open(
		UMagicPatternDefinition* PatternDefinition);


	virtual FVector2D ComputeDesiredSize(
		float LayoutScaleMultiplier) const override;


	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;


private:

	TWeakObjectPtr<UMagicPatternDefinition>
		PatternDefinition;


	void DrawGesture(
		const FGeometry& Geometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const struct FMagicGesture& Gesture,
		const FVector2D& CanvasMin,
		const FVector2D& CanvasSize) const;
};