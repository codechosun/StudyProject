// SPigeon.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFlyable.h" // 타 헤더 파일 인클루드 시에 반드시 generated.h 파일 위에 작성.
#include "SPigeon.generated.h"

USTRUCT()
struct FPigeonData
{
	GENERATED_BODY()

public:
	FPigeonData()
	{
	}

	FPigeonData(const FString& InName, int32 InID)
		: Name(InName) // 언리얼 코딩 컨벤션에는 입력 전용 매개변수명에 In-을 붙힘. 출력 전용은 Out-
		, ID(InID)
	{
	}

	friend FArchive& operator<<(FArchive& InArchive, FPigeonData& InPigeonData)
	{
		InArchive << InPigeonData.Name;
		InArchive << InPigeonData.ID;
		return InArchive;
	}

public:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 ID;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPigeonFlying, const FString&, InName, const int32, InID);

/**
 *
 */
UCLASS()
class STUDYPROJECT_API USPigeon
	: public UObject
	, public ISFlyable
{
	GENERATED_BODY()

public:
	USPigeon();

	virtual void Fly() override;

	const FString& GetPigeonName() const { return Name; }
	void SetPigeonName(const FString& InName) { Name = InName; }

	int32 GetPigeonID() const { return ID; }
	void SetPigeonID(const int32& InID) { ID = InID; }

	virtual void Serialize(FArchive& Ar) override;

public:
	FOnPigeonFlying OnPigeonFlying;

private:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 ID;

};
