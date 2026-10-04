#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Magic/MagicTypes.h"
#include "Magic/MagicPointCloudRecognizer.h"

#include "MagicPatternDefinition.generated.h"


USTRUCT(BlueprintType)
struct AURA_API FMagicPatternTemplate
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
    FName TemplateId = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Template")
    FMagicGesture SourceGesture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Template")
    TArray<FVector2D> PointCloud;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Template")
    int32 PointCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Template")
    bool bIsBaked = false;
};


UCLASS(BlueprintType)
class AURA_API UMagicPatternDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    FName PatternId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    FText PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
    EMagicElement Element = EMagicElement::Fire;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recognition",meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinimumSimilarityScore = 0.30f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Templates",meta = (TitleProperty = "TemplateId"))
    TArray<FMagicPatternTemplate> Templates;


#if WITH_EDITORONLY_DATA

    UPROPERTY(EditAnywhere, Category = "Editor")
    int32 SelectedTemplateIndex = 0;

    UFUNCTION(CallInEditor, Category = "Editor")
    void AddTemplate();

    UFUNCTION(CallInEditor, Category = "Editor")
    void RemoveSelectedTemplate();

    UFUNCTION(CallInEditor, Category = "Editor")
    void OpenTemplateEditor();

    UFUNCTION(CallInEditor, Category = "Editor")
    void BakeTemplates();

    UFUNCTION(CallInEditor, Category = "Preview")
    void PreviewTemplates();
    
#endif
    
    void BuildRuntimeTemplates(TArray<FMagicPointCloudTemplate>& OutTemplates) const;

private:
    
    bool BakeTemplate(FMagicPatternTemplate& Template,int32 TemplateIndex);

};