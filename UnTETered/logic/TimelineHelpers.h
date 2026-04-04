//
// Created by Armin Peukert on 04.04.26.
//
namespace TimelineHelpers
{
    inline double getQuarterNotesPerBar(int numerator, int denominator)
    {
        return denominator > 0
            ? 4.0 * static_cast<double>(numerator) / static_cast<double>(denominator)
            : 4.0;
    }

    inline double ppqToBar(double ppq, int numerator, int denominator)
    {
        const auto qnPerBar = getQuarterNotesPerBar(numerator, denominator);
        return qnPerBar > 0.0 ? ppq / qnPerBar : 0.0;
    }
}