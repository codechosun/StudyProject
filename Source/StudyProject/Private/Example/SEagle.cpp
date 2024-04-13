// SEagle.h


#include "Example/SEagle.h"

USEagle::USEagle()
{
	Name = TEXT("Eagle");
}

void USEagle::Fly()
{
	UE_LOG(LogTemp, Log, TEXT("%s is now flying."), *Name);
}
