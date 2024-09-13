/**
 * Copyright 2022-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/UObjectGlobals.h"
#include "InworldTestMacros.h"

namespace Inworld
{
	namespace Test
	{
		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(AddObjectToRoot, UObject*, Object);
		bool FAddObjectToRootCommand::Update()
		{
			Object->AddToRoot();
			return true;
		}

		DEFINE_INWORLD_TEST_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(RemoveObjectFromRoot, UObject*, Object);
		bool FRemoveObjectFromRootCommand::Update()
		{
			Object->RemoveFromRoot();
			GEngine->ForceGarbageCollection(true);
			return true;
		}

		template<typename T>
		struct TScopedGCObject
		{
		public:
			TScopedGCObject()
				: GCObject(NewObject<T>())
			{
				AddObjectToRoot(GCObject);
			}

			~TScopedGCObject()
			{
				RemoveObjectFromRoot(GCObject);
			}

			FORCEINLINE const bool IsValid() const
			{
				return GCObject != nullptr;
			}

			FORCEINLINE T& Get() const
			{
				checkSlow(IsValid());
				return *GCObject;
			}

			FORCEINLINE T& operator*() const
			{
				checkSlow(IsValid());
				return *GCObject;
			}

			FORCEINLINE T* operator->() const
			{
				checkSlow(IsValid());
				return GCObject;
			}

		private:
			T* GCObject;
		};
	}
}
