#ifndef SIEVE_ANALYSIS_STAGES_H
#define SIEVE_ANALYSIS_STAGES_H
#include <QString>

// Enum reprezentujący etapy analizy sitowej
namespace SieveAnalysisStages
{

enum class Stage
{
    None = -1,              // Brak aktywnego etapu
    InitialSieveData,   // Dane wstępne
    ConfigurationSieve, // Konfigracja
    InitialWeighing,    // Ważenie wstępne (pustych sit i próbki)
    Sieving,            // Przesiewanie
    FinalWeighing,      // Ważenie końcowe (sit z materiałem)
    Summary             // Podsumowanie i raport
};

inline QString stageToString(Stage stage)
{
    switch (stage)
    {
        case Stage::None:
            return "None";

        case Stage::ConfigurationSieve:
            return "Configuration";

        case Stage::InitialWeighing:
            return "InitialWeighing";

        case Stage::Sieving:
            return "Sieving";

        case Stage::FinalWeighing:
            return "FinalWeighing";

        case Stage::Summary:
            return "Summary";

        default:
            return "Unknown";
    }
}

// Nazwa etapu po polsku
inline QString stageDisplayName(Stage stage)
{
    switch (stage)
    {
        case Stage::None:
            return "Brak";

        case Stage::ConfigurationSieve:
            return "Dane wstępne";

        case Stage::InitialWeighing:
            return "Ważenie wstępne";

        case Stage::Sieving:
            return "Przesiewanie";

        case Stage::FinalWeighing:
            return "Ważenie końcowe";

        case Stage::Summary:
            return "Podsumowanie";

        default:
            return "Nieznany";
    }
}


inline bool isValidStage(Stage stage)
{
    return stage >= Stage::InitialSieveData && stage <= Stage::Summary;
}


inline Stage nextStage(Stage current)
{
    int next = static_cast<int>(current) + 1;
    if(next <= static_cast<int>(Stage::Summary))
        return static_cast<Stage>(next);
    return Stage::InitialSieveData;
}


inline Stage previousStage(Stage current)
{
    int prev = static_cast<int>(current) - 1;
    if(prev >= static_cast<int>(Stage::InitialSieveData))
        return static_cast<Stage>(prev);
    return Stage::Summary;
}

}

#endif // SIEVE_ANALYSIS_STAGES_H
