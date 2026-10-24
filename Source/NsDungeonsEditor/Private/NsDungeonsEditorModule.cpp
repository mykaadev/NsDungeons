// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/IConsoleManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "SNsDungeonPreview.h"
#include "Slate/WidgetRenderer.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "NsDungeonsEditor"

/** The editor module of NsDungeons: the NsDungeons Preview tab, in Tools of the main menu */
class FNsDungeonsEditorModule : public IModuleInterface
{
public:
    //~ Begin IModuleInterface
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PreviewTab, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
        {
            return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SNsDungeonPreview)];
        }))
        .SetDisplayName(LOCTEXT("PreviewTitle", "NsDungeons Preview"))
        .SetTooltipText(LOCTEXT("PreviewTooltip", "Generate a dungeon from a rule set and a seed, and step through the generation"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FNsDungeonsEditorModule::RegisterMenus));
        SavePreviewCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("NsDungeons.SavePreview"),
            TEXT("Saves a picture of the NsDungeons Preview for a seed: NsDungeons.SavePreview <Seed> <File.png>"),
            FConsoleCommandWithArgsDelegate::CreateStatic(&FNsDungeonsEditorModule::SavePreview));
    }

    virtual void ShutdownModule() override
    {
        IConsoleManager::Get().UnregisterConsoleObject(SavePreviewCommand);
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (FSlateApplication::IsInitialized())
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PreviewTab);
        }
    }
    //~ End IModuleInterface

private:
    /** Adds the tab to Tools in the main menu */
    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* const Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
        FToolMenuSection& Section = Menu->FindOrAddSection("NsDungeons", LOCTEXT("Section", "NsDungeons"));
        Section.AddMenuEntry("NsDungeonsPreview", LOCTEXT("PreviewTitle", "NsDungeons Preview"),
            LOCTEXT("PreviewTooltip", "Generate a dungeon from a rule set and a seed, and step through the generation"), FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(PreviewTab); })));
    }

    /** Renders the preview of a seed off screen and saves it as a PNG, for a check with no person at the editor */
    static void SavePreview(const TArray<FString>& Args)
    {
        const int32 Seed = Args.IsValidIndex(0) ? FCString::Atoi(*Args[0]) : 1;
        const FString File = Args.IsValidIndex(1) ? Args[1] : FPaths::ProjectSavedDir() / TEXT("NsDungeonsPreview.png");
        const FVector2D Size(1600.0, 900.0);
        const TSharedRef<SNsDungeonPreview> Preview = SNew(SNsDungeonPreview).Seed(Seed);
        UTextureRenderTarget2D* const Target = NewObject<UTextureRenderTarget2D>();
        Target->InitCustomFormat(Size.X, Size.Y, PF_B8G8R8A8, true);
        Target->UpdateResourceImmediate(true);
        FWidgetRenderer Renderer(true);
        // Two draws: the first one lays the widgets out
        Renderer.DrawWidget(Target, Preview, Size, 0.0f);
        Renderer.DrawWidget(Target, Preview, Size, 0.0f);
        FlushRenderingCommands();
        FImage Image;
        TArray64<uint8> Png;
        if (FImageUtils::GetRenderTargetImage(Target, Image) && FImageUtils::CompressImage(Png, TEXT("png"), Image) && FFileHelper::SaveArrayToFile(Png, *File))
        {
            UE_LOG(LogTemp, Display, TEXT("NsDungeons preview of seed %d saved to %s"), Seed, *File);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("NsDungeons preview of seed %d could not be saved to %s"), Seed, *File);
        }
    }

    /** The console command that saves a preview picture */
    IConsoleObject* SavePreviewCommand = nullptr;

    /** The id of the preview tab */
    static inline const FName PreviewTab = TEXT("NsDungeonsPreview");
};

IMPLEMENT_MODULE(FNsDungeonsEditorModule, NsDungeonsEditor)

#undef LOCTEXT_NAMESPACE
