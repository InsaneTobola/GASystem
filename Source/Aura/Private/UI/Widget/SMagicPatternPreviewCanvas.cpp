#include "UI/Widget/SMagicPatternPreviewCanvas.h"

#include "Magic/MagicPatternDefinition.h"
#include "Magic/MagicTypes.h"

#include "Framework/Application/SlateApplication.h"

#include "Styling/AppStyle.h"

#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"


void SMagicPatternPreviewCanvas::Construct(const FArguments& InArgs)
{
    PatternDefinition =        InArgs._PatternDefinition;
}


FVector2D SMagicPatternPreviewCanvas::ComputeDesiredSize(    float LayoutScaleMultiplier) const
{
    if (!PatternDefinition.IsValid())
    {
        return FVector2D(800.0f,300.0f);
    }
    constexpr float TemplateHeight = 360.0f;
    const int32 TemplateCount =PatternDefinition->Templates.Num();
 
    return FVector2D(900.0f,FMath::Max(300.0f,TemplateCount * TemplateHeight));
}
void SMagicPatternPreviewCanvas::Open(UMagicPatternDefinition* InPatternDefinition)
{
    if (!IsValid(InPatternDefinition))
    {
        return;
    }
    
    TSharedRef<SWindow> Window = SNew(SWindow).Title(FText::Format(FText::FromString(
                    TEXT("Magic Pattern Preview - {0}")),
                InPatternDefinition->PatternName.IsEmpty()
                    ? FText::FromName(
                        InPatternDefinition->PatternId)
                    : InPatternDefinition->PatternName))
        .ClientSize(
            FVector2D(
                1000.0f,
                800.0f))
        .SupportsMaximize(true)
        .SupportsMinimize(false);
    
    Window->SetContent(SNew(SScrollBox)+ SScrollBox::Slot()[SNew(SBox).WidthOverride(900.0f)[SNew(SMagicPatternPreviewCanvas).PatternDefinition(InPatternDefinition)]]
    );
    FSlateApplication::Get().AddWindow(Window);
}


void SMagicPatternPreviewCanvas::DrawGesture(const FGeometry& Geometry,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FMagicGesture& Gesture,const FVector2D& CanvasMin,const FVector2D& CanvasSize) const
{
    bool bFoundPoint = false;
    
    FVector2D Min(TNumericLimits<float>::Max(),TNumericLimits<float>::Max());
    FVector2D Max(-TNumericLimits<float>::Max(),-TNumericLimits<float>::Max());

    for (const FMagicStroke& Stroke : Gesture.Strokes)
    {
        for (const FVector2D& Point : Stroke.Points)
        {
            if (!FMath::IsFinite(Point.X) || !FMath::IsFinite(Point.Y))
            {
                continue;
            }
            bFoundPoint = true;

            Min.X = FMath::Min(Min.X, Point.X);
            Min.Y = FMath::Min(Min.Y, Point.Y);
            Max.X = FMath::Max(Max.X, Point.X);
            Max.Y = FMath::Max(Max.Y, Point.Y);
        }
    }
    if (!bFoundPoint)
    {
        return;
    }


    const float Width = Max.X - Min.X;
    const float Height = Max.Y - Min.Y;

    if (Width <= KINDA_SMALL_NUMBER || Height <= KINDA_SMALL_NUMBER)
    {
        return;
    }
    constexpr float Padding = 30.0f;
    const FVector2D AvailableSize = CanvasSize - FVector2D(Padding * 2.0f,Padding * 2.0f);
    const float Scale = FMath::Min(AvailableSize.X / Width,AvailableSize.Y / Height);
    const FVector2D DrawSize(Width * Scale,Height * Scale);
    const FVector2D Offset = CanvasMin + FVector2D((CanvasSize.X - DrawSize.X) * 0.5f,(CanvasSize.Y - DrawSize.Y) * 0.5f);
    
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
            const FVector2D LocalPoint((Point.X - Min.X) * Scale,(Point.Y - Min.Y) * Scale);
            
            ScreenPoints.Add(Offset + LocalPoint);
        }
        
        FSlateDrawElement::MakeLines(OutDrawElements,LayerId,Geometry.ToPaintGeometry(),ScreenPoints,ESlateDrawEffect::None,FLinearColor::White,true,4.0f);
    }
}


int32 SMagicPatternPreviewCanvas::OnPaint(const FPaintArgs& Args,const FGeometry& AllottedGeometry,const FSlateRect& MyCullingRect,FSlateWindowElementList& OutDrawElements,int32 LayerId,const FWidgetStyle& InWidgetStyle,bool bParentEnabled) const
{
    if (!PatternDefinition.IsValid())
    {
        return LayerId;
    }
    
    const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
    constexpr float TemplateHeight = 360.0f;
    constexpr float TemplatePadding = 20.0f;
    const FSlateBrush* WhiteBrush = FAppStyle::Get().GetBrush("WhiteBrush");
    const FSlateFontInfo Font = FAppStyle::Get().GetFontStyle("NormalFont");
    
    for (int32 Index = 0;Index < PatternDefinition->Templates.Num();++Index)
    {
        const FMagicPatternTemplate& Template = PatternDefinition->Templates[Index];
        const float Top = Index * TemplateHeight;
        const FVector2D CardMin(TemplatePadding,Top + TemplatePadding);
        const FVector2D CardSize(LocalSize.X - TemplatePadding * 2.0f,TemplateHeight - TemplatePadding * 2.0f);
        
        FSlateDrawElement::MakeBox( OutDrawElements,
    LayerId,
    AllottedGeometry.ToPaintGeometry(CardSize,FSlateLayoutTransform(CardMin)),WhiteBrush,ESlateDrawEffect::None,FLinearColor(0.08f,0.08f,0.08f,1.0f));
        
        const FString TemplateLabel =
            FString::Printf(
                TEXT("Template: %s | Points: %d | Baked: %s"),
                *Template.TemplateId.ToString(),
                Template.PointCount,
                Template.bIsBaked
                    ? TEXT("Yes")
                    : TEXT("No"));

        const FVector2D TextPosition = CardMin + FVector2D(15.0f, 10.0f);
        const FVector2D TextSize(CardSize.X - 30.0f,25.0f);

        FSlateDrawElement::MakeText(OutDrawElements,LayerId + 1,AllottedGeometry.ToPaintGeometry(TextSize,FSlateLayoutTransform(TextPosition)),TemplateLabel,Font,ESlateDrawEffect::None,FLinearColor::White);
        
        const FVector2D PreviewMin(CardMin.X + 15.0f,CardMin.Y + 45.0f);
        const FVector2D PreviewSize(CardSize.X - 30.0f,CardSize.Y - 60.0f);


        if (Template.SourceGesture.Strokes.Num() > 0)
        {
            DrawGesture(AllottedGeometry,OutDrawElements,LayerId + 1,Template.SourceGesture,PreviewMin,PreviewSize);
        }
        else if (Template.PointCloud.Num() >= 2)
        {
            FMagicGesture BakedGesture;
            FMagicStroke BakedStroke;
            
            BakedStroke.Points =Template.PointCloud;
            BakedGesture.Strokes.Add(MoveTemp(BakedStroke));
            
            DrawGesture(AllottedGeometry,OutDrawElements,LayerId + 1,BakedGesture,PreviewMin,PreviewSize);
        }
    }


    return LayerId + 2;
}