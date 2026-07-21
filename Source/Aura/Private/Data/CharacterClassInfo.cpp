// Copyright Druid Mechanics


#include "Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(EcharacterClass CharacterClass)
{
	return CharacterClassInformation.FindChecked(CharacterClass);
}
