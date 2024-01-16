/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldIntegrationTypes.h"

namespace Code {
	static const FName PP = { TEXT("PP") };
	static const FName FF = { TEXT("FF") };
	static const FName TH = { TEXT("TH") };
	static const FName DD = { TEXT("DD") };
	static const FName Kk = { TEXT("Kk") };
	static const FName CH = { TEXT("CH") };
	static const FName SS = { TEXT("SS") };
	static const FName Nn = { TEXT("Nn") };
	static const FName RR = { TEXT("RR") };
	static const FName Aa = { TEXT("Aa") };
	static const FName E = { TEXT("E") };
	static const FName I = { TEXT("I") };
	static const FName O = { TEXT("O") };
	static const FName U = { TEXT("U") };
	static const FName STOP = { TEXT("STOP") };
};

float& FInworldCharacterVisemeBlends::operator[](const FString& Code)
{
	return (*this)[FName(*Code)];
}

float& FInworldCharacterVisemeBlends::operator[](const FName& Code)
{
	if (Code == Code::PP) return PP;
	else if (Code == Code::FF) return FF;
	else if (Code == Code::TH) return TH;
	else if (Code == Code::DD) return DD;
	else if (Code == Code::Kk) return Kk;
	else if (Code == Code::CH) return CH;
	else if (Code == Code::SS) return SS;
	else if (Code == Code::Nn) return Nn;
	else if (Code == Code::RR) return RR;
	else if (Code == Code::Aa) return Aa;
	else if (Code == Code::E) return E;
	else if (Code == Code::I) return I;
	else if (Code == Code::O) return O;
	else if (Code == Code::U) return U;
	else return STOP;
}
