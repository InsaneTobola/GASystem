#include "Magic/MagicDebug.h"
#include "Engine/Engine.h"

void AuraMagicDebugPrint(
    const FString& Message,
    FColor Color,
    float Duration)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[MagicDebug] %s"),
        *Message
    );  

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            Duration,
            Color,
            Message
        );
    }
}