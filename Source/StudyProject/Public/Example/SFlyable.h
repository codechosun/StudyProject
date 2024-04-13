// SFlyable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SFlyable.generated.h"

UINTERFACE(MinimalAPI)
class USFlyable : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class STUDYPROJECT_API ISFlyable
{
	GENERATED_BODY()

public:
	virtual void Fly() = 0;
	// ISFlyable 인터페이스에서 구현 해도되고 안해도됨. 보통은 안함.

};
