// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "NsDungeonGenerator.h"
#include "Widgets/SCompoundWidget.h"

class UNsDungeonRuleSet;

/**
 * The NsDungeons Preview tab: pick a rule set and a seed, generate, and step through the generation. The graph of the chosen step
 * shows on the left and the cell layout on the right, with the validation result under them
 */
class SNsDungeonPreview : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SNsDungeonPreview) : _Seed(1) {}
        /** The first seed */
        SLATE_ARGUMENT(int32, Seed)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    /** Generates with the rule set and the seed, keeps every step, and shows the last one */
    void Generate();

    /** Returns the text over the views: the step, its note, and the result */
    FText GetStepText() const;

    /** Returns the share of the step slider */
    float GetStepShare() const;

    /** Sets the step from the slider */
    void SetStepShare(float Share);

private:
    /** The result that the views draw */
    TSharedPtr<FNsDungeonResult> Result;

    /** The rule set, or null for the default settings */
    TWeakObjectPtr<UNsDungeonRuleSet> RuleSet;

    /** The seed */
    int32 Seed = 1;

    /** The step that the graph view shows */
    int32 StepIndex = 0;
};
