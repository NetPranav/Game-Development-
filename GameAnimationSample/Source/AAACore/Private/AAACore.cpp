#include "AAACore.h"
#include "Modules/ModuleManager.h"
#include "AAAGameplayTags.h"

void FAAACoreModule::StartupModule()
{
	// Native tags must exist before any UObject/CDO or ability reads FAAAGameplayTags::Get().
	// Nothing else calls this, so without it every native tag is invalid at runtime.
	FAAAGameplayTags::InitializeNativeGameplayTags();
}

void FAAACoreModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FAAACoreModule, AAACore);
