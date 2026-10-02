#include "UI/Widget/SUAuraMagicDrawingWidget.h"

#include "Magic/UAuraMagicComponent.h"
#include "Player/AuraPlayerController.h"

#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"



void UAuraMagicDrawingWidget::InitializeDrawing(UAuraMagicComponent* InMagicComponent)
{
    MagicComponent = InMagicComponent;
}

void UAuraMagicDrawingWidget::CancelMagicDrawing()
{
    if (MagicComponent)
    {
        MagicComponent->CancelMagic();
    }
    AAuraPlayerController* PlayerController = Cast<AAuraPlayerController>(GetOwningPlayer());
    if (!PlayerController)
    {
        return;
    }
    PlayerController->StopMagicDrawingMode();
}

void UAuraMagicDrawingWidget::ResetMagicDrawing()
{
    if (!MagicComponent)
    {
        return;
    }

    MagicComponent->StartDrawing();

    bIsDrawing = false;
}

FReply UAuraMagicDrawingWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!MagicComponent)
    {
        return FReply::Unhandled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (MagicComponent->StartStroke())
        {
            bIsDrawing = true;
            const FVector2D NormalizedPoint =ScreenToNormalized(InGeometry,InMouseEvent.GetScreenSpacePosition());
            MagicComponent->AddStrokePoint(NormalizedPoint);

            return FReply::Handled().CaptureMouse(TakeWidget());
        }
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        return ConfirmMagicDrawing();
    }
    return FReply::Unhandled();
}

FReply UAuraMagicDrawingWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!MagicComponent || !bIsDrawing)
    {
        return FReply::Unhandled();
    }
    const FVector2D NormalizedPoint =ScreenToNormalized(InGeometry,InMouseEvent.GetScreenSpacePosition());
    MagicComponent->AddStrokePoint(NormalizedPoint);
    return FReply::Handled();
}

FReply UAuraMagicDrawingWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!MagicComponent)
    {
        return FReply::Unhandled();
    }

    if (InMouseEvent.GetEffectingButton() ==
        EKeys::LeftMouseButton)
    {
        if (bIsDrawing)
        {
            MagicComponent->FinishStroke();

            bIsDrawing = false;

            return FReply::Handled()
                .ReleaseMouseCapture();
        }
    }
    return FReply::Unhandled();
}

int32 UAuraMagicDrawingWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
    const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
   const int32 ResultLayer =Super::NativePaint(Args,AllottedGeometry,MyCullingRect,OutDrawElements,LayerId,InWidgetStyle,bParentEnabled);

    if (!MagicComponent)
    {
        return ResultLayer;
    }

    const float Width = AllottedGeometry.GetLocalSize().X;
    const float Height = AllottedGeometry.GetLocalSize().Y;
    const FMagicGesture& Gesture = MagicComponent->GetCurrentGesture();

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        if (Stroke.Points.Num() < 2)
        {
            continue;
        }

        TArray<FVector2D> ScreenPoints;
        ScreenPoints.Reserve(Stroke.Points.Num());

        for (const FVector2D& Point : Stroke.Points)
        {
            ScreenPoints.Add(FVector2D(Point.X * Width,Point.Y * Height));
        }

        FSlateDrawElement::MakeLines(OutDrawElements,ResultLayer + 1,AllottedGeometry.ToPaintGeometry(),ScreenPoints,ESlateDrawEffect::None,FLinearColor::White,true,4.0f);
    }

    if (MagicComponent->IsStrokeActive())
    {
        const FMagicStroke& CurrentStroke =MagicComponent->GetCurrentStroke();

        if (CurrentStroke.Points.Num() >= 2)
        {
            TArray<FVector2D> ScreenPoints;
            ScreenPoints.Reserve(CurrentStroke.Points.Num());

            for (const FVector2D& Point :CurrentStroke.Points)
            {
                ScreenPoints.Add(FVector2D(Point.X * Width,Point.Y * Height));
            }
            FSlateDrawElement::MakeLines(OutDrawElements,ResultLayer + 1,AllottedGeometry.ToPaintGeometry(),ScreenPoints,ESlateDrawEffect::None,FLinearColor::White,true,4.0f);
        }
    }

    return ResultLayer + 1;
}

FVector2D UAuraMagicDrawingWidget::ScreenToNormalized(const FGeometry& Geometry, const FVector2D& ScreenPosition) const
{
    const FVector2D LocalPosition =Geometry.AbsoluteToLocal(ScreenPosition);
    const FVector2D LocalSize =Geometry.GetLocalSize();

    if (LocalSize.X <= KINDA_SMALL_NUMBER || LocalSize.Y <= KINDA_SMALL_NUMBER)
    {
        return FVector2D::ZeroVector;
    }
    FVector2D NormalizedPosition;

    NormalizedPosition.X = LocalPosition.X / LocalSize.X;
    NormalizedPosition.Y = LocalPosition.Y / LocalSize.Y;
    NormalizedPosition.X = FMath::Clamp(NormalizedPosition.X,0.0f,1.0f);
    NormalizedPosition.Y = FMath::Clamp(NormalizedPosition.Y,0.0f,1.0f);

    return NormalizedPosition;
}

FReply UAuraMagicDrawingWidget::ConfirmMagicDrawing()
{
    if (!MagicComponent)
    {
        return FReply::Handled();
    }

    AAuraPlayerController* PlayerController =Cast<AAuraPlayerController>(GetOwningPlayer());

    if (!PlayerController)
    {
        return FReply::Handled();
    }

    PlayerController->ConfirmMagicDrawingMode();

    return FReply::Handled();
}

