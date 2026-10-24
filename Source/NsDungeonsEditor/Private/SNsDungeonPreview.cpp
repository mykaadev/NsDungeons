// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "SNsDungeonPreview.h"

#include "NsDungeonRuleSet.h"
#include "PropertyCustomizationHelpers.h"
#include "SNsDungeonView.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "NsDungeonsPreview"

void SNsDungeonPreview::Construct(const FArguments& InArgs)
{
    Result = MakeShared<FNsDungeonResult>();
    Seed = InArgs._Seed;
    FallbackSettings = InArgs._Settings.Get(FNsDungeonSettings());
    // The legend: the colors of the views
    const FString Legend = TEXT("Rooms: green start, gold goal, grey room, blue side loop, dark ring dead end, number = doors from the start.  ")
        TEXT("Doors: grey open, red locked (L), orange one-way (dot on the open side), purple hidden.  Keys: gold box (K = the lock it opens).");

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
            [
                SNew(STextBlock).Text(LOCTEXT("RuleSet", "Rule set"))
            ]
            + SHorizontalBox::Slot().FillWidth(1.0f).MaxWidth(320.0f)
            [
                SNew(SObjectPropertyEntryBox)
                .AllowedClass(UNsDungeonRuleSet::StaticClass())
                .ObjectPath_Lambda([this]() { return RuleSet.IsValid() ? RuleSet->GetPathName() : FString(); })
                .OnObjectChanged_Lambda([this](const FAssetData& Asset)
                {
                    RuleSet = Cast<UNsDungeonRuleSet>(Asset.GetAsset());
                    Generate();
                })
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.0f, 0.0f, 8.0f, 0.0f)
            [
                SNew(STextBlock).Text(LOCTEXT("Seed", "Seed"))
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SBox).WidthOverride(120.0f)
                [
                    SNew(SSpinBox<int32>)
                    .MinValue(0)
                    .MaxValue(MAX_int32)
                    .Value_Lambda([this]() { return Seed; })
                    .OnValueCommitted_Lambda([this](const int32 Value, ETextCommit::Type) { Seed = Value; Generate(); })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
            [
                SNew(SButton).Text(LOCTEXT("Generate", "Generate")).OnClicked_Lambda([this]() { Generate(); return FReply::Handled(); })
            ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton).Text(LOCTEXT("Next", "Next seed")).OnClicked_Lambda([this]() { ++Seed; Generate(); return FReply::Handled(); })
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
            [
                SNew(SButton).Text(LOCTEXT("Random", "Random seed")).OnClicked_Lambda([this]() { Seed = FMath::Rand(); Generate(); return FReply::Handled(); })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
            [
                SNew(STextBlock).Text(LOCTEXT("Step", "Step"))
            ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [
                SNew(SSlider)
                .Value_Lambda([this]() { return GetStepShare(); })
                .OnValueChanged_Lambda([this](const float Share) { SetStepShare(Share); })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f)
        [
            SNew(STextBlock).Text_Lambda([this]() { return GetStepText(); }).AutoWrapText(true)
        ]
        + SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f)
        [
            SNew(SSplitter)
            + SSplitter::Slot().Value(0.5f)
            [
                SNew(SNsDungeonView).Result(Result).StepIndex_Lambda([this]() { return StepIndex; }).bShowLayout(false)
            ]
            + SSplitter::Slot().Value(0.5f)
            [
                SNew(SNsDungeonView).Result(Result).StepIndex_Lambda([this]() { return StepIndex; }).bShowLayout(true)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [
            SNew(STextBlock).Text(FText::FromString(Legend)).AutoWrapText(true).ColorAndOpacity(FSlateColor::UseSubduedForeground())
        ]
    ];
    Generate();
    if (Result->Steps.IsValidIndex(InArgs._Step))
    {
        StepIndex = InArgs._Step;
    }
}

void SNsDungeonPreview::Generate()
{
    *Result = FNsDungeonGenerator::Generate(RuleSet.IsValid() ? RuleSet->Settings : FallbackSettings, Seed, true);
    StepIndex = FMath::Max(Result->Steps.Num() - 1, 0);
}

FText SNsDungeonPreview::GetStepText() const
{
    const FString Step = Result->Steps.IsValidIndex(StepIndex)
        ? FString::Printf(TEXT("Step %d of %d: %s. %s"), StepIndex + 1, Result->Steps.Num(), *Result->Steps[StepIndex].Name.ToString(), *Result->Steps[StepIndex].Note)
        : FString();
    const FString Seeds = Result->Seed != Seed ? FString::Printf(TEXT(" (seed %d failed, seed %d worked)"), Seed, Result->Seed) : FString();
    return FText::FromString(FString::Printf(TEXT("%s\n%s%s"), *Step, *Result->Message, *Seeds));
}

float SNsDungeonPreview::GetStepShare() const
{
    return Result->Steps.Num() > 1 ? static_cast<float>(StepIndex) / (Result->Steps.Num() - 1) : 1.0f;
}

void SNsDungeonPreview::SetStepShare(const float Share)
{
    StepIndex = Result->Steps.Num() > 1 ? FMath::RoundToInt32(Share * (Result->Steps.Num() - 1)) : 0;
}

#undef LOCTEXT_NAMESPACE
