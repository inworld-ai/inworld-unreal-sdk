/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once


#include "CoreMinimal.h"

namespace Inworld
{
    template<class T>
    inline bool CheckEmpty(const T& Value) { return !Value; }
    template<class T>
    inline bool CheckEmpty(const TWeakObjectPtr<T>& Value) { return !Value.IsValid(); }
    template<>
    inline bool CheckEmpty<FString>(const FString& Value) { return Value.IsEmpty(); }
    template<class T>
    inline bool CheckEmpty(const TArray<T>& Value) { return Value.Num() == 0; }
}

#ifndef INWORLD_WARN_AND_RETURN_EMPTY
#define INWORLD_WARN_AND_RETURN_EMPTY(LogCategory, Class, Argument, Return) if (Inworld::CheckEmpty(Argument)) { UE_LOG(LogCategory, Warning, TEXT("%s::%s skipped: %s is empty."), TEXT(#Class), *FString(__func__), TEXT(#Argument)); return Return; }
#endif
