#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#include "Magic/MagicTypes.h"


class SWindow;
class SMagicPatternTemplateCanvas;
class UMagicPatternDefinition;


class SMagicPatternTemplateEditor
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SMagicPatternTemplateEditor){}
	SLATE_ARGUMENT(UMagicPatternDefinition*,PatternDefinition)
	SLATE_ARGUMENT(int32,TemplateIndex)
	SLATE_END_ARGS()
	
	void Construct(
	const FArguments& InArgs);
	static void Open(UMagicPatternDefinition* PatternDefinition,int32 TemplateIndex);

private:

	TWeakObjectPtr<UMagicPatternDefinition>PatternDefinition;
	int32 TemplateIndex =INDEX_NONE;
	TSharedPtr<FMagicGesture>EditingGesture;
	TSharedPtr<SMagicPatternTemplateCanvas>Canvas;
	TWeakPtr<SWindow>ParentWindow;
	
	FReply SaveTemplate();
	FReply Cancel();
	FReply ClearTemplate();
	void CloseWindow();
	FText GetTemplateName() const;
};