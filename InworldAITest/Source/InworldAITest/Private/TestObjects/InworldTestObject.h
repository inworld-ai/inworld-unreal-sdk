/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InworldTypes.h"
#include "Commands/InworldTestCommandsGarbageCollection.h"
#include "InworldTestObject.generated.h"


UCLASS()
class UInworldTestObject : public UObject
{
	GENERATED_BODY()

public:
	UInworldTestObject()
	{
		if (!FParse::Value(FCommandLine::Get(), TEXT("InworldTestRuntimeApiKey="), RuntimeAuth.Base64Signature))
		{
			const UInworldAITestSettings* InworldAITestSettings = GetDefault<UInworldAITestSettings>();
			RuntimeAuth.Base64Signature = InworldAITestSettings->RuntimeApiKey;
		}
	}

	UPROPERTY()
	FInworldAuth RuntimeAuth;
};

namespace Inworld
{
	namespace Test
	{
		template<typename T>
		struct TInworldTestObjectScoped
		{
		public:
			TInworldTestObjectScoped(FAutomationTestBase* Test)
				: OwningTest(Test)
				, InworldTestObject(NewObject<T>())
			{
				ADD_LATENT_AUTOMATION_COMMAND(AddObjectToRoot(InworldTestObject));
			}

			~TInworldTestObjectScoped()
			{
				ADD_LATENT_AUTOMATION_COMMAND(RemoveObjectFromRoot(InworldTestObject));
			}

			FORCEINLINE const bool IsValid() const
			{
				return InworldTestObject != nullptr;
			}

			FORCEINLINE T& Get() const
			{
				checkSlow(IsValid());
				return *InworldTestObject;
			}

			FORCEINLINE T& operator*() const
			{
				checkSlow(IsValid());
				return *InworldTestObject;
			}

			FORCEINLINE T* operator->() const
			{
				checkSlow(IsValid());
				return InworldTestObject;
			}

		protected:
			FAutomationTestBase* OwningTest;
		private:
			T* InworldTestObject;
		};
	}
}

