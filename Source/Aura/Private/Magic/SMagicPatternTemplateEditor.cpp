#include "Magic/SMagicPatternTemplateEditor.h"

#include "Magic/MagicPatternDefinition.h"
#include "Magic/SMagicPatternTemplateCanvas.h"

#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

#include "Framework/Application/SlateApplication.h"


void SMagicPatternTemplateEditor::Construct(
    const FArguments& InArgs)
{
    PatternDefinition =
        InArgs._PatternDefinition;

    TemplateIndex =
        InArgs._TemplateIndex;


    EditingGesture =
        MakeShared<FMagicGesture>();


    if (PatternDefinition.IsValid() &&
        PatternDefinition->Templates.IsValidIndex(
            TemplateIndex))
    {
        *EditingGesture =
            PatternDefinition
                ->Templates[TemplateIndex]
                .SourceGesture;
    }


    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f)
        [
            SNew(STextBlock)
            .Text(
                GetTemplateName())
        ]


        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(8.0f)
        [
            SNew(SBorder)
            .Padding(2.0f)
            [
                SAssignNew(
                    Canvas,
                    SMagicPatternTemplateCanvas)
                .Gesture(
                    EditingGesture)
            ]
        ]


        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f)
        [
            SNew(SHorizontalBox)


            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(
                    FText::FromString(
                        TEXT("Clear")))
                .OnClicked(
                    FOnClicked::CreateSP(
                        this,
                        &SMagicPatternTemplateEditor::ClearTemplate))
            ]


            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(
                    FText::FromString(
                        TEXT("Cancel")))
                .OnClicked(
                    FOnClicked::CreateSP(
                        this,
                        &SMagicPatternTemplateEditor::Cancel))
            ]


            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f)
            [
                SNew(SButton)
                .Text(
                    FText::FromString(
                        TEXT("Save Template")))
                .OnClicked(
                    FOnClicked::CreateSP(
                        this,
                        &SMagicPatternTemplateEditor::SaveTemplate))
            ]
        ]
    ];
}


void SMagicPatternTemplateEditor::Open(
    UMagicPatternDefinition* PatternDefinition,
    int32 TemplateIndex)
{
    if (!IsValid(PatternDefinition))
    {
        return;
    }


    if (!PatternDefinition->Templates.IsValidIndex(
            TemplateIndex))
    {
        return;
    }


    const FString PatternName =
        PatternDefinition->PatternId.IsNone()
            ? PatternDefinition->GetName()
            : PatternDefinition->PatternId.ToString();


    TSharedRef<SWindow> Window =
        SNew(SWindow)
        .Title(
            FText::Format(
                FText::FromString(
                    TEXT("Magic Pattern Template - {0}")),
                FText::FromString(
                    PatternName)))
        .ClientSize(
            FVector2D(
                900.0f,
                900.0f))
        .SupportsMaximize(false)
        .SupportsMinimize(false);


    TSharedRef<SMagicPatternTemplateEditor>
        Editor =
            SNew(SMagicPatternTemplateEditor)
            .PatternDefinition(
                PatternDefinition)
            .TemplateIndex(
                TemplateIndex);


    Editor->ParentWindow =
        Window;


    Window->SetContent(
        Editor);


    FSlateApplication::Get().AddWindow(
        Window);
}


FText SMagicPatternTemplateEditor::GetTemplateName() const
{
    if (!PatternDefinition.IsValid())
    {
        return FText::FromString(
            TEXT("Invalid Pattern"));
    }


    if (!PatternDefinition->Templates.IsValidIndex(
            TemplateIndex))
    {
        return FText::FromString(
            TEXT("Invalid Template"));
    }


    const FMagicPatternTemplate& Template =
        PatternDefinition
            ->Templates[TemplateIndex];


    return FText::Format(
        FText::FromString(
            TEXT("Template: {0}")),
        FText::FromName(
            Template.TemplateId));
}


FReply SMagicPatternTemplateEditor::SaveTemplate()
{
    if (!PatternDefinition.IsValid())
    {
        return FReply::Handled();
    }


    if (!PatternDefinition->Templates.IsValidIndex(
            TemplateIndex))
    {
        return FReply::Handled();
    }


    PatternDefinition->Modify();


    FMagicPatternTemplate& Template =
        PatternDefinition
            ->Templates[TemplateIndex];


    Template.SourceGesture =
        *EditingGesture;


    Template.PointCloud.Reset();

    Template.PointCount = 0;

    Template.bIsBaked = false;


    PatternDefinition->BakeTemplates();


    PatternDefinition->MarkPackageDirty();

    PatternDefinition->PostEditChange();


    CloseWindow();


    return FReply::Handled();
}


FReply SMagicPatternTemplateEditor::Cancel()
{
    CloseWindow();

    return FReply::Handled();
}


FReply SMagicPatternTemplateEditor::ClearTemplate()
{
    if (EditingGesture.IsValid())
    {
        EditingGesture->Reset();
    }


    if (Canvas.IsValid())
    {
        Canvas->Invalidate(
            EInvalidateWidgetReason::Paint);
    }


    return FReply::Handled();
}


void SMagicPatternTemplateEditor::CloseWindow()
{
    if (TSharedPtr<SWindow> Window =
            ParentWindow.Pin())
    {
        Window->RequestDestroyWindow();
    }
}