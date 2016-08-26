#pragma once

#include "GameFramework/SaveGame.h"

#include "DirectInputPadSaveGame.generated.h"

USTRUCT()
struct FDIPadKeyMaps
{
	GENERATED_BODY()

	UPROPERTY()
	FString	GUID;

	UPROPERTY()
	TArray<FDIKeyMapInfo> aMapInfo_;
};

UCLASS()
class DIRECTINPUTPADPLUGIN_API UDirectInputPadSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FDIPadKeyMaps> DIPadKeyMaps;
};
