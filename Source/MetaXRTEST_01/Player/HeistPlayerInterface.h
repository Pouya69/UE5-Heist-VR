

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistPlayerInterface.generated.h"

UINTERFACE()
class UHeistPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class METAXRTEST_01_API IHeistPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void TraceRightFinger();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Player Interface")
		void TraceLeftFinger();
};
