#include "Magic/MagicPatternDefinition.h"

#include "Math/UnrealMathUtility.h"
#include "UI/Widget/SMagicPatternPreviewCanvas.h"

#if WITH_EDITOR

#include "Magic/SMagicPatternTemplateEditor.h"

#endif


#if WITH_EDITOR

void UMagicPatternDefinition::AddTemplate()
{
    Modify();
    FMagicPatternTemplate NewTemplate;
    const int32 NewIndex = Templates.Num() + 1;

    NewTemplate.TemplateId =
        FName(
            *FString::Printf(
                TEXT("Template_%02d"),
                NewIndex
            )
        );
    Templates.Add(MoveTemp(NewTemplate));
    SelectedTemplateIndex = Templates.Num() - 1;

    MarkPackageDirty();
    PostEditChange();
}
void UMagicPatternDefinition::RemoveSelectedTemplate()
{
    if (!Templates.IsValidIndex(SelectedTemplateIndex))
    {
        return;
    }
    Modify();

    Templates.RemoveAt(SelectedTemplateIndex);

    if (Templates.Num() == 0)
    {
        SelectedTemplateIndex = 0;
    }
    else
    {
        SelectedTemplateIndex = FMath::Clamp(SelectedTemplateIndex,0,Templates.Num() - 1);
    }
    MarkPackageDirty();
    PostEditChange();
}


void UMagicPatternDefinition::OpenTemplateEditor()
{
    if (!Templates.IsValidIndex(SelectedTemplateIndex))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': invalid template index %d."
            ),
            *GetName(),
            SelectedTemplateIndex);

        return;
    }
    SMagicPatternTemplateEditor::Open(this,SelectedTemplateIndex);
}

void UMagicPatternDefinition::BakeTemplates()
{
    Modify();

    bool bAnyFailed = false;

    for (int32 Index = 0;Index < Templates.Num();++Index)
    {
        if (!BakeTemplate(Templates[Index],Index))
        {
            bAnyFailed = true;
        }
    }
    MarkPackageDirty();
    PostEditChange();

    if (bAnyFailed)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': one or more templates failed to bake."
            ),
            *GetName());
    }
    else
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "Magic Pattern '%s': all templates baked."
            ),
            *GetName());
    }
}

void UMagicPatternDefinition::PreviewTemplates()
{
    SMagicPatternPreviewCanvas::Open(this);
}

bool UMagicPatternDefinition::BakeTemplate(FMagicPatternTemplate& Template,int32 TemplateIndex)
{
    Template.PointCloud.Reset();
    Template.PointCount = 0;
    Template.bIsBaked = false;

    if (Template.TemplateId.IsNone())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': template %d has no TemplateId."
            ),
            *GetName(),
            TemplateIndex);

        return false;
    }


    if (Template.SourceGesture.GetTotalPointCount() < 2)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': template '%s' has too few points."
            ),
            *GetName(),
            *Template.TemplateId.ToString());

        return false;
    }
    FMagicPointCloud PointCloud;

    if (!FMagicPointCloudRecognizer::BuildPointCloud(Template.SourceGesture,PointCloud))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': failed to build point cloud for template '%s'."
            ),
            *GetName(),
            *Template.TemplateId.ToString());

        return false;
    }

    /*
     * BuildPointCloud already performs:
     * - flattening
     * - resampling
     * - scaling
     * - translation to origin
     * and produces the 32-point cloud.
     */


    if (!PointCloud.IsValid(FMagicPointCloudRecognizer::ResamplePointCount))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "Magic Pattern '%s': template '%s' does not contain %d baked points."
            ),
            *GetName(),
            *Template.TemplateId.ToString(),
            FMagicPointCloudRecognizer::ResamplePointCount);

        return false;
    }
    Template.PointCloud = PointCloud.Points;
    Template.PointCount = Template.PointCloud.Num();
    Template.bIsBaked = true;


    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "Magic Pattern '%s': baked template '%s' with %d points."
        ),
        *GetName(),
        *Template.TemplateId.ToString(),
        Template.PointCount);


    return true;
}

#endif

void UMagicPatternDefinition::BuildRuntimeTemplates(TArray<FMagicPointCloudTemplate>& OutTemplates) const
{
    OutTemplates.Reset();
    
    for (const FMagicPatternTemplate& SourceTemplate : Templates)
    {
        if (!SourceTemplate.bIsBaked)
        {
            continue;
        }
        if (SourceTemplate.TemplateId.IsNone())
        {
            continue;
        }
        if (SourceTemplate.PointCloud.Num() != FMagicPointCloudRecognizer::ResamplePointCount)
        {
            continue;
        }
        FMagicPointCloudTemplate RuntimeTemplate;
        RuntimeTemplate.PatternId = PatternId;
        RuntimeTemplate.TemplateId = SourceTemplate.TemplateId;
        RuntimeTemplate.PointCloud.Points = SourceTemplate.PointCloud;
        OutTemplates.Add(MoveTemp(RuntimeTemplate));
    }
}
