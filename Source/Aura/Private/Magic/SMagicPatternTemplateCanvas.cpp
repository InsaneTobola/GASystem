#include "Magic/SMagicPatternTemplateCanvas.h"

#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"


void SMagicPatternTemplateCanvas::Construct(const FArguments& InArgs)
{
    Gesture = InArgs._Gesture;
    bIsDrawing = false;
}
FVector2D SMagicPatternTemplateCanvas::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
    return FVector2D(700.0f,700.0f);
}
FVector2D SMagicPatternTemplateCanvas::ScreenToNormalized(const FGeometry& Geometry,const FVector2D& ScreenPosition) const
{
    const FVector2D LocalPosition =Geometry.AbsoluteToLocal(ScreenPosition);
    const FVector2D LocalSize = Geometry.GetLocalSize();
    
    if (LocalSize.X <= KINDA_SMALL_NUMBER || LocalSize.Y <= KINDA_SMALL_NUMBER)
    {
        return FVector2D::ZeroVector;
    }
    FVector2D NormalizedPosition;
    
    NormalizedPosition.X =LocalPosition.X /LocalSize.X;
    NormalizedPosition.Y =LocalPosition.Y /LocalSize.Y;
    NormalizedPosition.X =FMath::Clamp(NormalizedPosition.X,0.0f,1.0f);
    NormalizedPosition.Y =FMath::Clamp(NormalizedPosition.Y,0.0f,1.0f);
    
    return NormalizedPosition;
}
void SMagicPatternTemplateCanvas::AddPoint(const FGeometry& Geometry,const FVector2D& ScreenPosition)
{
    if (!Gesture.IsValid() || Gesture->Strokes.Num() == 0)
    {
        return;
    }
    FMagicStroke& CurrentStroke = Gesture->Strokes.Last();
    
    const FVector2D Point =ScreenToNormalized(Geometry,ScreenPosition);

    if (CurrentStroke.Points.Num() > 0)
    {
        const FVector2D& Previous = CurrentStroke.Points.Last();
        constexpr float MinimumPointDistance =0.0025f;
        
        if (FVector2D::DistSquared(Previous,Point) < FMath::Square(MinimumPointDistance))
        {
            return;
        }
    }
    CurrentStroke.Points.Add(Point);
    
    Invalidate(EInvalidateWidgetReason::Paint);
}
FReply SMagicPatternTemplateCanvas::OnMouseButtonDown(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent)
{
    if (!Gesture.IsValid())
    {
        return FReply::Unhandled();
    }
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }
    FMagicStroke NewStroke;
    NewStroke.Points.Add(ScreenToNormalized(MyGeometry,MouseEvent.GetScreenSpacePosition()));
 
    Gesture->Strokes.Add(MoveTemp(NewStroke));
    bIsDrawing = true;
    Invalidate(EInvalidateWidgetReason::Paint);
    
    return FReply::Handled().CaptureMouse(AsShared());
}
FReply SMagicPatternTemplateCanvas::OnMouseMove(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent)
{
    if (!bIsDrawing || !Gesture.IsValid())
    {
        return FReply::Unhandled();
    }
    if (!MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        return FReply::Unhandled();
    }
    AddPoint(MyGeometry,MouseEvent.GetScreenSpacePosition());

    return FReply::Handled();
}
FReply SMagicPatternTemplateCanvas::OnMouseButtonUp(const FGeometry& MyGeometry,const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }
    if (!bIsDrawing)
    {
        return FReply::Unhandled();
    }
    bIsDrawing = false;

    return FReply::Handled().ReleaseMouseCapture();
}

int32 SMagicPatternTemplateCanvas::OnPaint(const FPaintArgs& Args,const FGeometry& AllottedGeometry,const FSlateRect& MyCullingRect,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FWidgetStyle& InWidgetStyle,bool bParentEnabled) const
{
    const FSlateBrush* BackgroundBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    
    FSlateDrawElement::MakeBox(OutDrawElements,LayerId,AllottedGeometry.ToPaintGeometry(),BackgroundBrush,ESlateDrawEffect::None,FLinearColor(0.05f,0.05f,0.05f,1.0f));
    
    if (!Gesture.IsValid())
    {
        return LayerId;
    }
    const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
    
    for (const FMagicStroke& Stroke : Gesture->Strokes)
    {
        if (Stroke.Points.Num() < 2)
        {
            continue;
        }
        
        TArray<FVector2D> ScreenPoints;
        ScreenPoints.Reserve(Stroke.Points.Num());
 
        for (const FVector2D& Point : Stroke.Points)
        {
            ScreenPoints.Add(FVector2D(Point.X * LocalSize.X,Point.Y * LocalSize.Y));
        }
        FSlateDrawElement::MakeLines(OutDrawElements,LayerId + 1,AllottedGeometry.ToPaintGeometry(),ScreenPoints,ESlateDrawEffect::None,FLinearColor::White,true,4.0f);
    }
    return LayerId + 1;
}